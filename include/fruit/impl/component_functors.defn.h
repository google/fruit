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
template <typename I, typename C>
struct AddDeferredInterfaceBinding {
  template <typename Comp>
  struct apply {
    struct type {
      using Comp1 = ConsComp<typename Comp::Rs,
                             typename Comp::Ps,
                             typename Comp::Deps,
                             Apply<AddToSet,
                                   ConsInterfaceBinding<I, C>,
                                   typename Comp::InterfaceBindings>,
                             typename Comp::DeferredBindingFunctors>;
      // Note that we do NOT call AddProvidedType here. We'll only know the right required type
      // when the binding will be used.
      using Result = Eval<std::conditional<!std::is_base_of<I, C>::value,
                                           Error<NotABaseClassOfErrorTag, I, C>,
                                           Comp1>>;
      void operator()(ComponentStorage&) {}
    };
  };
};

template <typename I, typename C>
struct ProcessInterfaceBinding {
  template <typename Comp>
  struct apply {
    struct type {
      // This must be here (and not in AddDeferredInterfaceBinding) because the binding might be
      // used to bind functors instead, so we might never need to add C to the requirements.
      using Result = Apply<AddProvidedType, Comp, I, Vector<C>>;
      void operator()(ComponentStorage& storage) {
        storage.addBinding(InjectorStorage::createBindingDataForBind<I, C>());
      };
    };
  };
};

template <typename I, typename C>
struct AddInterfaceMultibinding {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Eval<std::conditional<!std::is_base_of<I, C>::value,
                                           Error<NotABaseClassOfErrorTag, I, C>,
                                           Apply<AddRequirements, Comp, Vector<C>>
                                           >>;
      void operator()(ComponentStorage& storage) {
        storage.addMultibinding(InjectorStorage::createMultibindingDataForBinding<I, C>());
      };
    };
  };
};

template <typename Lambda, typename OptionalI>
struct PostProcessRegisterProviderHelper {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForProvider<Lambda>());
    component.addCompressedBinding(InjectorStorage::createBindingDataForCompressedProvider<Lambda, OptionalI>());
  }
};

template <typename Lambda>
struct PostProcessRegisterProviderHelper<Lambda, None> {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForProvider<Lambda>());
  }
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerProvider() overloads in ComponentStorage.
template <typename Lambda>
struct PostProcessRegisterProvider {
  template <typename Comp>
  struct apply {
    struct type {
      using Signature = Apply<FunctionSignature, Lambda>;
      using C = CheckedApply<GetClassForType, CheckedApply<SignatureType, Signature>>;
      using OptionalI = CheckedApply<GetBindingToInterface, C, typename Comp::InterfaceBindings>;
      using Result = Comp;
      void operator()(ComponentStorage& storage) {
        PostProcessRegisterProviderHelper<Lambda, OptionalI>()(storage);
      }
    };
  };
};

template <typename Lambda>
struct PreProcessRegisterProvider {
  template <typename Comp>
  struct apply {
    struct type {
      using Signature = Apply<FunctionSignature, Lambda>;
      using C = CheckedApply<GetClassForType, CheckedApply<SignatureType, Signature>>;
      using OptionalI = CheckedApply<GetBindingToInterface, C, typename Comp::InterfaceBindings>;
      using CDeps = CheckedApply<ExpandProvidersInParams, 
                                CheckedApply<GetClassForTypeVector, CheckedApply<SignatureArgs, Signature>>>;
      using Result = CheckedApply<AddProvidedType, Comp, C, CDeps>;
      void operator()(ComponentStorage&) {}
    };
  };
};

// The registration is actually deferred until the PartialComponent is converted to a component.
template <typename Lambda>
struct DeferredRegisterProvider {
  template <typename Comp>
  struct apply {
    struct type {
      using Comp1 = Apply<AddDeferredBinding, Comp, PostProcessRegisterProvider<Lambda>>;
      using Comp2 = CheckedApply<GetResult, CheckedApply<PreProcessRegisterProvider<Lambda>, Comp1>>;
      using Result = Comp2;
      void operator()(ComponentStorage&) {}
    };
  };
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerMultibindingProvider() overloads in ComponentStorage.
template <typename Lambda>
struct RegisterMultibindingProvider {
  template <typename Comp>
  struct apply {
    struct type {
      using Args = CheckedApply<SignatureArgs, Apply<FunctionSignature, Lambda>>;
      using ArgSet = CheckedApply<ExpandProvidersInParams,
                                  CheckedApply<GetClassForTypeVector, Args>>;
      using Result = CheckedApply<AddRequirements, Comp, Args>;
      void operator()(ComponentStorage& storage) {
        storage.addMultibinding(InjectorStorage::createMultibindingDataForProvider<Lambda>());
      }
    };
  };
};

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

template <typename AnnotatedSignature,
          typename Lambda,
          typename InjectedSignature = Apply<InjectedSignatureForAssistedFactory, AnnotatedSignature>,
          typename RequiredSignature = Apply<RequiredSignatureForAssistedFactory, AnnotatedSignature>,
          typename InjectedArgs = Apply<RemoveAssisted, Apply<SignatureArgs, AnnotatedSignature>>,
          typename IndexSequence = GenerateIntSequence<
              Apply<VectorSize,
                  Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>
              >::value>
          >
struct RegisterFactory;

template <typename AnnotatedSignature, typename Lambda, typename C, typename... UserProvidedArgs, typename... AllArgs, typename... InjectedArgs, int... indexes>
struct RegisterFactory<AnnotatedSignature, Lambda, C(UserProvidedArgs...), C(AllArgs...), Vector<InjectedArgs...>, IntVector<indexes...>> {
  template <typename Comp>
  struct apply {
    using T = Apply<SignatureType, AnnotatedSignature>;
    using AnnotatedArgs = Apply<SignatureArgs, AnnotatedSignature>;
    using InjectedSignature = C(UserProvidedArgs...);
    using RequiredSignature = C(AllArgs...);
    using fun_t = std::function<InjectedSignature>;
    using FunDeps = Apply<ExpandProvidersInParams, Apply<GetClassForTypeVector, AnnotatedArgs>>;
    struct type {
      // The first is_same check is a bit of a hack, it's to make the F2/RealF2 split work in the caller (we need to allow Lambda to be a function type).
      using Result = Eval<Conditional<Lazy<Bool<!std::is_empty<Lambda>::value && !std::is_same<Lambda, Apply<FunctionSignature, Lambda>>::value>>,
                                      Lazy<Error<LambdaWithCapturesErrorTag, Lambda>>,
                                      Conditional<Lazy<Bool<!std::is_same<RequiredSignature, Apply<FunctionSignature, Lambda>>::value>>,
                                                  Lazy<Error<FunctorSignatureDoesNotMatchErrorTag, RequiredSignature, Apply<FunctionSignature, Lambda>>>,
                                                  Conditional<Lazy<Bool<std::is_pointer<T>::value>>,
                                                              Lazy<Error<FactoryReturningPointerErrorTag, AnnotatedSignature>>,
                                                              Apply<LazyFunctor<AddProvidedType>, Lazy<Comp>, Lazy<fun_t>, Lazy<FunDeps>>
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
                                                                Apply<NumAssistedBefore, Int<indexes>, AnnotatedArgs>::value,
                                                                indexes - Apply<NumAssistedBefore, Int<indexes>, AnnotatedArgs>::value,
                                                                GetNthType<indexes, AnnotatedArgs>,
                                                                decltype(injected_args),
                                                                decltype(user_provided_args)
                                                            >()(injected_args, user_provided_args)
                                                            ...);
          };
          return fun_t(object_provider);
        };  
        storage.addBinding(InjectorStorage::createBindingDataForProvider<decltype(function_provider)>());
      }
    };
  };
};

template <typename Signature>
struct PostProcessRegisterConstructor;

template <typename Signature, typename OptionalI>
struct PostProcessRegisterConstructorHelper {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForConstructor<Signature>());
    component.addCompressedBinding(InjectorStorage::createBindingDataForCompressedConstructor<Signature, OptionalI>());
  }
};

template <typename Signature>
struct PostProcessRegisterConstructorHelper<Signature, None> {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForConstructor<Signature>());
  }
};

template <typename T, typename... Args>
struct PostProcessRegisterConstructor<T(Args...)> {
  template <typename Comp>
  struct apply {
    using C = Apply<GetClassForType, T>;
    struct type {
      using Result = Comp;
      void operator()(ComponentStorage& storage) {
        PostProcessRegisterConstructorHelper<T(Args...), Apply<GetBindingToInterface, C, typename Comp::InterfaceBindings>>()(storage);
      }
    };
  };
};

// We need to extract this to make the computation of SignatureType lazy, otherwise it'd be evaluated in DeferredRegisterConstructor even when it should not be.
struct AddProvidedTypeForRegisterConstructor {
  template <typename Signature, typename Comp>
  struct apply {
    using T = Apply<SignatureType, Signature>;
    using Args = Apply<SignatureArgs, Signature>;
    
    using C = Apply<GetClassForType, T>;
    using CDeps = Apply<ExpandProvidersInParams, Apply<GetClassForTypeVector, Args>>;
    using type = Apply<AddProvidedType, Comp, C, CDeps>;
  };
};

// We need to extract this to make the computation of SignatureType lazy, otherwise it'd be evaluated in DeferredRegisterConstructor even when it should not be.
struct ConstructNoConstructorMatchingInjectSignatureError {
  template <typename Signature>
  struct apply {
    using type = Error<NoConstructorMatchingInjectSignatureErrorTag, Apply<SignatureType, Signature>, Signature>;
  };
};

template <typename Signature>
struct PreProcessRegisterConstructor {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Eval<Conditional<Lazy<Apply<Not, Apply<IsValidSignature, Signature>>>,
                                      Lazy<Error<NotASignatureErrorTag, Signature>>,
                                      Conditional<Apply<LazyFunctor<Not>,
                                                        Apply<LazyFunctor<IsConstructibleWithVector>,
                                                              Apply<LazyFunctor<SignatureType>, Lazy<Signature>>,
                                                              Apply<LazyFunctor<SignatureArgs>, Lazy<Signature>>
                                                              >
                                                        >,
                                                  Apply<LazyFunctor<ConstructNoConstructorMatchingInjectSignatureError>, Lazy<Signature>>,
                                                  Apply<LazyFunctor<AddProvidedTypeForRegisterConstructor>,
                                                        Lazy<Signature>,
                                                        Lazy<Comp>
                                                        >
                                                  >
                                      >>;
      void operator()(ComponentStorage&) {}
    };
  };
};

template <typename Signature>
struct DeferredRegisterConstructor {
  template <typename Comp>
  struct apply {
    struct type {
      using Comp1 = Apply<AddDeferredBinding,
                          Comp,
                          PostProcessRegisterConstructor<Signature>
                          >;
      using Comp2 = CheckedApply<GetResult, CheckedApply<PreProcessRegisterConstructor<Signature>, Comp1>>;
      using Result = Comp2;
      void operator()(ComponentStorage&) {}
    };
  };
};

template <typename C>
struct RegisterInstance {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Apply<AddProvidedType, Comp, C, Vector<>>;
      void operator()(ComponentStorage& storage, C& instance) {
        storage.addBinding(InjectorStorage::createBindingDataForBindInstance<C>(instance));
      };
    };
  };
};

template <typename C>
struct AddInstanceMultibinding {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Comp;
      void operator()(ComponentStorage& storage, C& instance) {
        storage.addMultibinding(InjectorStorage::createMultibindingDataForInstance<C>(instance));
      };
    };
  };
};

template <typename C>
struct AddInstanceMultibindings {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Comp;
      void operator()(ComponentStorage& storage, std::vector<C>& instances) {
        for (C& instance : instances) {
          storage.addMultibinding(InjectorStorage::createMultibindingDataForInstance<C>(instance));
        }
      };
    };
  };
};

template <typename AnnotatedSignature,
          typename RequiredSignature = Apply<ConstructSignature,
                                             Apply<SignatureType, AnnotatedSignature>,
                                             Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>>>
struct RegisterConstructorAsValueFactory;

template <typename AnnotatedSignature, typename T, typename... Args>
struct RegisterConstructorAsValueFactory<AnnotatedSignature, T(Args...)> {
  template <typename Comp>
  struct apply {
    using RequiredSignature = T(Args...);
    using F1 = RegisterFactory<AnnotatedSignature, RequiredSignature>;
    using Op = Apply<F1, Comp>;
    struct type {
      using Result = typename Op::Result;
      void operator()(ComponentStorage& storage) {
        auto provider = [](Args... args) {
          return T(std::forward<Args>(args)...);
        };
        using RealF1 = RegisterFactory<AnnotatedSignature, decltype(provider)>;
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
template <typename TargetRequirements, typename C>
struct AutoRegister;

template <typename TargetRequirements, bool has_inject_annotation, typename C>
struct AutoRegisterHelper;

// C has an Inject typedef, use it.
template <typename TargetRequirements, typename C>
struct AutoRegisterHelper<TargetRequirements, true, C> {
  template <typename Comp>
  struct apply {
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

template <typename TargetRequirements, typename C>
struct AutoRegisterHelper<TargetRequirements, false, C> {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Error<NoBindingFoundErrorTag, C>;
      void operator()(ComponentStorage&) {}
    };
  };
};

template <typename TargetRequirements, bool has_interface_binding, bool has_inject_annotation, typename C, typename... Args>
struct AutoRegisterFactoryHelper;

// I has an interface binding, use it and look for a factory that returns the type that I is bound to.
template <typename TargetRequirements, bool unused, typename I, typename... Argz>
struct AutoRegisterFactoryHelper<TargetRequirements, true, unused, std::unique_ptr<I>, Argz...> {
  template <typename Comp>
  struct apply {
    using C = Apply<GetInterfaceBinding, I, typename Comp::InterfaceBindings>;
    using original_function_t = std::function<std::unique_ptr<C>(Argz...)>;
    using function_t = std::function<std::unique_ptr<I>(Argz...)>;
    
    using F1 = EnsureProvidedType<TargetRequirements, original_function_t>;
    using F2 = PreProcessRegisterProvider<function_t*(original_function_t*)>;
    using F3 = PostProcessRegisterProvider<function_t*(original_function_t*)>;
    using Op = Apply<ComposeFunctors<F1, F2, F3>, Comp>;
    struct type {
      using Result = typename Op::Result;
      void operator()(ComponentStorage& storage) {
        auto provider = [](original_function_t* fun) {
          return new function_t([=](Argz... args) {
            C* c = (*fun)(args...).release();
            I* i = static_cast<I*>(c);
            return std::unique_ptr<I>(i);
          });
        };
        using RealF2 = PreProcessRegisterProvider<decltype(provider)>;
        using RealF3 = PostProcessRegisterProvider<decltype(provider)>;
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
// Bind std::function<unique_ptr<C>(Args...)> to std::function<C(Args...)>.
template <typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<TargetRequirements, false, false, std::unique_ptr<C>, Argz...> {
  template <typename Comp>
  struct apply {
    using original_function_t = std::function<C(Argz...)>;
    using function_t = std::function<std::unique_ptr<C>(Argz...)>;
    
    using F1 = EnsureProvidedType<TargetRequirements, original_function_t>;
    using F2 = PreProcessRegisterProvider<function_t*(original_function_t*)>;
    using F3 = PostProcessRegisterProvider<function_t*(original_function_t*)>;
    using Op = Apply<ComposeFunctors<F1, F2, F3>, Comp>;
    struct type {
      // If we are about to report a NoBindingFound error for std::function<C(Argz...)>, report one for std::function<std::unique_ptr<C>(Argz...)> instead,
      // otherwise we'd report an error about a type that the user doesn't expect.
      using Result = Eval<std::conditional<std::is_same<typename Op::Result, Error<NoBindingFoundErrorTag, std::function<C(Argz...)>>>::value,
                                           Error<NoBindingFoundErrorTag, std::function<std::unique_ptr<C>(Argz...)>>,
                                           typename Op::Result
                                           >> ;
      void operator()(ComponentStorage& storage) {
        auto provider = [](original_function_t* fun) {
          return new function_t([=](Argz... args) {
            C* c = new C((*fun)(args...));
            return std::unique_ptr<C>(c);
          });
        };
        using RealF2 = PreProcessRegisterProvider<decltype(provider)>;
        using RealF3 = PostProcessRegisterProvider<decltype(provider)>;
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
template <typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<TargetRequirements, false, true, std::unique_ptr<C>, Argz...> {
  template <typename Comp>
  struct apply;
};

// C has an Inject typedef, use it. Value (not unique_ptr) case.
// TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
template <typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<TargetRequirements, false, true, C, Argz...> {
  template <typename Comp>
  struct apply {
    using AnnotatedSignature = Apply<GetInjectAnnotation, C>;
    using AnnotatedSignatureArgs = CheckedApply<SignatureArgs, AnnotatedSignature>;
    using ExpectedSignatureInInjectionTypedef = CheckedApply<ConstructSignature, C, Vector<Argz...>>;
    using ActualSignatureInInjectionTypedef = CheckedApply<ConstructSignature, C, CheckedApply<RemoveNonAssisted, AnnotatedSignatureArgs>>;
    using NonAssistedArgs = CheckedApply<RemoveAssisted, AnnotatedSignatureArgs>;
    
    using F1 = RegisterConstructorAsValueFactory<AnnotatedSignature>;
    using F2 = EnsureProvidedTypes<TargetRequirements, CheckedApply<ExpandProvidersInParams, NonAssistedArgs>>;
    
    struct E {
      using Result = Error<FunctorSignatureDoesNotMatchErrorTag, ExpectedSignatureInInjectionTypedef, ActualSignatureInInjectionTypedef>;
      void operator()(ComponentStorage&) {}
    };
    
    using type = Eval<std::conditional<!std::is_same<ExpectedSignatureInInjectionTypedef, ActualSignatureInInjectionTypedef>::value,
                                       E,
                                       Apply<ComposeFunctors<F1, F2>, Comp>
                                       >>;
  };
};

template <typename TargetRequirements, typename C, typename... Args>
struct AutoRegisterFactoryHelper<TargetRequirements, false, false, C, Args...> {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Error<NoBindingFoundErrorTag, std::function<C(Args...)>>;
      void operator()(ComponentStorage&) {}
    };
  };
};

// Tries to registers C by looking for a typedef called Inject inside C.
template <typename TargetRequirements, typename C>
struct AutoRegister : public AutoRegisterHelper<
      TargetRequirements,
      Apply<HasInjectAnnotation, C>::value,
      C
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
        Args...>;
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
      Args...>;
    using type = Apply<F1, Comp>;
  };
};

template <typename TargetRequirements, bool is_already_provided_or_in_target_requirements, bool has_interface_binding, typename C>
struct EnsureProvidedTypeHelper;

// Already provided or in target requirements, ok.
template <typename TargetRequirements, bool unused, typename C>
struct EnsureProvidedTypeHelper<TargetRequirements, true, unused, C> : public Identity {};

// Has an interface binding.
template <typename TargetRequirements, typename I>
struct EnsureProvidedTypeHelper<TargetRequirements, false, true, I> {
  template <typename Comp>
  struct apply {
    using C = Apply<GetInterfaceBinding, I, typename Comp::InterfaceBindings>;
    using F1 = ProcessInterfaceBinding<I, C>;
    using F2 = EnsureProvidedType<TargetRequirements, C>;
    using type = Apply<ComposeFunctors<F1, F2>, Comp>;
  };
};

// Not yet provided, nor in target requirements, nor in InterfaceBindings. Try auto-registering.
template <typename TargetRequirements, typename C>
struct EnsureProvidedTypeHelper<TargetRequirements, false, false, C> : public AutoRegister<TargetRequirements, C> {};

template <typename TargetRequirements, typename T>
struct EnsureProvidedType {
  template <typename Comp>
  struct apply {
    using C = Apply<GetClassForType, T>;
    using F1 = EnsureProvidedTypeHelper<TargetRequirements,
                                        Apply<IsInVector, C, typename Comp::Ps>::value
                                        || Apply<IsInVector, C, TargetRequirements>::value,
                                        Apply<HasInterfaceBinding, C, typename Comp::InterfaceBindings>::value,
                                        C>;
    using type = Apply<F1, Comp>;
  };
};

// General case, empty list.
template <typename TargetRequirements, typename L>
struct EnsureProvidedTypes : public Identity {
  FruitStaticAssert(Apply<IsEmptyVector, L>::value, "Implementation error");
};

template <typename TargetRequirements, typename... Ts>
struct EnsureProvidedTypes<TargetRequirements, Vector<None, Ts...>> 
  : public EnsureProvidedTypes<TargetRequirements, Vector<Ts...>> {
};


template <typename TargetRequirements, typename T, typename... Ts>
struct EnsureProvidedTypes<TargetRequirements, Vector<T, Ts...>> {
  template <typename Comp>
  struct apply {
    using F1 = EnsureProvidedType<TargetRequirements, T>;
    using F2 = EnsureProvidedTypes<TargetRequirements, Vector<Ts...>>;
    using type = Apply<ComposeFunctors<F1, F2>, Comp>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_COMPONENT_FUNCTORS_DEFN_H
