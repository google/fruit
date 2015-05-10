/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRUIT_COMPONENT_FUNCTORS_DEFN_H
#define FRUIT_COMPONENT_FUNCTORS_DEFN_H

#include "../component.h"

#include "injection_errors.h"
#include "storage/component_storage.h"

#include <memory>

/*********************************************************************************************************************************
  This file contains functors that take a Comp and return a struct Op with the form:
  struct {
    using Result = Comp1;
    void operator()(ComponentStorage& storage) {...}
  }
*********************************************************************************************************************************/

namespace fruit {
namespace impl {
namespace meta {

struct GetResult {
  template <typename F>
  struct apply {
    using type = typename F::Result;
  };
};

struct Identity {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Comp;
      void operator()(ComponentStorage&) {}
    };
  };
};

template <typename... Functors>
struct ComposeFunctors;

template <>
struct ComposeFunctors<> : Identity {
};

template <typename F, typename... Functors>
struct ComposeFunctors<F, Functors...> {
  template <typename Comp>
  struct apply {
    using Op = Apply<F, Comp>;
    using LazyOps = Apply<LazyFunctor<ComposeFunctors<Functors...>>, Lazy<typename Op::Result>>;
    struct type {
      using Result = Eval<Conditional<Lazy<Apply<IsError, typename Op::Result>>,
                                      Lazy<typename Op::Result>,
                                      Apply<LazyFunctor<GetResult>, LazyOps>
                                      >>;
      void operator()(ComponentStorage& storage) {
        Op()(storage);
        Eval<LazyOps>()(storage);
      }
    };
  };
};

template <typename TargetRequirements, typename T>
struct EnsureProvidedType;

template <typename TargetRequirements, typename L>
struct EnsureProvidedTypes;

// Doesn't actually bind in ComponentStorage. The binding is added later (if needed) using ProcessInterfaceBinding.
template <typename AnnotatedI, typename AnnotatedC>
struct AddDeferredInterfaceBinding {
  template <typename Comp>
  struct apply {
    struct type {
      using Comp1 = ConsComp<typename Comp::Rs,
                             typename Comp::Ps,
                             typename Comp::Deps,
                             Apply<AddToSet,
                                   ConsInterfaceBinding<AnnotatedI, AnnotatedC>,
                                   typename Comp::InterfaceBindings>,
                             typename Comp::DeferredBindingFunctors>;
      using I = Apply<RemoveAnnotations, AnnotatedI>;
      using C = Apply<RemoveAnnotations, AnnotatedC>;
      // Note that we do NOT call AddProvidedType here. We'll only know the right required type
      // when the binding will be used.
      using Result = Eval<std::conditional<!std::is_base_of<I, C>::value,
                                           Error<NotABaseClassOfErrorTag, I, C>,
                                           Comp1>>;
      void operator()(ComponentStorage&) {}
    };
  };
};

template <typename AnnotatedI, typename AnnotatedC>
struct ProcessInterfaceBinding {
  template <typename Comp>
  struct apply {
    struct type {
      // This must be here (and not in AddDeferredInterfaceBinding) because the binding might be
      // used to bind functors instead, so we might never need to add C to the requirements.
      using Result = Apply<AddProvidedType, Comp, AnnotatedI, Vector<AnnotatedC>>;
      void operator()(ComponentStorage& storage) {
        storage.addBinding(InjectorStorage::createBindingDataForBind<AnnotatedI, AnnotatedC>());
      };
    };
  };
};

template <typename AnnotatedI, typename AnnotatedC>
struct AddInterfaceMultibinding {
  template <typename Comp>
  struct apply {
    struct type {
      using I = Apply<RemoveAnnotations, AnnotatedI>;
      using C = Apply<RemoveAnnotations, AnnotatedC>;
      using Result = Eval<std::conditional<!std::is_base_of<I, C>::value,
                                           Error<NotABaseClassOfErrorTag, I, C>,
                                           Apply<AddRequirements, Comp, Vector<AnnotatedC>>
                                           >>;
      void operator()(ComponentStorage& storage) {
        storage.addMultibinding(InjectorStorage::createMultibindingDataForBinding<AnnotatedI, AnnotatedC>());
      };
    };
  };
};

template <typename AnnotatedSignature, typename Lambda, typename OptionalAnnotatedI>
struct PostProcessRegisterProviderHelper;

template <typename AnnotatedSignature, typename Lambda, typename AnnotatedI>
struct PostProcessRegisterProviderHelper {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForProvider<AnnotatedSignature, Lambda>());
    component.addCompressedBinding(InjectorStorage::createBindingDataForCompressedProvider<AnnotatedSignature, Lambda, AnnotatedI>());
  }
};

template <typename AnnotatedSignature, typename Lambda>
struct PostProcessRegisterProviderHelper<AnnotatedSignature, Lambda, None> {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForProvider<AnnotatedSignature, Lambda>());
  }
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerProvider() overloads in ComponentStorage.
template <typename AnnotatedSignature, typename Lambda>
struct PostProcessRegisterProvider {
  template <typename Comp>
  struct apply {
    struct type {
      using AnnotatedC = CheckedApply<NormalizeType, CheckedApply<SignatureType, AnnotatedSignature>>;
      using OptionalAnnotatedI = CheckedApply<GetBindingToInterface, AnnotatedC, typename Comp::InterfaceBindings>;
      using Result = Comp;
      void operator()(ComponentStorage& storage) {
        PostProcessRegisterProviderHelper<AnnotatedSignature, Lambda, OptionalAnnotatedI>()(storage);
      }
    };
  };
};

template <typename AnnotatedSignature, typename Lambda>
struct PreProcessRegisterProvider {
  template <typename Comp>
  struct apply {
    struct type {
      using Signature = CheckedApply<RemoveAnnotationsFromSignature, AnnotatedSignature>;
      using SignatureFromLambda = CheckedApply<FunctionSignature, Lambda>;
      
      using AnnotatedC = CheckedApply<NormalizeType, CheckedApply<SignatureType, AnnotatedSignature>>;
      using OptionalI = CheckedApply<GetBindingToInterface, AnnotatedC, typename Comp::InterfaceBindings>;
      using AnnotatedCDeps = CheckedApply<ExpandProvidersInParams,
                                CheckedApply<NormalizeTypeVector, CheckedApply<SignatureArgs, AnnotatedSignature>>>;
      using Result = Apply<ApplyAndPostponeFirstArgument<PropagateErrors, Signature, SignatureFromLambda>,
                           Eval<Conditional<Apply<LazyFunctor<IsSame>, Lazy<Signature>, Lazy<SignatureFromLambda>>,
                                            Lazy<CheckedApply<AddProvidedType, Comp, AnnotatedC, AnnotatedCDeps>>,
                                            Lazy<Error<AnnotatedSignatureDifferentFromLambdaSignatureErrorTag, Signature, SignatureFromLambda>>
                                            >
                           >>;
      void operator()(ComponentStorage&) {}
    };
  };
};

// The registration is actually deferred until the PartialComponent is converted to a component.
template <typename AnnotatedSignature, typename Lambda>
struct DeferredRegisterProviderWithAnnotations {
  template <typename Comp>
  struct apply {
    struct type {
      using Comp1 = Apply<AddDeferredBinding, Comp, PostProcessRegisterProvider<AnnotatedSignature, Lambda>>;
      using Comp2 = CheckedApply<GetResult, CheckedApply<PreProcessRegisterProvider<AnnotatedSignature, Lambda>, Comp1>>;
      using Result = Comp2;
      void operator()(ComponentStorage&) {}
    };
  };
};

// The registration is actually deferred until the PartialComponent is converted to a component.
template <typename Lambda>
using DeferredRegisterProvider = DeferredRegisterProviderWithAnnotations<Apply<FunctionSignature, Lambda>, Lambda>;

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerMultibindingProvider() overloads in ComponentStorage.
template <typename AnnotatedSignature, typename Lambda>
struct RegisterMultibindingProviderWithAnnotations {
  template <typename Comp>
  struct apply {
    struct type {
      using Signature = CheckedApply<RemoveAnnotationsFromSignature, AnnotatedSignature>;
      using SignatureFromLambda = CheckedApply<FunctionSignature, Lambda>;
      
      using AnnotatedArgs = CheckedApply<SignatureArgs, AnnotatedSignature>;
      using AnnotatedArgSet = CheckedApply<ExpandProvidersInParams,
                                           CheckedApply<NormalizeTypeVector, AnnotatedArgs>>;
      using Result = Apply<ApplyAndPostponeFirstArgument<PropagateErrors, Signature, SignatureFromLambda>,
                           Eval<Conditional<Apply<LazyFunctor<IsSame>, Lazy<Signature>, Lazy<SignatureFromLambda>>,
                                            Lazy<CheckedApply<AddRequirements, Comp, AnnotatedArgSet>>,
                                            Lazy<Error<AnnotatedSignatureDifferentFromLambdaSignatureErrorTag, Signature, SignatureFromLambda>>
                                            >
                           >>;
      void operator()(ComponentStorage& storage) {
        storage.addMultibinding(InjectorStorage::createMultibindingDataForProvider<AnnotatedSignature, Lambda>());
      }
    };
  };
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerMultibindingProvider() overloads in ComponentStorage.
template <typename Lambda>
using RegisterMultibindingProvider = RegisterMultibindingProviderWithAnnotations<Apply<FunctionSignature, Lambda>, Lambda>;

// Non-assisted case.
template <int numAssistedBefore, int numNonAssistedBefore, typename Arg, typename InjectedArgsTuple, typename UserProvidedArgsTuple>
struct GetAssistedArg {
  inline Arg operator()(InjectedArgsTuple& injected_args, UserProvidedArgsTuple&) {
    return std::get<numNonAssistedBefore>(injected_args);
  }
};

// Assisted case.
template <int numAssistedBefore, int numNonAssistedBefore, typename Arg, typename InjectedArgsTuple, typename UserProvidedArgsTuple>
struct GetAssistedArg<numAssistedBefore, numNonAssistedBefore, Assisted<Arg>, InjectedArgsTuple, UserProvidedArgsTuple> {
  inline Arg operator()(InjectedArgsTuple&, UserProvidedArgsTuple& user_provided_args) {
    return std::get<numAssistedBefore>(user_provided_args);
  }
};

template <typename DecoratedSignature,
          typename Lambda,
          // std::function<InjectedSignature> is the injected type (possibly with an Annotation<> wrapping it)
          typename InjectedSignature       = Apply<InjectedSignatureForAssistedFactory, DecoratedSignature>,
          typename RequiredLambdaSignature = Apply<RequiredLambdaSignatureForAssistedFactory, DecoratedSignature>,
          typename InjectedAnnotatedArgs   = Apply<RemoveAssisted, Apply<SignatureArgs, DecoratedSignature>>,
          // The types that are injected, unwrapped from any Annotation<>.
          typename InjectedArgs            = Apply<RemoveAnnotationsFromVector, InjectedAnnotatedArgs>,
          typename IndexSequence = GenerateIntSequence<
              Apply<VectorSize,
                  Apply<RequiredLambdaArgsForAssistedFactory, DecoratedSignature>
              >::value>
          >
struct RegisterFactory;

template <typename DecoratedSignature, typename Lambda, typename C, typename... UserProvidedArgs, typename... AllArgs, typename... InjectedAnnotatedArgs, typename... InjectedArgs, int... indexes>
struct RegisterFactory<DecoratedSignature, Lambda, C(UserProvidedArgs...), C(AllArgs...), Vector<InjectedAnnotatedArgs...>, Vector<InjectedArgs...>, IntVector<indexes...>> {
  template <typename Comp>
  struct apply {
    // Here we call "decorated" the types that might be wrapped in Annotated<> or Assisted<>,
    // while we call "annotated" the ones that might only be wrapped in Annotated<> (but not Assisted<>).
    using AnnotatedT = Apply<SignatureType, DecoratedSignature>;
    using T          = Apply<RemoveAnnotations, AnnotatedT>;
    using DecoratedArgs = Apply<SignatureArgs, DecoratedSignature>;
    using InjectedSignature = C(UserProvidedArgs...);
    using RequiredSignature = C(AllArgs...);
    using Functor = std::function<InjectedSignature>;
    // This is usually the same as Functor, but this might be annotated.
    using AnnotatedFunctor = Apply<CopyAnnotation, AnnotatedT, Functor>;
    using FunctorDeps = Apply<NormalizeTypeVector, Vector<InjectedAnnotatedArgs...>>;
    struct type {
      // The first is_same check is a bit of a hack, it's to make the F2/RealF2 split work in the caller (we need to allow Lambda to be a function type).
      using Result = Eval<Conditional<Lazy<Bool<!std::is_empty<Lambda>::value && !std::is_same<Lambda, Apply<FunctionSignature, Lambda>>::value>>,
                                      Lazy<Error<LambdaWithCapturesErrorTag, Lambda>>,
                                      Conditional<Lazy<Bool<!std::is_same<RequiredSignature, Apply<FunctionSignature, Lambda>>::value>>,
                                                  Lazy<Error<FunctorSignatureDoesNotMatchErrorTag, RequiredSignature, Apply<FunctionSignature, Lambda>>>,
                                                  Conditional<Lazy<Bool<std::is_pointer<T>::value>>,
                                                              Lazy<Error<FactoryReturningPointerErrorTag, DecoratedSignature>>,
                                                              Apply<LazyFunctor<AddProvidedType>, Lazy<Comp>, Lazy<Functor>, Lazy<FunctorDeps>>
                                                              >
                                                  >
                                      >>;
      void operator()(ComponentStorage& storage) {
        auto function_provider = [](InjectedArgs... args) {
          // TODO: Using auto and make_tuple here results in a GCC segfault with GCC 4.8.1.
          // Check this on later versions and consider filing a bug.
          std::tuple<InjectedArgs...> injected_args(args...);
          auto object_provider = [injected_args](UserProvidedArgs... params) mutable {
            auto user_provided_args = std::tie(params...);
            // These are unused if they are 0-arg tuples. Silence the unused-variable warnings anyway.
            (void) injected_args;
            (void) user_provided_args;
            
            return LambdaInvoker::invoke<Lambda, AllArgs...>(GetAssistedArg<
                                                                Apply<NumAssistedBefore, Int<indexes>, DecoratedArgs>::value,
                                                                indexes - Apply<NumAssistedBefore, Int<indexes>, DecoratedArgs>::value,
                                                                // Note that the Assisted<> wrapper (if any) remains, we just remove any wrapping Annotated<>.
                                                                Apply<RemoveAnnotations, GetNthType<indexes, DecoratedArgs>>,
                                                                decltype(injected_args),
                                                                decltype(user_provided_args)
                                                            >()(injected_args, user_provided_args)
                                                            ...);
          };
          return Functor(object_provider);
        };
        storage.addBinding(InjectorStorage::createBindingDataForProvider<Apply<ConstructSignature, AnnotatedFunctor, Vector<InjectedAnnotatedArgs...>>,
                                                                         decltype(function_provider)>());
      }
    };
  };
};

template <typename AnnotatedSignature>
struct PostProcessRegisterConstructor;

template <typename AnnotatedSignature, typename OptionalAnnotatedI>
struct PostProcessRegisterConstructorHelper;

template <typename AnnotatedSignature, typename AnnotatedI>
struct PostProcessRegisterConstructorHelper {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForConstructor<AnnotatedSignature>());
    component.addCompressedBinding(InjectorStorage::createBindingDataForCompressedConstructor<AnnotatedSignature, AnnotatedI>());
  }
};

template <typename AnnotatedSignature>
struct PostProcessRegisterConstructorHelper<AnnotatedSignature, None> {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForConstructor<AnnotatedSignature>());
  }
};

template <typename AnnotatedSignature>
struct PostProcessRegisterConstructor {
  template <typename Comp>
  struct apply {
    struct type {
      using AnnotatedC = Apply<NormalizeType, Apply<SignatureType, AnnotatedSignature>>;
      using Result = Comp;
      void operator()(ComponentStorage& storage) {
        PostProcessRegisterConstructorHelper<AnnotatedSignature, Apply<GetBindingToInterface, AnnotatedC, typename Comp::InterfaceBindings>>()(storage);
      }
    };
  };
};

// We need to extract this to make the computation of SignatureType lazy, otherwise it'd be evaluated in DeferredRegisterConstructor even when it should not be.
struct AddProvidedTypeForRegisterConstructor {
  template <typename AnnotatedSignature, typename Comp>
  struct apply {
    using AnnotatedT    = Apply<SignatureType, AnnotatedSignature>;
    using AnnotatedArgs = Apply<SignatureArgs, AnnotatedSignature>;
    
    using AnnotatedC = Apply<NormalizeType, AnnotatedT>;
    using CDeps = Apply<ExpandProvidersInParams, Apply<NormalizeTypeVector, AnnotatedArgs>>;
    using type = Apply<AddProvidedType, Comp, AnnotatedC, CDeps>;
  };
};

// We need to extract this to make the computation of RemoveAnnotationsFromSignature/SignatureType lazy, otherwise it'd be evaluated in DeferredRegisterConstructor even when it should not be.
struct ConstructNoConstructorMatchingInjectSignatureError {
  template <typename AnnotatedSignature>
  struct apply {
    using Signature = Apply<RemoveAnnotationsFromSignature, AnnotatedSignature>;
    using C = Apply<SignatureType, Signature>;
    using type = Error<NoConstructorMatchingInjectSignatureErrorTag, C, Signature>;
  };
};

template <typename AnnotatedSignature>
struct PreProcessRegisterConstructor {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Eval<Conditional<Lazy<Apply<Not, Apply<IsValidSignature, AnnotatedSignature>>>,
                                      Lazy<Error<NotASignatureErrorTag, AnnotatedSignature>>,
                                      Conditional<Apply<LazyFunctor<Not>,
                                                        Apply<LazyFunctor<IsConstructibleWithVector>,
                                                              Apply<LazyFunctor<RemoveAnnotations>,           Apply<LazyFunctor<SignatureType>, Lazy<AnnotatedSignature>>>,
                                                              Apply<LazyFunctor<RemoveAnnotationsFromVector>, Apply<LazyFunctor<SignatureArgs>, Lazy<AnnotatedSignature>>>
                                                              >
                                                        >,
                                                  Apply<LazyFunctor<ConstructNoConstructorMatchingInjectSignatureError>,
                                                                    Apply<LazyFunctor<RemoveAnnotationsFromSignature>, Lazy<AnnotatedSignature>>>,
                                                  Apply<LazyFunctor<AddProvidedTypeForRegisterConstructor>,
                                                        Lazy<AnnotatedSignature>,
                                                        Lazy<Comp>
                                                        >
                                                  >
                                      >>;
      void operator()(ComponentStorage&) {}
    };
  };
};

template <typename AnnotatedSignature>
struct DeferredRegisterConstructor {
  template <typename Comp>
  struct apply {
    struct type {
      using Comp1 = Apply<AddDeferredBinding,
                          Comp,
                          PostProcessRegisterConstructor<AnnotatedSignature>
                          >;
      using Comp2 = CheckedApply<GetResult, CheckedApply<PreProcessRegisterConstructor<AnnotatedSignature>, Comp1>>;
      using Result = Comp2;
      void operator()(ComponentStorage&) {}
    };
  };
};

template <typename AnnotatedC, typename C>
struct RegisterInstance {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Apply<AddProvidedType, Comp, AnnotatedC, Vector<>>;
      void operator()(ComponentStorage& storage, C& instance) {
        storage.addBinding(InjectorStorage::createBindingDataForBindInstance<AnnotatedC, C>(instance));
      };
    };
  };
};

template <typename AnnotatedC, typename C>
struct AddInstanceMultibinding {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Comp;
      void operator()(ComponentStorage& storage, C& instance) {
        storage.addMultibinding(InjectorStorage::createMultibindingDataForInstance<AnnotatedC, C>(instance));
      };
    };
  };
};

template <typename AnnotatedC, typename C>
struct AddInstanceMultibindings {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Comp;
      void operator()(ComponentStorage& storage, std::vector<C>& instances) {
        for (C& instance : instances) {
          storage.addMultibinding(InjectorStorage::createMultibindingDataForInstance<AnnotatedC, C>(instance));
        }
      };
    };
  };
};

template <typename DecoratedSignature,
          typename RequiredSignature = Apply<RequiredLambdaSignatureForAssistedFactory, DecoratedSignature>>
struct RegisterConstructorAsValueFactory;

template <typename DecoratedSignature, typename T, typename... Args>
struct RegisterConstructorAsValueFactory<DecoratedSignature, T(Args...)> {
  template <typename Comp>
  struct apply {
    using RequiredSignature = T(Args...);
    using F1 = RegisterFactory<DecoratedSignature, RequiredSignature>;
    using Op = Apply<F1, Comp>;
    struct type {
      using Result = typename Op::Result;
      void operator()(ComponentStorage& storage) {
        auto provider = [](Args... args) {
          return T(std::forward<Args>(args)...);
        };
        using RealF1 = RegisterFactory<DecoratedSignature, decltype(provider)>;
        using RealOp = Apply<RealF1, Comp>;
        FruitStaticAssert(std::is_same<typename Op::Result,
                                       typename RealOp::Result>::value,
                          "Fruit bug, F1 and RealF1 out of sync.");
        RealOp()(storage);
      }
    };
  };
};

template <typename OtherComp>
struct InstallComponent {
  template <typename Comp>
  struct apply {
    using new_Ps = Apply<ConcatVectors, typename OtherComp::Ps, typename Comp::Ps>;
    using new_Rs = Apply<SetDifference,
                         Apply<SetUnion,
                               typename OtherComp::Rs,
                               typename Comp::Rs>,
                         new_Ps>;
    using new_Deps = 
        Apply<AddProofTreeVectorToForest, typename OtherComp::Deps, typename Comp::Deps, typename Comp::Ps>;
    using new_InterfaceBindings = 
        Apply<SetUnion, typename OtherComp::InterfaceBindings, typename Comp::InterfaceBindings>;
    using new_DeferredBindingFunctors =
        Apply<ConcatVectors, typename OtherComp::DeferredBindingFunctors, typename Comp::DeferredBindingFunctors>;
    using Comp1 = ConsComp<new_Rs, new_Ps, new_Deps, new_InterfaceBindings, new_DeferredBindingFunctors>;
    using DuplicateTypes = Apply<SetIntersection, typename OtherComp::Ps, typename Comp::Ps>;
    struct type {
      using Result = Eval<std::conditional<Apply<VectorSize, DuplicateTypes>::value != 0,
                                           Apply<ConstructErrorWithArgVector, DuplicateTypesInComponentErrorTag, DuplicateTypes>,
                                           Comp1
                                           >>;
      void operator()(ComponentStorage& storage, ComponentStorage&& other_storage) {
        storage.install(std::move(other_storage));
      }
    };
  };
};

// Used to limit the amount of metaprogramming in component.h, that might confuse users.
template <typename... OtherCompParams>
struct InstallComponentHelper {
  template <typename Comp>
  struct apply {
    using OtherComp = Apply<ConstructComponentImpl, OtherCompParams...>;
    struct E {
      using type = OtherComp;
      void operator()(ComponentStorage&, ComponentStorage&&) {}
    };
    using type = Eval<Conditional<Lazy<Apply<IsError, OtherComp>>,
                                  Lazy<E>,
                                  Lazy<Apply<InstallComponent<OtherComp>, Comp>>
                                  >>;
  };
};

template <typename DestComp>
struct ConvertComponent {
  template <typename SourceComp>
  struct apply {
    // We need to register:
    // * All the types provided by the new component
    // * All the types required by the old component
    // except:
    // * The ones already provided by the old component.
    // * The ones required by the new one.
    using ToRegister = Apply<SetDifference,
                             Apply<SetUnion, typename DestComp::Ps, typename SourceComp::Rs>,
                             Apply<SetUnion, typename DestComp::Rs, typename SourceComp::Ps>>;
    using F1 = EnsureProvidedTypes<typename DestComp::Rs, ToRegister>;    
    using type = Apply<F1, SourceComp>;
    
    // Not needed, just double-checking.
    // Uses FruitStaticAssert instead of FruitDelegateCheck so that it's checked only in debug mode.
    FruitStaticAssert(true || sizeof(Eval<Conditional<Lazy<Bool<Apply<IsError, typename type::Result>::value || Apply<IsError, DestComp>::value>>,
                                                      Lazy<int>, // No checks, we'll report a user error soon.
                                                      Apply<LazyFunctor<CheckComponentEntails>, Lazy<typename type::Result>, Lazy<DestComp>>
                                                      >>), "");
  };
};

struct ProcessDeferredBindings {
  template <typename Comp>
  struct apply;
  
  template <typename RsParam, typename PsParam, typename DepsParam, typename InterfaceBindingsParam, 
            typename... DeferredBindingFunctors>
  struct apply<ConsComp<RsParam, PsParam, DepsParam, InterfaceBindingsParam, Vector<DeferredBindingFunctors...>>> {
    // Comp1 is the same as Comp, but without the DeferredBindingFunctors.
    using Comp1 = ConsComp<RsParam, PsParam, DepsParam, InterfaceBindingsParam, Vector<>>;
    using type = Apply<ComposeFunctors<DeferredBindingFunctors...>, Comp1>;
  };
};

// The types in TargetRequirements will not be auto-registered.
template <typename TargetRequirements, typename AnnotatedC>
struct AutoRegister;

template <typename TargetRequirements, bool has_inject_annotation, typename AnnotatedC>
struct AutoRegisterHelper;

// C has an Inject typedef, use it.
template <typename TargetRequirements, typename AnnotatedC>
struct AutoRegisterHelper<TargetRequirements, true, AnnotatedC> {
  template <typename Comp>
  struct apply {
    using C = Apply<RemoveAnnotations, AnnotatedC>;
    using Inject = Apply<GetInjectAnnotation, C>;
    using F = ComposeFunctors<
                  PreProcessRegisterConstructor<Inject>,
                  PostProcessRegisterConstructor<Inject>,
                  EnsureProvidedTypes<TargetRequirements,
                                      CheckedApply<ExpandProvidersInParams, CheckedApply<SignatureArgs, Inject>>>
              >;
    struct E {
      using Result = Inject;
      void operator()(ComponentStorage&) {}
    };
    using type = Eval<Conditional<Lazy<Apply<IsError, Inject>>,
                                  Lazy<E>,
                                  Apply<LazyFunctor<F>, Lazy<Comp>>
                                  >>;
  };
};

template <typename TargetRequirements, typename AnnotatedC>
struct AutoRegisterHelper<TargetRequirements, false, AnnotatedC> {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Error<NoBindingFoundErrorTag, AnnotatedC>;
      void operator()(ComponentStorage&) {}
    };
  };
};

template <typename TargetRequirements, bool has_interface_binding, bool has_inject_annotation, typename C, typename AnnotatedSignature, typename... Args>
struct AutoRegisterFactoryHelper;

// AnnotatedI has an interface binding, use it and look for a factory that returns the type that AnnotatedI is bound to.
template <typename TargetRequirements, bool unused, typename I, typename AnnotatedSignature, typename... Argz>
struct AutoRegisterFactoryHelper<TargetRequirements, true, unused, std::unique_ptr<I>, AnnotatedSignature, Argz...> {
  template <typename Comp>
  struct apply {
    using AnnotatedI = Apply<CopyAnnotation, Apply<SignatureType, AnnotatedSignature>, I>;
    using AnnotatedC = Apply<GetInterfaceBinding, AnnotatedI, typename Comp::InterfaceBindings>;
    using C          = Apply<RemoveAnnotations, AnnotatedC>;
    using IFunctor = std::function<std::unique_ptr<I>(Argz...)>;
    using CFunctor = std::function<std::unique_ptr<C>(Argz...)>;
    using AnnotatedIFunctor = Apply<CopyAnnotation, AnnotatedI, IFunctor>;
    using AnnotatedCFunctor = Apply<CopyAnnotation, AnnotatedI, CFunctor>;
    
    using F1 = EnsureProvidedType<TargetRequirements, AnnotatedCFunctor>;
    using F2 = PreProcessRegisterProvider<AnnotatedIFunctor*(AnnotatedCFunctor*), IFunctor*(CFunctor*)>;
    using F3 = PostProcessRegisterProvider<AnnotatedIFunctor*(AnnotatedCFunctor*), IFunctor*(CFunctor*)>;
    using Op = Apply<ComposeFunctors<F1, F2, F3>, Comp>;
    struct type {
      using Result = typename Op::Result;
      void operator()(ComponentStorage& storage) {
        auto provider = [](CFunctor* fun) {
          // TODO: Switch to return by value here if possible.
          return new IFunctor([=](Argz... args) {
            C* c = (*fun)(args...).release();
            I* i = static_cast<I*>(c);
            return std::unique_ptr<I>(i);
          });
        };
        using RealF2 = PreProcessRegisterProvider<AnnotatedIFunctor*(AnnotatedCFunctor*), decltype(provider)>;
        using RealF3 = PostProcessRegisterProvider<AnnotatedIFunctor*(AnnotatedCFunctor*), decltype(provider)>;
        using RealOp = Apply<ComposeFunctors<F1, RealF2, RealF3>, Comp>;
        FruitStaticAssert(std::is_same<typename Op::Result,
                                       typename RealOp::Result>::value,
                          "Fruit bug, F2/F3 and RealF2/RealF3 out of sync.");
        RealOp()(storage);
      }
    };
  };
};

// C doesn't have an interface binding as interface, nor an INJECT annotation.
// Bind std::function<unique_ptr<C>(Args...)> to std::function<C(Args...)> (possibly with annotations).
template <typename TargetRequirements, typename C, typename AnnotatedSignature, typename... Argz>
struct AutoRegisterFactoryHelper<TargetRequirements, false, false, std::unique_ptr<C>, AnnotatedSignature, Argz...> {
  template <typename Comp>
  struct apply {
    using CFunctor = std::function<C(Argz...)>;
    using CUniquePtrFunctor = std::function<std::unique_ptr<C>(Argz...)>;
    using AnnotatedCUniquePtr        = Apply<SignatureType, AnnotatedSignature>;
    using AnnotatedC                 = Apply<CopyAnnotation, AnnotatedCUniquePtr, C>;
    using AnnotatedCFunctor          = Apply<CopyAnnotation, AnnotatedCUniquePtr, CFunctor>;
    using AnnotatedCUniquePtrFunctor = Apply<CopyAnnotation, AnnotatedCUniquePtr, CUniquePtrFunctor>;
    
    using F1 = EnsureProvidedType<TargetRequirements, AnnotatedCFunctor>;
    using F2 = PreProcessRegisterProvider<AnnotatedCUniquePtrFunctor*(AnnotatedCFunctor*), CUniquePtrFunctor*(CFunctor*)>;
    using F3 = PostProcessRegisterProvider<AnnotatedCUniquePtrFunctor*(AnnotatedCFunctor*), CUniquePtrFunctor*(CFunctor*)>;
    using Op = Apply<ComposeFunctors<F1, F2, F3>, Comp>;
    struct type {
      // If we are about to report a NoBindingFound error for AnnotatedCFunctor, report one for std::function<std::unique_ptr<C>(Argz...)> instead,
      // otherwise we'd report an error about a type that the user doesn't expect.
      using Result = Eval<std::conditional<std::is_same<typename Op::Result, Error<NoBindingFoundErrorTag, AnnotatedCFunctor>>::value,
                                           Error<NoBindingFoundErrorTag, std::function<std::unique_ptr<C>(Argz...)>>,
                                           typename Op::Result
                                           >> ;
      void operator()(ComponentStorage& storage) {
        auto provider = [](CFunctor* fun) {
          // TODO: Switch to return by value here if possible.
          return new CUniquePtrFunctor([=](Argz... args) {
            C* c = new C((*fun)(args...));
            return std::unique_ptr<C>(c);
          });
        };
        using RealF2 = PreProcessRegisterProvider<AnnotatedCUniquePtrFunctor*(AnnotatedCFunctor*), decltype(provider)>;
        using RealF3 = PostProcessRegisterProvider<AnnotatedCUniquePtrFunctor*(AnnotatedCFunctor*), decltype(provider)>;
        using RealOp = Apply<ComposeFunctors<F1, RealF2, RealF3>, Comp>;
        FruitStaticAssert(std::is_same<typename Op::Result,
                                       typename RealOp::Result>::value,
                          "Fruit bug, F2/F3 and RealF2/RealF3 out of sync.");
        RealOp()(storage);
      }
    };
  };
};

// This case never happens, has_inject_annotation is set to false below if the factory returns an unique_ptr.
template <typename TargetRequirements, typename C, typename AnnotatedSignature, typename... Argz>
struct AutoRegisterFactoryHelper<TargetRequirements, false, true, std::unique_ptr<C>, AnnotatedSignature, Argz...> {
  template <typename Comp>
  struct apply;
};

// C has an Inject typedef, use it. Value (not unique_ptr) case.
// TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
template <typename TargetRequirements, typename C, typename AnnotatedSignature, typename... Argz>
struct AutoRegisterFactoryHelper<TargetRequirements, false, true, C, AnnotatedSignature, Argz...> {
  template <typename Comp>
  struct apply {
    using DecoratedSignature = Apply<GetInjectAnnotation, C>;
    using DecoratedSignatureArgs = CheckedApply<SignatureArgs, DecoratedSignature>;
    using ActualSignatureInInjectionTypedef = CheckedApply<ConstructSignature,
                                                           CheckedApply<SignatureType, DecoratedSignature>,
                                                           CheckedApply<RemoveNonAssisted, DecoratedSignatureArgs>>;
    using NonAssistedArgs = CheckedApply<RemoveAssisted, DecoratedSignatureArgs>;
    
    using F1 = RegisterConstructorAsValueFactory<DecoratedSignature>;
    using F2 = EnsureProvidedTypes<TargetRequirements, CheckedApply<ExpandProvidersInParams, NonAssistedArgs>>;
    
    struct E {
      using Result = Error<FunctorSignatureDoesNotMatchErrorTag, AnnotatedSignature, ActualSignatureInInjectionTypedef>;
      void operator()(ComponentStorage&) {}
    };
    
    using type = Eval<std::conditional<!std::is_same<AnnotatedSignature, ActualSignatureInInjectionTypedef>::value,
                                       E,
                                       Apply<ComposeFunctors<F1, F2>, Comp>
                                       >>;
  };
};

template <typename TargetRequirements, typename C, typename AnnotatedSignature, typename... Args>
struct AutoRegisterFactoryHelper<TargetRequirements, false, false, C, AnnotatedSignature, Args...> {
  template <typename Comp>
  struct apply {
    struct type {
      using AnnotatedC        = Apply<SignatureType, AnnotatedSignature>;
      using Signature         = Apply<RemoveAnnotations, AnnotatedSignature>;
      using CFunctor          = std::function<Signature>;
      using AnnotatedCFunctor = Apply<CopyAnnotation, AnnotatedC, CFunctor>;
      using Result = Error<NoBindingFoundErrorTag, AnnotatedCFunctor>;
      void operator()(ComponentStorage&) {}
    };
  };
};

// Tries to register C by looking for a typedef called Inject inside C.
template <typename TargetRequirements, typename AnnotatedC>
struct AutoRegister : public AutoRegisterHelper<
      TargetRequirements,
      Apply<HasInjectAnnotation, Apply<RemoveAnnotations, AnnotatedC>>::value,
      AnnotatedC
>{};

template <typename TargetRequirements, typename C, typename... Args>
struct AutoRegister<TargetRequirements, std::function<C(Args...)>> {
  template <typename Comp>
  struct apply {
    using F1 = AutoRegisterFactoryHelper<
        TargetRequirements,
        Apply<HasInterfaceBinding, C, typename Comp::InterfaceBindings>::value,
        Apply<HasInjectAnnotation, C>::value,
        C,
        C(Args...),
        Apply<RemoveAnnotations, Args>...>;
    using type = Apply<F1, Comp>;
  };
};    

template <typename TargetRequirements, typename C, typename... Args>
struct AutoRegister<TargetRequirements, std::function<std::unique_ptr<C>(Args...)>> {
  template <typename Comp>
  struct apply {
    using F1 = AutoRegisterFactoryHelper<
      TargetRequirements,
      Apply<HasInterfaceBinding, C, typename Comp::InterfaceBindings>::value,
      false,
      std::unique_ptr<C>,
      std::unique_ptr<C>(Args...),
      Apply<RemoveAnnotations, Args>...>;
    using type = Apply<F1, Comp>;
  };
};

template <typename TargetRequirements, typename Annotation, typename C, typename... Args>
struct AutoRegister<TargetRequirements, fruit::Annotated<Annotation, std::function<C(Args...)>>> {
  template <typename Comp>
  struct apply {
    using F1 = AutoRegisterFactoryHelper<
        TargetRequirements,
        Apply<HasInterfaceBinding, C, typename Comp::InterfaceBindings>::value,
        Apply<HasInjectAnnotation, C>::value,
        C,
        fruit::Annotated<Annotation, C>(Args...),
        Apply<RemoveAnnotations, Args>...>;
    using type = Apply<F1, Comp>;
  };
};    

template <typename TargetRequirements, typename Annotation, typename C, typename... Args>
struct AutoRegister<TargetRequirements, fruit::Annotated<Annotation, std::function<std::unique_ptr<C>(Args...)>>> {
  template <typename Comp>
  struct apply {
    using F1 = AutoRegisterFactoryHelper<
      TargetRequirements,
      Apply<HasInterfaceBinding, C, typename Comp::InterfaceBindings>::value,
      false,
      std::unique_ptr<C>,
      fruit::Annotated<Annotation, std::unique_ptr<C>>(Args...),
      Apply<RemoveAnnotations, Args>...>;
    using type = Apply<F1, Comp>;
  };
};

template <typename TargetRequirements, bool is_already_provided_or_in_target_requirements, bool has_interface_binding, typename AnnotatedC>
struct EnsureProvidedTypeHelper;

// Already provided or in target requirements, ok.
template <typename TargetRequirements, bool unused, typename AnnotatedC>
struct EnsureProvidedTypeHelper<TargetRequirements, true, unused, AnnotatedC> : public Identity {};

// Has an interface binding.
template <typename TargetRequirements, typename AnnotatedI>
struct EnsureProvidedTypeHelper<TargetRequirements, false, true, AnnotatedI> {
  template <typename Comp>
  struct apply {
    using AnnotatedC = Apply<GetInterfaceBinding, AnnotatedI, typename Comp::InterfaceBindings>;
    using F1 = ProcessInterfaceBinding<AnnotatedI, AnnotatedC>;
    using F2 = EnsureProvidedType<TargetRequirements, AnnotatedC>;
    using type = Apply<ComposeFunctors<F1, F2>, Comp>;
  };
};

// Not yet provided, nor in target requirements, nor in InterfaceBindings. Try auto-registering.
template <typename TargetRequirements, typename AnnotatedC>
struct EnsureProvidedTypeHelper<TargetRequirements, false, false, AnnotatedC> : public AutoRegister<TargetRequirements, AnnotatedC> {};

template <typename TargetRequirements, typename AnnotatedT>
struct EnsureProvidedType {
  template <typename Comp>
  struct apply {
    using AnnotatedC = Apply<NormalizeType, AnnotatedT>;
    using F1 = EnsureProvidedTypeHelper<TargetRequirements,
                                        Apply<IsInVector, AnnotatedC, typename Comp::Ps>::value
                                        || Apply<IsInVector, AnnotatedC, TargetRequirements>::value,
                                        Apply<HasInterfaceBinding, AnnotatedC, typename Comp::InterfaceBindings>::value,
                                        AnnotatedC>;
    using type = Apply<F1, Comp>;
  };
};

// General case, empty list.
template <typename TargetRequirements, typename L>
struct EnsureProvidedTypes : public Identity {
  FruitStaticAssert(Apply<IsEmptyVector, L>::value, "Implementation error");
};

template <typename TargetRequirements, typename... AnnotatedTs>
struct EnsureProvidedTypes<TargetRequirements, Vector<None, AnnotatedTs...>> 
  : public EnsureProvidedTypes<TargetRequirements, Vector<AnnotatedTs...>> {
};


template <typename TargetRequirements, typename AnnotatedT, typename... AnnotatedTs>
struct EnsureProvidedTypes<TargetRequirements, Vector<AnnotatedT, AnnotatedTs...>> {
  template <typename Comp>
  struct apply {
    using F1 = EnsureProvidedType<TargetRequirements, AnnotatedT>;
    using F2 = EnsureProvidedTypes<TargetRequirements, Vector<AnnotatedTs...>>;
    using type = Apply<ComposeFunctors<F1, F2>, Comp>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_COMPONENT_FUNCTORS_DEFN_H
