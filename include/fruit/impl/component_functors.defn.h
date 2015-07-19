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
#include "injection_debug_errors.h"
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

// Call(ComponentFunctor(F, Args...), Comp)
// is equivalent to:
// F(Comp, Args...)
struct ComponentFunctor {
  template <typename F, typename... Args>
  struct apply {
    struct type {
      template <typename Comp>
      struct apply {
        using type = F(Comp, Args...);
      };
    };
  };
};

struct NoOpComponentFunctor {
  using Result = void;
  template <typename... Types>
  void operator()(ComponentStorage&, const Types&...) {}
};

struct ComponentFunctorIdentity {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Comp;
      void operator()(ComponentStorage&) {}
    };
  };
};

struct Compose2ComponentFunctors {
  template <typename F1, typename F2>
  struct apply {
    struct type {
      template <typename Comp>
      struct apply {
        using Op1 = F1(Comp);
        using Op2 = F2(GetResult(Op1));
        struct Op {
          using Result = Eval<GetResult(Op2)>;
          void operator()(ComponentStorage& storage) {
            Eval<Op1>()(storage);
            Eval<Op2>()(storage);
          }
        };
        using type = PropagateError(Op1,
                     PropagateError(Op2,
                     Op));
      };
    };
  };
};

// ComposeFunctors(F1,..,Fn) returns a functor that executes F1,..,Fn in order (stopping at the
// first Error).
struct ComposeFunctors {
  template <typename... Functors>
  struct apply {    
    using type = Fold(Compose2ComponentFunctors, ComponentFunctorIdentity, Functors...);
  };
};

struct EnsureProvidedType;

struct EnsureProvidedTypes;

// Doesn't actually bind in ComponentStorage. The binding is added later (if needed) using ProcessInterfaceBinding.
struct AddDeferredInterfaceBinding {
  template <typename Comp, typename AnnotatedI, typename AnnotatedC>
  struct apply {
    using Comp1 = ConsComp(typename Comp::RsSuperset,
                           typename Comp::Ps,
                           typename Comp::Deps,
                           AddToSet(typename Comp::InterfaceBindings,
                                    Pair<AnnotatedI, AnnotatedC>),
                           typename Comp::DeferredBindingFunctors);
    struct Op {
      // Note that we do NOT call AddProvidedType here. We'll only know the right required type
      // when the binding will be used.
      using Result = Eval<Comp1>;
      void operator()(ComponentStorage&) {}
    };
    using I = RemoveAnnotations(AnnotatedI);
    using C = RemoveAnnotations(AnnotatedC);
    using type = If(Not(IsBaseOf(I, C)),
                    ConstructError(NotABaseClassOfErrorTag, I, C),
                 Op);
  };
};

struct ProcessInterfaceBinding {
  template <typename Comp, typename AnnotatedI, typename AnnotatedC>
  struct apply {
    using R = AddProvidedType(Comp, AnnotatedI, Vector<AnnotatedC>);
    struct Op {
      // This must be here (and not in AddDeferredInterfaceBinding) because the binding might be
      // used to bind functors instead, so we might never need to add C to the requirements.
      using Result = Eval<R>;
      void operator()(ComponentStorage& storage) {
        storage.addBinding(InjectorStorage::createBindingDataForBind<
            UnwrapType<AnnotatedI>, UnwrapType<AnnotatedC>>());
      };
    };
    using type = PropagateError(R,
                 Op);
  };
};

struct AddInterfaceMultibinding {
  template <typename Comp, typename AnnotatedI, typename AnnotatedC>
  struct apply {
    using I = RemoveAnnotations(AnnotatedI);
    using C = RemoveAnnotations(AnnotatedC);
    using R = AddRequirements(Comp, Vector<AnnotatedC>);
    struct Op {
      using Result = Eval<R>;
      void operator()(ComponentStorage& storage) {
        storage.addMultibinding(InjectorStorage::createMultibindingDataForBinding<
            UnwrapType<AnnotatedI>, UnwrapType<AnnotatedC>>());
      };
    };
    using type = If(Not(IsBaseOf(I, C)),
                    ConstructError(NotABaseClassOfErrorTag, I, C),
                 Op);
  };
};

template <typename AnnotatedSignature, typename Lambda, typename OptionalAnnotatedI>
struct PostProcessRegisterProviderHelper;

template <typename AnnotatedSignature, typename Lambda, typename AnnotatedI>
struct PostProcessRegisterProviderHelper;

template <typename AnnotatedSignature, typename Lambda, typename AnnotatedI>
struct PostProcessRegisterProviderHelper<AnnotatedSignature, Lambda, Type<AnnotatedI>> {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForProvider<
        AnnotatedSignature, Lambda>());
    component.addCompressedBinding(InjectorStorage::createBindingDataForCompressedProvider<
        AnnotatedSignature, Lambda, AnnotatedI>());
  }
};

template <typename AnnotatedSignature, typename Lambda>
struct PostProcessRegisterProviderHelper<AnnotatedSignature, Lambda, None> {
  inline void operator()(ComponentStorage& component) {
    component.addBinding(InjectorStorage::createBindingDataForProvider<
        AnnotatedSignature, Lambda>());
  }
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerProvider() overloads in ComponentStorage.
struct PostProcessRegisterProvider {
  template <typename Comp, typename AnnotatedSignature, typename Lambda>
  struct apply {
    using AnnotatedC = NormalizeType(SignatureType(AnnotatedSignature));
    using OptionalAnnotatedI = FindInMap(typename Comp::InterfaceBindings, AnnotatedC);
    struct Op {
      using Result = Comp;
      void operator()(ComponentStorage& storage) {
        PostProcessRegisterProviderHelper<
            UnwrapType<AnnotatedSignature>, UnwrapType<Lambda>, Eval<OptionalAnnotatedI>>()(storage);
      }
    };
    using type = Op;
  };
};

struct PreProcessRegisterProvider {
  template <typename Comp, typename AnnotatedSignature, typename Lambda>
  struct apply {
    using Signature = RemoveAnnotationsFromSignature(AnnotatedSignature);
    using SignatureFromLambda = FunctionSignature(Lambda);
    
    using AnnotatedC = NormalizeType(SignatureType(AnnotatedSignature));
    using OptionalI = FindInMap(typename Comp::InterfaceBindings, AnnotatedC);
    using AnnotatedCDeps = ExpandProvidersInParams(NormalizeTypeVector(SignatureArgs(AnnotatedSignature)));
    using R = AddProvidedType(Comp, AnnotatedC, AnnotatedCDeps);
    using type = PropagateError(SignatureFromLambda,
                 If(Not(IsSame(Signature, SignatureFromLambda)),
                   ConstructError(AnnotatedSignatureDifferentFromLambdaSignatureErrorTag, Signature, SignatureFromLambda),
                 PropagateError(R,
                 ComponentFunctorIdentity(R))));
  };
};

// The registration is actually deferred until the PartialComponent is converted to a component.
struct DeferredRegisterProviderWithAnnotations {
  template <typename Comp, typename AnnotatedSignature, typename Lambda>
  struct apply {
    using Comp1 = AddDeferredBinding(Comp, 
                                     ComponentFunctor(PostProcessRegisterProvider, AnnotatedSignature, Lambda));
    using Op = PreProcessRegisterProvider(Comp1, AnnotatedSignature, Lambda);
    using type = PropagateError(Comp1,
                 Op);
  };
};

// The registration is actually deferred until the PartialComponent is converted to a component.
struct DeferredRegisterProvider {
  template <typename Comp, typename Lambda>
  struct apply {
    using Comp1 = DeferredRegisterProviderWithAnnotations(Comp, FunctionSignature(Lambda), Lambda);
#ifdef FRUIT_DEEP_TEMPLATE_INSTANTIATION_STACKTRACES_FOR_ERRORS
    using type = If(IsSignature(Lambda),
                    // TODO: Check if this hack is still needed.
                    // Hack, assume that we're evaluating an overload (of a method in PartialComponent) that will never be chosen.
                    // Evaluating LazyOp in this case could report errors that would not surface otherwise.
                    Comp,
                    Comp1);
#else
    using type = Comp1;
#endif
  };
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerMultibindingProvider() overloads in ComponentStorage.
struct RegisterMultibindingProviderWithAnnotations {
  template <typename Comp, typename AnnotatedSignature, typename Lambda>
  struct apply {
    using Signature = RemoveAnnotationsFromSignature(AnnotatedSignature);
    using SignatureFromLambda = FunctionSignature(Lambda);
    
    using AnnotatedArgs = SignatureArgs(AnnotatedSignature);
    using AnnotatedArgVector = ExpandProvidersInParams(NormalizeTypeVector(AnnotatedArgs));
    using R = AddRequirements(Comp, AnnotatedArgVector);
    struct Op {
      using Result = Eval<R>;
      void operator()(ComponentStorage& storage) {
        auto multibindingData = InjectorStorage::createMultibindingDataForProvider<
            UnwrapType<AnnotatedSignature>, UnwrapType<Lambda>>();
        storage.addMultibinding(multibindingData);
      }
    };
    using type = PropagateError(SignatureFromLambda,
                 If(Not(IsSame(Signature, SignatureFromLambda)),
                    ConstructError(AnnotatedSignatureDifferentFromLambdaSignatureErrorTag, Signature, SignatureFromLambda),
                 PropagateError(R,
                 Op)));
  };
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerMultibindingProvider() overloads in ComponentStorage.
struct RegisterMultibindingProvider {
  template <typename Comp, typename Lambda>
  struct apply {
    using Comp1 = RegisterMultibindingProviderWithAnnotations(Comp, FunctionSignature(Lambda), Lambda);
#ifdef FRUIT_DEEP_TEMPLATE_INSTANTIATION_STACKTRACES_FOR_ERRORS
    using type = If(IsSignature(Lambda),
                    // TODO: Check if this is still needed.
                    // Hack, assume that we're evaluating an overload (of a method in PartialComponent) that will never be chosen.
                    // Evaluating LazyOp in this case could report errors that would not surface otherwise.
                    Comp,
                    Comp1);
#else
    using type = Comp1;
#endif
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

struct RegisterFactoryHelper {
  
  template <typename Comp,
            typename DecoratedSignature,
            typename Lambda,
            // std::function<InjectedSignature> is the injected type (possibly with an Annotation<> wrapping it)
            typename InjectedSignature,
            typename RequiredLambdaSignature,
            typename InjectedAnnotatedArgs,
            // The types that are injected, unwrapped from any Annotation<>.
            typename InjectedArgs,
            typename IndexSequence>
  struct apply;
  
  template <typename Comp, typename DecoratedSignature, typename Lambda, typename NakedC, 
      typename... NakedUserProvidedArgs, typename... NakedAllArgs, typename... InjectedAnnotatedArgs,
      typename... NakedInjectedArgs, int... indexes>
  struct apply<Comp, DecoratedSignature, Lambda, Type<NakedC(NakedUserProvidedArgs...)>,
               Type<NakedC(NakedAllArgs...)>, Vector<InjectedAnnotatedArgs...>,
               Vector<Type<NakedInjectedArgs>...>, Vector<Int<indexes>...>> {
    // Here we call "decorated" the types that might be wrapped in Annotated<> or Assisted<>,
    // while we call "annotated" the ones that might only be wrapped in Annotated<> (but not Assisted<>).
    using AnnotatedT = SignatureType(DecoratedSignature);
    using T          = RemoveAnnotations(AnnotatedT);
    using DecoratedArgs = SignatureArgs(DecoratedSignature);
    using NakedInjectedSignature = NakedC(NakedUserProvidedArgs...);
    using NakedRequiredSignature = NakedC(NakedAllArgs...);
    using NakedFunctor = std::function<NakedInjectedSignature>;
    // This is usually the same as Functor, but this might be annotated.
    using AnnotatedFunctor = CopyAnnotation(AnnotatedT, Type<NakedFunctor>);
    using FunctorDeps = NormalizeTypeVector(Vector<InjectedAnnotatedArgs...>);
    using R = AddProvidedType(Comp, AnnotatedFunctor, FunctorDeps);
    struct Op {
      using Result = Eval<R>;
      void operator()(ComponentStorage& storage) {
        auto function_provider = [](NakedInjectedArgs... args) {
          // TODO: Using auto and make_tuple here results in a GCC segfault with GCC 4.8.1.
          // Check this on later versions and consider filing a bug.
          std::tuple<NakedInjectedArgs...> injected_args(args...);
          auto object_provider = [injected_args](NakedUserProvidedArgs... params) mutable {
            auto user_provided_args = std::tie(params...);
            // These are unused if they are 0-arg tuples. Silence the unused-variable warnings anyway.
            (void) injected_args;
            (void) user_provided_args;
            
            return LambdaInvoker::invoke<UnwrapType<Lambda>, NakedAllArgs...>(
              GetAssistedArg<Eval<NumAssistedBefore(Int<indexes>, DecoratedArgs)>::value,
                             indexes - 1 - Eval<NumAssistedBefore(Int<indexes>, DecoratedArgs)>::value,
                             // Note that the Assisted<> wrapper (if any) remains, we just remove any wrapping Annotated<>.
                             UnwrapType<Eval<RemoveAnnotations(GetNthType(Int<indexes>, DecoratedArgs))>>,
                             decltype(injected_args),
                             decltype(user_provided_args)
                             >()(injected_args, user_provided_args)
              ...);
          };
          return NakedFunctor(object_provider);
        };
        storage.addBinding(InjectorStorage::createBindingDataForProvider<
            UnwrapType<Eval<ConsSignatureWithVector(AnnotatedFunctor, Vector<InjectedAnnotatedArgs...>)>>,
            decltype(function_provider)>());
      }
    };
    // The first is_same check is a bit of a hack, it's to make the F2/RealF2 split work in the caller (we need to allow Lambda to be a function type).
    using type = If(Not(Or(IsEmpty(Lambda), IsSame(Lambda, FunctionSignature(Lambda)))),
                    ConstructError(LambdaWithCapturesErrorTag, Lambda),
                 PropagateError(FunctionSignature(Lambda),
                 If(Not(IsSame(Type<NakedRequiredSignature>, FunctionSignature(Lambda))),
                    ConstructError(FunctorSignatureDoesNotMatchErrorTag, Type<NakedRequiredSignature>, FunctionSignature(Lambda)),
                 If(IsPointer(T),
                    ConstructError(FactoryReturningPointerErrorTag, DecoratedSignature),
                 PropagateError(R,
                 Op)))));
  };
};

struct RegisterFactory {
  template <typename Comp, typename DecoratedSignature, typename Lambda>
  struct apply {
    using type = RegisterFactoryHelper(Comp,
                                       DecoratedSignature,
                                       Lambda,
                                       InjectedSignatureForAssistedFactory(DecoratedSignature),
                                       RequiredLambdaSignatureForAssistedFactory(DecoratedSignature),
                                       RemoveAssisted(SignatureArgs(DecoratedSignature)),
                                       RemoveAnnotationsFromVector(RemoveAssisted(SignatureArgs(DecoratedSignature))),
                                       GenerateIntSequence(
                                          VectorSize(RequiredLambdaArgsForAssistedFactory(DecoratedSignature))));
  };
};

struct PostProcessRegisterConstructor;

template <typename AnnotatedSignature, typename OptionalAnnotatedI>
struct PostProcessRegisterConstructorHelper;

template <typename AnnotatedSignature, typename AnnotatedI>
struct PostProcessRegisterConstructorHelper;

template <typename AnnotatedSignature, typename AnnotatedI>
struct PostProcessRegisterConstructorHelper<AnnotatedSignature, Type<AnnotatedI>> {
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

struct PostProcessRegisterConstructor {
  template <typename Comp, typename AnnotatedSignature>
  struct apply {
    struct type {
      using AnnotatedC = NormalizeType(SignatureType(AnnotatedSignature));
      using Result = Comp;
      void operator()(ComponentStorage& storage) {
        PostProcessRegisterConstructorHelper<
            UnwrapType<AnnotatedSignature>,
            Eval<FindInMap(typename Comp::InterfaceBindings, AnnotatedC)>
            >()(storage);
      }
    };
  };
};

struct PreProcessRegisterConstructor {
  template <typename Comp, typename AnnotatedSignature>
  struct apply {
    using Signature = RemoveAnnotationsFromSignature(AnnotatedSignature);
    using C = SignatureType(Signature);
    using Args = SignatureArgs(Signature);
    using AnnotatedT    = SignatureType(AnnotatedSignature);
    using AnnotatedArgs = SignatureArgs(AnnotatedSignature);
    using AnnotatedC = NormalizeType(AnnotatedT);
    using CDeps = ExpandProvidersInParams(NormalizeTypeVector(AnnotatedArgs));
    using R = AddProvidedType(Comp, AnnotatedC, CDeps);
    using type = If(Not(IsValidSignature(AnnotatedSignature)),
                    ConstructError(NotASignatureErrorTag, AnnotatedSignature),
                 If(Not(IsConstructibleWithVector(C, Args)),
                    ConstructError(NoConstructorMatchingInjectSignatureErrorTag, C, Signature),
                 PropagateError(R,
                 ComponentFunctorIdentity(R))));
  };
};

struct DeferredRegisterConstructor {
  template <typename Comp, typename AnnotatedSignature>
  struct apply {
    using Comp1 = AddDeferredBinding(Comp,
                                     ComponentFunctor(PostProcessRegisterConstructor, AnnotatedSignature));
    using Op = PreProcessRegisterConstructor(Comp1, AnnotatedSignature);
    using type = PropagateError(Comp1,
                 Op);
  };
};

struct RegisterInstance {
  template <typename Comp, typename AnnotatedC, typename C>
  struct apply {
    using R = AddProvidedType(Comp, AnnotatedC, Vector<>);
    struct Op {
      using Result = Eval<R>;
      void operator()(ComponentStorage& storage, UnwrapType<C>& instance) {
        storage.addBinding(InjectorStorage::createBindingDataForBindInstance<
            UnwrapType<AnnotatedC>, UnwrapType<C>>(instance));
      };
    };
    using type = PropagateError(R,
                 Op);
  };
};

struct AddInstanceMultibinding {
  template <typename Comp, typename AnnotatedC, typename C>
  struct apply {
    struct type {
      using Result = Comp;
      void operator()(ComponentStorage& storage, UnwrapType<C>& instance) {
        auto multibindingData = InjectorStorage::createMultibindingDataForInstance<
            UnwrapType<AnnotatedC>, UnwrapType<C>>(instance);
        storage.addMultibinding(multibindingData);
      };
    };
  };
};

struct AddInstanceMultibindings {
  template <typename Comp, typename AnnotatedC, typename C>
  struct apply {
    struct type {
      using Result = Comp;
      using NakedC = UnwrapType<C>;
      void operator()(ComponentStorage& storage, std::vector<NakedC>& instances) {
        for (NakedC& instance : instances) {
          auto multibindingData = InjectorStorage::createMultibindingDataForInstance<
                UnwrapType<AnnotatedC>, NakedC>(instance);
          storage.addMultibinding(multibindingData);
        }
      };
    };
  };
};

struct RegisterConstructorAsValueFactory {
  template<typename Comp, 
           typename DecoratedSignature, 
           typename RequiredSignature = 
               Eval<RequiredLambdaSignatureForAssistedFactory(DecoratedSignature)>>
  struct apply;
  
  template <typename Comp, typename DecoratedSignature, typename NakedT, typename... NakedArgs>
  struct apply<Comp, DecoratedSignature, Type<NakedT(NakedArgs...)>> {
    using RequiredSignature = Type<NakedT(NakedArgs...)>;
    using Op1 = RegisterFactory(Comp, DecoratedSignature, RequiredSignature);
    struct Op {
      using Result = Eval<GetResult(Op1)>;
      void operator()(ComponentStorage& storage) {
        auto provider = [](NakedArgs... args) {
          return NakedT(std::forward<NakedArgs>(args)...);
        };
        using RealOp = RegisterFactory(Comp, DecoratedSignature, Type<decltype(provider)>);
        FruitStaticAssert(IsSame(GetResult(Op1),
                                 GetResult(RealOp)));
        Eval<RealOp>()(storage);
      }
    };
    using type = PropagateError(Op1,
                 Op);
  };
};

struct InstallComponent {
  template <typename Comp, typename OtherComp>
  struct apply {
    using new_RsSuperset = SetUnion(typename OtherComp::RsSuperset,
                                    typename Comp::RsSuperset);
    using new_Ps = ConcatVectors(typename OtherComp::Ps,
                                 typename Comp::Ps);
    using new_Deps = ConcatVectors(typename OtherComp::Deps,
                                   typename Comp::Deps);
    // TODO: Use ConcatVectors here as well, once we've added a check+error for duplicates of these.
    using new_InterfaceBindings = SetUnion(typename OtherComp::InterfaceBindings,
                                           typename Comp::InterfaceBindings);
    
    FruitStaticAssert(IsSame(typename OtherComp::DeferredBindingFunctors, EmptyList));
    using new_DeferredBindingFunctors = typename Comp::DeferredBindingFunctors;
    
    using R = ConsComp(new_RsSuperset, new_Ps, new_Deps, new_InterfaceBindings, new_DeferredBindingFunctors);
    struct Op {
      using Result = Eval<R>;
      void operator()(ComponentStorage& storage, ComponentStorage&& other_storage) {
        storage.install(std::move(other_storage));
      }
    };
    using DuplicateTypes = SetIntersection(typename OtherComp::Ps,
                                           typename Comp::Ps);
    using type = If(Not(IsDisjoint(typename OtherComp::Ps, typename Comp::Ps)),
                    ConstructErrorWithArgVector(DuplicateTypesInComponentErrorTag, 
                                                SetToVector(DuplicateTypes)),
                 Op);
  };
};

struct InstallComponentHelper {
  template <typename Comp, typename... OtherCompParams>
  struct apply {
    using OtherComp = ConstructComponentImpl(OtherCompParams...);
    using type = PropagateError(OtherComp,
                 InstallComponent(Comp, OtherComp));
  };
};

struct ConvertComponent {
  template <typename SourceComp, typename DestComp>
  struct apply {
    // We need to register:
    // * All the types provided by the new component
    // * All the types required by the old component
    // except:
    // * The ones already provided by the old component.
    // * The ones required by the new one.
    using SourcePs = typename SourceComp::Ps;
    using DestPs   = typename   DestComp::Ps;
    using SourceRs = SetDifference(typename SourceComp::RsSuperset, typename SourceComp::Ps);
    using DestRs   = SetDifference(typename   DestComp::RsSuperset, typename   DestComp::Ps);
    using ToRegister = SetDifference(SetUnion(DestPs, SourceRs),
                                     SetUnion(DestRs, SourcePs));    
    using type = EnsureProvidedTypes(SourceComp, DestRs, SetToVector(ToRegister));
    
    // Not needed, just double-checking.
    // Uses FruitStaticAssert instead of FruitDelegateCheck so that it's checked only in debug mode.
#ifdef FRUIT_EXTRA_DEBUG
    FruitDelegateCheck(If(Or(IsError(type), IsError(DestComp)),
                         Type<int>, // No checks, we'll report a user error soon.
                         CheckComponentEntails(GetResult(type), DestComp)));
#endif // FRUIT_EXTRA_DEBUG
  };
};

struct ProcessDeferredBindings {
  template <typename Comp>
  struct apply;
  
  template <typename RsSupersetParam, typename PsParam, typename DepsParam, typename InterfaceBindingsParam, 
            typename DeferredBindingFunctors>
  struct apply<Comp<RsSupersetParam, PsParam, DepsParam, InterfaceBindingsParam, DeferredBindingFunctors>> {
    // Comp1 is the same as Comp, but without the DeferredBindingFunctors.
    using Comp1 = ConsComp(RsSupersetParam, PsParam, DepsParam, InterfaceBindingsParam, EmptyList);
    using type = Call(FoldList(DeferredBindingFunctors, Compose2ComponentFunctors, ComponentFunctorIdentity),
                      Comp1);
  };
};

struct AutoRegisterFactoryHelper {
  
  // General case, no way to bind it.
  template <typename Comp, typename TargetRequirements, typename InterfaceBinding, 
            typename has_inject_annotation, typename is_abstract, typename C,
            typename AnnotatedSignature, typename... Args>
  struct apply {
    using AnnotatedC        = SignatureType(AnnotatedSignature);
    using CFunctor          = ConsStdFunction(RemoveAnnotationsFromSignature(AnnotatedSignature));
    using AnnotatedCFunctor = CopyAnnotation(AnnotatedC, CFunctor);
    using type = ConstructError(NoBindingFoundErrorTag, AnnotatedCFunctor);
  };

  // No way to bind it (we need this specialization too to ensure that the specialization below
  // is not chosen for AnnotatedC=None.
  template <typename Comp, typename TargetRequirements, typename unused1, typename unused2,
            typename NakedI, typename AnnotatedSignature, typename... Argz>
  struct apply<Comp, TargetRequirements, None, unused1, unused2, Type<std::unique_ptr<NakedI>>,
               AnnotatedSignature, Argz...> {
    using AnnotatedC        = SignatureType(AnnotatedSignature);
    using CFunctor          = ConsStdFunction(RemoveAnnotationsFromSignature(AnnotatedSignature));
    using AnnotatedCFunctor = CopyAnnotation(AnnotatedC, CFunctor);
    using type = ConstructError(NoBindingFoundErrorTag, AnnotatedCFunctor);
  };
  
  // AnnotatedI has an interface binding, use it and look for a factory that returns the type that AnnotatedI is bound to.
  template <typename Comp, typename TargetRequirements, typename AnnotatedC, typename unused1, typename unused2,
            typename NakedI, typename AnnotatedSignature, typename... Argz>
  struct apply<Comp, TargetRequirements, AnnotatedC, unused1, unused2, Type<std::unique_ptr<NakedI>>,
               AnnotatedSignature, Argz...> {
      using I          = Type<NakedI>;
      using AnnotatedI = CopyAnnotation(SignatureType(AnnotatedSignature), I);
      using C          = RemoveAnnotations(AnnotatedC);
      using IFunctor = ConsStdFunction(ConsSignature(ConsUniquePtr(I), Argz...));
      using CFunctor = ConsStdFunction(ConsSignature(ConsUniquePtr(C), Argz...));
      using AnnotatedIFunctor = CopyAnnotation(AnnotatedI, IFunctor);
      using AnnotatedCFunctor = CopyAnnotation(AnnotatedC, CFunctor);
      
      using ProvidedSignature = ConsSignature(AnnotatedIFunctor, CopyAnnotation(AnnotatedC, ConsReference(CFunctor)));
      using LambdaSignature = ConsSignature(IFunctor, ConsReference(CFunctor));
      
      using F1 = ComponentFunctor(EnsureProvidedType, TargetRequirements, AnnotatedCFunctor);
      using F2 = ComponentFunctor(PreProcessRegisterProvider,  ProvidedSignature, LambdaSignature);
      using F3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, LambdaSignature);
      using R = Call(ComposeFunctors(F1, F2, F3), Comp);
      struct Op {
        using Result = Eval<GetResult(R)>;
        void operator()(ComponentStorage& storage) {
          using NakedC     = UnwrapType<Eval<C>>;
          auto provider = [](UnwrapType<Eval<CFunctor>>& fun) {
            return UnwrapType<Eval<IFunctor>>([=](UnwrapType<Argz>... args) {
              NakedC* c = fun(args...).release();
              NakedI* i = static_cast<NakedI*>(c);
              return std::unique_ptr<NakedI>(i);
            });
          };
          using RealF2 = ComponentFunctor(PreProcessRegisterProvider,  ProvidedSignature, Type<decltype(provider)>);
          using RealF3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
          using RealOp = Call(ComposeFunctors(F1, RealF2, RealF3), Comp);
          FruitStaticAssert(IsSame(GetResult(RealOp), GetResult(R)));
          Eval<RealOp>()(storage);
        }
      };
      using type = PropagateError(R,
                   Op);
  };

  // C doesn't have an interface binding as interface, nor an INJECT annotation, and is not an abstract class.
  // Bind std::function<unique_ptr<C>(Args...)> to std::function<C(Args...)> (possibly with annotations).
  template <typename Comp, typename TargetRequirements, typename NakedC, typename AnnotatedSignature,
            typename... Argz>
  struct apply<Comp, TargetRequirements, None, Bool<false>, Bool<false>,
               Type<std::unique_ptr<NakedC>>, AnnotatedSignature, Argz...> {
    using C = Type<NakedC>;
    using CFunctor          = ConsStdFunction(ConsSignature(C,                Argz...));
    using CUniquePtrFunctor = ConsStdFunction(ConsSignature(ConsUniquePtr(C), Argz...));
    using AnnotatedCUniquePtr        = SignatureType(AnnotatedSignature);
    using AnnotatedC                 = CopyAnnotation(AnnotatedCUniquePtr, C);
    using AnnotatedCFunctor          = CopyAnnotation(AnnotatedCUniquePtr, CFunctor);
    using AnnotatedCUniquePtrFunctor = CopyAnnotation(AnnotatedCUniquePtr, CUniquePtrFunctor);
    using AnnotatedCFunctorRef       = CopyAnnotation(AnnotatedCUniquePtr, ConsReference(CFunctor));
    
    using ProvidedSignature = ConsSignature(AnnotatedCUniquePtrFunctor, AnnotatedCFunctorRef);
    using LambdaSignature = ConsSignature(CUniquePtrFunctor, ConsReference(CFunctor));
    
    using F1 = ComponentFunctor(EnsureProvidedType, TargetRequirements, AnnotatedCFunctor);
    using F2 = ComponentFunctor(PreProcessRegisterProvider, ProvidedSignature, LambdaSignature);
    using F3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, LambdaSignature);
    using R = Call(ComposeFunctors(F1, F2, F3), Comp);
    struct Op {
      using Result = Eval<GetResult(R)>;
      void operator()(ComponentStorage& storage) {
        auto provider = [](UnwrapType<Eval<CFunctor>>& fun) {
          return UnwrapType<Eval<CUniquePtrFunctor>>([=](UnwrapType<Argz>... args) {
            NakedC* c = new NakedC(fun(args...));
            return std::unique_ptr<NakedC>(c);
          });
        };
        using RealF2 = ComponentFunctor(PreProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealF3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealOp = Call(ComposeFunctors(F1, RealF2, RealF3), Comp);
        FruitStaticAssert(IsSame(GetResult(RealOp), GetResult(R)));
        Eval<RealOp>()(storage);
      }
    };
    using type = If(IsError(R),
                    If(IsSame(R, Error<NoBindingFoundErrorTag, UnwrapType<Eval<AnnotatedCFunctor>>>),
                      // If we are about to report a NoBindingFound error for AnnotatedCFunctor,
                      // report one for std::function<std::unique_ptr<C>(Argz...)> instead,
                      // otherwise we'd report an error about a type that the user doesn't expect.
                      ConstructError(NoBindingFoundErrorTag, AnnotatedCUniquePtrFunctor),
                      R),
                    Op);
  };

  // This case never happens, has_inject_annotation is set to false below if the factory returns an unique_ptr.
  template <typename Comp, typename TargetRequirements, typename unused, typename NakedC, typename AnnotatedSignature, typename... Argz>
  struct apply<Comp, TargetRequirements, None, Bool<true>, unused, Type<std::unique_ptr<NakedC>>, AnnotatedSignature, Argz...> {
  };

  // C has an Inject typedef, use it. Value (not unique_ptr) case.
  // TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
  template <typename Comp, typename TargetRequirements, typename unused, typename NakedC, typename AnnotatedSignature, typename... Argz>
  struct apply<Comp, TargetRequirements, None, Bool<true>, unused, Type<NakedC>, AnnotatedSignature, Argz...> {
    using AnnotatedC = SignatureType(AnnotatedSignature);
    using DecoratedSignature = GetInjectAnnotation(AnnotatedC);
    using DecoratedSignatureArgs = SignatureArgs(DecoratedSignature);
    using ActualSignatureInInjectionTypedef = ConsSignatureWithVector(SignatureType(DecoratedSignature),
                                                                      RemoveNonAssisted(DecoratedSignatureArgs));
    using NonAssistedArgs = RemoveAssisted(DecoratedSignatureArgs);
    
    using F1 = ComponentFunctor(RegisterConstructorAsValueFactory, DecoratedSignature);
    using F2 = ComponentFunctor(EnsureProvidedTypes, TargetRequirements, ExpandProvidersInParams(NonAssistedArgs));
    
    using type = PropagateError(DecoratedSignature,
                 If(Not(IsSame(AnnotatedSignature, ActualSignatureInInjectionTypedef)),
                    ConstructError(FunctorSignatureDoesNotMatchErrorTag, AnnotatedSignature, ActualSignatureInInjectionTypedef),
                 Call(ComposeFunctors(F1, F2), Comp)));
  };
};

struct AutoRegisterHelper {

  template <typename Comp, typename TargetRequirements, typename has_inject_annotation, typename AnnotatedC>
  struct apply;

  // C has an Inject typedef, use it.
  template <typename Comp, typename TargetRequirements, typename AnnotatedC>
  struct apply<Comp, TargetRequirements, Bool<true>, AnnotatedC> {
    using Inject = GetInjectAnnotation(AnnotatedC);
    using CRequirements = ExpandProvidersInParams(SignatureArgs(Inject));
    using F = ComposeFunctors(
                  ComponentFunctor(PreProcessRegisterConstructor, Inject),
                  ComponentFunctor(PostProcessRegisterConstructor, Inject),
                  ComponentFunctor(EnsureProvidedTypes, TargetRequirements, CRequirements));
    using type = PropagateError(Inject,
                 Call(F, Comp));
  };
  

  template <typename Comp, typename TargetRequirements, typename AnnotatedC>
  struct apply<Comp, TargetRequirements, Bool<false>, AnnotatedC> {
    using type = ConstructError(NoBindingFoundErrorTag, AnnotatedC);
  };
};

struct AutoRegister {
  // The types in TargetRequirements will not be auto-registered.
  template <typename Comp, typename TargetRequirements, typename AnnotatedC>
  struct apply;

  // Tries to register C by looking for a typedef called Inject inside C.
  template <typename Comp, typename TargetRequirements, typename AnnotatedC>
  struct apply {
    using type = AutoRegisterHelper(Comp,
                                    TargetRequirements,
                                    HasInjectAnnotation(RemoveAnnotations(AnnotatedC)),
                                    AnnotatedC);
  };

  template <typename Comp, typename TargetRequirements, typename NakedC, typename... NakedArgs>
  struct apply<Comp, TargetRequirements, Type<std::function<NakedC(NakedArgs...)>>> {
    using type = AutoRegisterFactoryHelper(Comp,
                                           TargetRequirements,
                                           FindInMap(typename Comp::InterfaceBindings, Type<NakedC>),
                                           HasInjectAnnotation(Type<NakedC>),
                                           IsAbstract(Type<NakedC>),
                                           Type<NakedC>,
                                           Type<NakedC(NakedArgs...)>,
                                           Id<RemoveAnnotations(Type<NakedArgs>)>...);
  };    

  template <typename Comp, typename TargetRequirements, typename NakedC, typename... NakedArgs>
  struct apply<Comp, TargetRequirements, Type<std::function<std::unique_ptr<NakedC>(NakedArgs...)>>> {
    using type = AutoRegisterFactoryHelper(Comp,
                                           TargetRequirements,
                                           FindInMap(typename Comp::InterfaceBindings, Type<NakedC>),
                                           Bool<false>,
                                           IsAbstract(Type<NakedC>),
                                           Type<std::unique_ptr<NakedC>>,
                                           Type<std::unique_ptr<NakedC>(NakedArgs...)>,
                                           Id<RemoveAnnotations(Type<NakedArgs>)>...);
  };

  template <typename Comp, typename TargetRequirements, typename Annotation, typename NakedC, typename... NakedArgs>
  struct apply<Comp, TargetRequirements, 
               Type<fruit::Annotated<Annotation, std::function<NakedC(NakedArgs...)>>>> {
    using type = AutoRegisterFactoryHelper(Comp,
                                           TargetRequirements,
                                           FindInMap(typename Comp::InterfaceBindings,
                                                     Type<fruit::Annotated<Annotation, NakedC>>),
                                           HasInjectAnnotation(Type<NakedC>),
                                           IsAbstract(Type<NakedC>),
                                           Type<NakedC>,
                                           Type<fruit::Annotated<Annotation, NakedC>(NakedArgs...)>,
                                           Id<RemoveAnnotations(Type<NakedArgs>)>...);
  };    

  template <typename Comp, typename TargetRequirements, typename Annotation, typename NakedC, typename... NakedArgs>
  struct apply<Comp, TargetRequirements, 
               Type<fruit::Annotated<Annotation, std::function<std::unique_ptr<NakedC>(NakedArgs...)>>>> {
    using type = AutoRegisterFactoryHelper(Comp,
                                           TargetRequirements,
                                           FindInMap(typename Comp::InterfaceBindings,
                                                     Type<fruit::Annotated<Annotation, NakedC>>),
                                           Bool<false>,
                                           IsAbstract(Type<NakedC>),
                                           Type<std::unique_ptr<NakedC>>,
                                           Type<fruit::Annotated<Annotation, std::unique_ptr<NakedC>>(NakedArgs...)>,
                                           Id<RemoveAnnotations(Type<NakedArgs>)>...);
  };
};

struct EnsureProvidedTypeHelper {
  template <typename Comp, typename TargetRequirements, 
            typename is_already_provided_or_in_target_requirements, typename InterfaceBinding, 
            typename AnnotatedC>
  struct apply;

  // Already provided or in target requirements, ok.
  template <typename Comp, typename TargetRequirements, typename unused, typename AnnotatedC>
  struct apply<Comp, TargetRequirements, Bool<true>, unused, AnnotatedC> {
    using type = ComponentFunctorIdentity(Comp);
  };

  // Has an interface binding.
  template <typename Comp, typename TargetRequirements, typename AnnotatedC,
            typename AnnotatedI>
  struct apply<Comp, TargetRequirements, Bool<false>, AnnotatedC, AnnotatedI> {
    using F1 = ComponentFunctor(ProcessInterfaceBinding, AnnotatedI, AnnotatedC);
    using F2 = ComponentFunctor(EnsureProvidedType, TargetRequirements, AnnotatedC);
    using type = Call(ComposeFunctors(F1, F2), Comp);
  };

  // Not yet provided, nor in target requirements, nor in InterfaceBindings. Try auto-registering.
  template <typename Comp, typename TargetRequirements, typename AnnotatedC>
  struct apply<Comp, TargetRequirements, Bool<false>, None, AnnotatedC> {
    using type = AutoRegister(Comp, TargetRequirements, AnnotatedC);
  };
};

struct EnsureProvidedType {
  template <typename Comp, typename TargetRequirements, typename AnnotatedT>
  struct apply {
    using AnnotatedC = NormalizeType(AnnotatedT);
    using type = EnsureProvidedTypeHelper(Comp,
                                          TargetRequirements,
                                          Or(MapContainsKey(typename Comp::Deps, AnnotatedC),
                                             IsInSet(AnnotatedC, TargetRequirements)),
                                          FindInMap(typename Comp::InterfaceBindings, AnnotatedC),
                                          AnnotatedC);
  };
};

struct EnsureProvidedTypes {
  template <typename Comp, typename TargetRequirements, typename TypeVector>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename T>
      struct apply {
        using type = Compose2ComponentFunctors(ComponentFunctor(EnsureProvidedType, TargetRequirements, T),
                                               CurrentResult);
      };      
    };
    
    using type = Call(FoldVector(TypeVector, Helper, ComponentFunctorIdentity),
                      Comp);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_COMPONENT_FUNCTORS_DEFN_H
