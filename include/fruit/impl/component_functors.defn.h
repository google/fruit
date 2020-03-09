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

#include <fruit/component.h>

#include <fruit/impl/injection_debug_errors.h>
#include <fruit/impl/injection_errors.h>
#include <fruit/impl/injector/injector_storage.h>

#include <memory>

/*********************************************************************************************************************************
  This file contains functors that take a Comp and return a struct Op with the form:
  struct {
    using Result = Comp1;
    void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {...}
    std::size_t numEntries() {...}
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

struct ComponentFunctorIdentity {
  template <typename Comp>
  struct apply {
    struct type {
      using Result = Comp;
      void operator()(FixedSizeVector<ComponentStorageEntry>&) {}
      std::size_t numEntries() {
        return 0;
      }
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
          void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
            Eval<Op2>()(entries);
            Eval<Op1>()(entries);
          }
          std::size_t numEntries() {
            return Eval<Op1>().numEntries() + Eval<Op2>().numEntries();
          }
        };
        using type = PropagateError(Op1, PropagateError(Op2, Op));
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

// ReverseComposeFunctors(T1, ..., Tn) is equivalent to ComposeFunctors(Tn, ..., T1), but it's more
// efficient when all of the following must be evaluated:
// ReverseComposeFunctors<T1>
// ReverseComposeFunctors<T2, T1>
// ReverseComposeFunctors<T3, T2, T1>
// In that case, this implementation shares many more instantiations with previous invocations
struct ReverseComposeFunctors {
  template <typename... Functors>
  struct apply {
    using type = ComponentFunctorIdentity;
  };

  template <typename Functor>
  struct apply<Functor> {
    using type = Functor;
  };

  template <typename Functor, typename... Functors>
  struct apply<Functor, Functors...> {
    using type = Compose2ComponentFunctors(ReverseComposeFunctors(Functors...), Functor);
  };
};

struct EnsureProvidedType;

struct EnsureProvidedTypes;

// Doesn't actually bind in ComponentStorage. The binding is added later (if needed) using ProcessInterfaceBinding.
struct AddDeferredInterfaceBinding {
  template <typename Comp, typename AnnotatedI, typename AnnotatedC>
  struct apply {
    using Comp1 = ConsComp(typename Comp::RsSuperset, typename Comp::Ps, typename Comp::NonConstRsPs,
#if !FRUIT_NO_LOOP_CHECK
                           typename Comp::Deps,
#endif
                           PushFront(typename Comp::InterfaceBindings, Pair<AnnotatedI, AnnotatedC>),
                           typename Comp::DeferredBindingFunctors);
    struct Op {
      // Note that we do NOT call AddProvidedType here. We'll only know the right required type
      // when the binding will be used.
      using Result = Eval<Comp1>;
      void operator()(FixedSizeVector<ComponentStorageEntry>&) {}
      std::size_t numEntries() {
        return 0;
      }
    };
    using I = RemoveAnnotations(AnnotatedI);
    using C = RemoveAnnotations(AnnotatedC);
    using type =
        If(IsSame(I, C), ConstructError(InterfaceBindingToSelfErrorTag, C),
           If(Not(IsBaseOf(I, C)), ConstructError(NotABaseClassOfErrorTag, I, C),
              If(Not(IsSame(I, NormalizeType(I))), ConstructError(NonClassTypeErrorTag, I, NormalizeUntilStable(I)),
                 If(Not(IsSame(C, NormalizeType(C))),
                    // We handle this case too, just to be on the safe side, but this should never happen.
                    ConstructError(NonClassTypeErrorTag, C, NormalizeUntilStable(C)),
                    If(IsInSet(AnnotatedI, typename Comp::Ps), ConstructError(TypeAlreadyBoundErrorTag, AnnotatedI),
                       If(MapContainsKey(typename Comp::InterfaceBindings, AnnotatedI),
                          ConstructError(TypeAlreadyBoundErrorTag, AnnotatedI), Op))))));
  };
};

struct ProcessInterfaceBinding {
  template <typename Comp, typename AnnotatedI, typename AnnotatedC, typename NonConstBindingRequired>
  struct apply {
    using R = If(NonConstBindingRequired,
                 AddProvidedTypeIgnoringInterfaceBindings(Comp, AnnotatedI, NonConstBindingRequired, Vector<AnnotatedC>,
                                                          Vector<AnnotatedC>),
                 AddProvidedTypeIgnoringInterfaceBindings(Comp, AnnotatedI, NonConstBindingRequired, Vector<AnnotatedC>,
                                                          Vector<>));
    struct ConstOp {
      // This must be here (and not in AddDeferredInterfaceBinding) because the binding might be
      // used to bind functors instead, so we might never need to add C to the requirements.
      using Result = Eval<R>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        entries.push_back(
            InjectorStorage::createComponentStorageEntryForConstBind<UnwrapType<AnnotatedI>, UnwrapType<AnnotatedC>>());
      };

      std::size_t numEntries() {
        return 1;
      }
    };
    struct NonConstOp {
      // This must be here (and not in AddDeferredInterfaceBinding) because the binding might be
      // used to bind functors instead, so we might never need to add C to the requirements.
      using Result = Eval<R>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        entries.push_back(
            InjectorStorage::createComponentStorageEntryForBind<UnwrapType<AnnotatedI>, UnwrapType<AnnotatedC>>());
      };

      std::size_t numEntries() {
        return 1;
      }
    };
    using type = PropagateError(R, If(NonConstBindingRequired, NonConstOp, ConstOp));
  };
};

struct AddInterfaceMultibinding {
  template <typename Comp, typename AnnotatedI, typename AnnotatedC>
  struct apply {
    using I = RemoveAnnotations(AnnotatedI);
    using C = RemoveAnnotations(AnnotatedC);
    using R = AddRequirements(Comp, Vector<AnnotatedC>, Vector<AnnotatedC>);
    struct Op {
      using Result = Eval<R>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        entries.push_back(InjectorStorage::createComponentStorageEntryForMultibinding<UnwrapType<AnnotatedI>,
                                                                                      UnwrapType<AnnotatedC>>());
        entries.push_back(
            InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<UnwrapType<AnnotatedI>>());
      };

      std::size_t numEntries() {
        return 2;
      }
    };
    using type = If(Not(IsBaseOf(I, C)), ConstructError(NotABaseClassOfErrorTag, I, C), Op);
  };
};

template <typename AnnotatedSignature, typename Lambda, typename OptionalAnnotatedI>
struct PostProcessRegisterProviderHelper;

template <typename AnnotatedSignature, typename Lambda, typename AnnotatedI>
struct PostProcessRegisterProviderHelper;

template <typename AnnotatedSignature, typename Lambda, typename AnnotatedI>
struct PostProcessRegisterProviderHelper<AnnotatedSignature, Lambda, Type<AnnotatedI>> {
  inline void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
    entries.push_back(
        InjectorStorage::createComponentStorageEntryForCompressedProvider<AnnotatedSignature, Lambda, AnnotatedI>());
    entries.push_back(InjectorStorage::createComponentStorageEntryForProvider<AnnotatedSignature, Lambda>());
  }

  std::size_t numEntries() {
    return 2;
  }
};

template <typename AnnotatedSignature, typename Lambda>
struct PostProcessRegisterProviderHelper<AnnotatedSignature, Lambda, None> {
  inline void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
    entries.push_back(InjectorStorage::createComponentStorageEntryForProvider<AnnotatedSignature, Lambda>());
  }

  std::size_t numEntries() {
    return 1;
  }
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerProvider() overloads in ComponentStorage.
struct PostProcessRegisterProvider {
  template <typename Comp, typename AnnotatedSignature, typename Lambda>
  struct apply {
    using AnnotatedC = NormalizeType(SignatureType(AnnotatedSignature));
    using OptionalAnnotatedI = FindValueInMap(typename Comp::InterfaceBindings, AnnotatedC);
    struct Op {
      using Result = Comp;

      using Helper = PostProcessRegisterProviderHelper<UnwrapType<AnnotatedSignature>, UnwrapType<Lambda>,
                                                       Eval<OptionalAnnotatedI>>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        Helper()(entries);
      }
      std::size_t numEntries() {
        return Helper().numEntries();
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
    using AnnotatedCDeps = NormalizeTypeVector(SignatureArgs(AnnotatedSignature));
    using R = AddProvidedType(Comp, AnnotatedC, Bool<true>, AnnotatedCDeps,
                              Id<NormalizedNonConstTypesIn(SignatureArgs(AnnotatedSignature))>);
    using type =
        If(Not(IsSame(Signature, SignatureFromLambda)),
           ConstructError(AnnotatedSignatureDifferentFromLambdaSignatureErrorTag, Signature, SignatureFromLambda),
           PropagateError(
               CheckInjectableType(RemoveAnnotations(SignatureType(AnnotatedSignature))),
               PropagateError(
                   CheckInjectableTypeVector(RemoveAnnotationsFromVector(AnnotatedCDeps)),
                   PropagateError(
                       CheckInjectableType(SignatureType(SignatureFromLambda)),
                       PropagateError(
                           CheckInjectableTypeVector(SignatureArgs(SignatureFromLambda)),
                           If(And(IsPointer(SignatureType(SignatureFromLambda)),
                                  And(IsAbstract(RemovePointer(SignatureType(SignatureFromLambda))),
                                      Not(HasVirtualDestructor(RemovePointer(SignatureType(SignatureFromLambda)))))),
                              ConstructError(ProviderReturningPointerToAbstractClassWithNoVirtualDestructorErrorTag,
                                             RemovePointer(SignatureType(SignatureFromLambda))),
                              ComponentFunctorIdentity(R)))))));
  };
};

// The registration is actually deferred until the PartialComponent is converted to a component.
struct DeferredRegisterProviderWithAnnotations {
  template <typename Comp, typename AnnotatedSignature, typename Lambda>
  struct apply {
    using Comp1 = AddDeferredBinding(Comp, ComponentFunctor(PostProcessRegisterProvider, AnnotatedSignature, Lambda));
    using type = PreProcessRegisterProvider(Comp1, AnnotatedSignature, Lambda);
  };
};

// The registration is actually deferred until the PartialComponent is converted to a component.
struct DeferredRegisterProvider {
  template <typename Comp, typename Lambda>
  struct apply {
    using type = DeferredRegisterProviderWithAnnotations(Comp, FunctionSignature(Lambda), Lambda);
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
    using AnnotatedArgVector = NormalizeTypeVector(AnnotatedArgs);
    using NonConstRequirements = NormalizedNonConstTypesIn(AnnotatedArgs);
    using R = AddRequirements(Comp, AnnotatedArgVector, NonConstRequirements);
    struct Op {
      using Result = Eval<R>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        entries.push_back(
            InjectorStorage::createComponentStorageEntryForMultibindingProvider<UnwrapType<AnnotatedSignature>,
                                                                                UnwrapType<Lambda>>());
        entries.push_back(InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<
                          UnwrapType<Eval<NormalizeType(SignatureType(AnnotatedSignature))>>>());
      }
      std::size_t numEntries() {
        return 2;
      }
    };
    using type = If(
        Not(IsValidSignature(AnnotatedSignature)), ConstructError(NotASignatureErrorTag, AnnotatedSignature),
        PropagateError(
            CheckInjectableType(RemoveAnnotations(SignatureType(AnnotatedSignature))),
            PropagateError(
                CheckInjectableTypeVector(RemoveAnnotationsFromVector(SignatureArgs(AnnotatedSignature))),
                PropagateError(
                    CheckInjectableType(SignatureType(SignatureFromLambda)),
                    PropagateError(
                        CheckInjectableTypeVector(SignatureArgs(SignatureFromLambda)),
                        If(IsAbstract(RemoveAnnotations(SignatureType(AnnotatedSignature))),
                           ConstructError(CannotConstructAbstractClassErrorTag,
                                          RemoveAnnotations(SignatureType(AnnotatedSignature))),
                           If(Not(IsSame(Signature, SignatureFromLambda)),
                              ConstructError(AnnotatedSignatureDifferentFromLambdaSignatureErrorTag, Signature,
                                             SignatureFromLambda),
                              If(And(IsPointer(SignatureType(SignatureFromLambda)),
                                     And(IsAbstract(RemovePointer(SignatureType(SignatureFromLambda))),
                                         Not(HasVirtualDestructor(RemovePointer(SignatureType(SignatureFromLambda)))))),
                                 ConstructError(
                                     MultibindingProviderReturningPointerToAbstractClassWithNoVirtualDestructorErrorTag,
                                     RemovePointer(SignatureType(SignatureFromLambda))),
                                 PropagateError(R, Op)))))))));
  };
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerMultibindingProvider() overloads in ComponentStorage.
struct RegisterMultibindingProvider {
  template <typename Comp, typename Lambda>
  struct apply {
    using type = RegisterMultibindingProviderWithAnnotations(Comp, FunctionSignature(Lambda), Lambda);
  };
};

// Non-assisted case.
template <int numAssistedBefore, int numNonAssistedBefore, typename Arg>
struct GetAssistedArg {
  template <typename InjectedArgsTuple, typename UserProvidedArgsTuple>
  inline Arg operator()(InjectedArgsTuple& injected_args, UserProvidedArgsTuple&) {
    return std::get<numNonAssistedBefore>(injected_args);
  }
};

// Assisted case.
template <int numAssistedBefore, int numNonAssistedBefore, typename Arg>
struct GetAssistedArg<numAssistedBefore, numNonAssistedBefore, Assisted<Arg>> {
  template <typename InjectedArgsTuple, typename UserProvidedArgsTuple>
  inline Arg operator()(InjectedArgsTuple&, UserProvidedArgsTuple& user_provided_args) {
    return std::get<numAssistedBefore>(user_provided_args);
  }
};

struct RegisterFactoryHelper {

  template <typename Comp, typename DecoratedSignature, typename Lambda,
            // std::function<InjectedSignature> is the injected type (possibly with an Annotation<> wrapping it)
            typename InjectedSignature, typename RequiredLambdaSignature, typename InjectedAnnotatedArgs,
            // The types that are injected, unwrapped from any Annotation<>.
            typename InjectedArgs, typename IndexSequence>
  struct apply;

  template <typename Comp, typename DecoratedSignature, typename Lambda, typename NakedC,
            typename... NakedUserProvidedArgs, typename... NakedAllArgs, typename... InjectedAnnotatedArgs,
            typename... NakedInjectedArgs, typename... Indexes>
  struct apply<Comp, DecoratedSignature, Lambda, Type<NakedC(NakedUserProvidedArgs...)>, Type<NakedC(NakedAllArgs...)>,
               Vector<InjectedAnnotatedArgs...>, Vector<Type<NakedInjectedArgs>...>, Vector<Indexes...>> {
    // Here we call "decorated" the types that might be wrapped in Annotated<> or Assisted<>,
    // while we call "annotated" the ones that might only be wrapped in Annotated<> (but not Assisted<>).
    using AnnotatedT = SignatureType(DecoratedSignature);
    using T = RemoveAnnotations(AnnotatedT);
    using DecoratedArgs = SignatureArgs(DecoratedSignature);
    using NakedInjectedSignature = NakedC(NakedUserProvidedArgs...);
    using NakedRequiredSignature = NakedC(NakedAllArgs...);
    using NakedFunctor = std::function<NakedInjectedSignature>;
    // This is usually the same as Functor, but this might be annotated.
    using AnnotatedFunctor = CopyAnnotation(AnnotatedT, Type<NakedFunctor>);
    using FunctorDeps = NormalizeTypeVector(Vector<InjectedAnnotatedArgs...>);
    using FunctorNonConstDeps = NormalizedNonConstTypesIn(Vector<InjectedAnnotatedArgs...>);
    using R = AddProvidedType(Comp, AnnotatedFunctor, Bool<true>, FunctorDeps, FunctorNonConstDeps);
    struct Op {
      using Result = Eval<R>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        auto function_provider = [](NakedInjectedArgs... args) {
          auto injected_args = std::make_tuple(args...);
          auto object_provider = [injected_args](NakedUserProvidedArgs... params) mutable {
            auto user_provided_args = std::tie(params...);
            // These are unused if they are 0-arg tuples. Silence the unused-variable warnings anyway.
            (void)injected_args;
            (void)user_provided_args;

            return LambdaInvoker::invoke<UnwrapType<Lambda>, NakedAllArgs...>(
                GetAssistedArg<
                    Eval<NumAssistedBefore(Indexes, DecoratedArgs)>::value,
                    getIntValue<Indexes>() - Eval<NumAssistedBefore(Indexes, DecoratedArgs)>::value,
                    // Note that the Assisted<> wrapper (if any) remains, we just remove any wrapping Annotated<>.
                    UnwrapType<Eval<RemoveAnnotations(GetNthType(Indexes, DecoratedArgs))>>>()(injected_args,
                                                                                               user_provided_args)...);
          };
          return NakedFunctor(object_provider);
        };
        entries.push_back(InjectorStorage::createComponentStorageEntryForProvider<
                          UnwrapType<Eval<ConsSignatureWithVector(AnnotatedFunctor, Vector<InjectedAnnotatedArgs...>)>>,
                          decltype(function_provider)>());
      }
      std::size_t numEntries() {
        return 1;
      }
    };
    // The first two IsValidSignature checks are a bit of a hack, they are needed to make the F2/RealF2 split
    // work in the caller (we need to allow Lambda to be a function type).
    using type = If(Not(IsSame(Type<NakedRequiredSignature>, FunctionSignature(Lambda))),
                    ConstructError(FunctorSignatureDoesNotMatchErrorTag, Type<NakedRequiredSignature>,
                                   FunctionSignature(Lambda)),
                    If(IsPointer(T), ConstructError(FactoryReturningPointerErrorTag, DecoratedSignature),
                       PropagateError(R, Op)));
  };
};

struct RegisterFactory {
  template <typename Comp, typename DecoratedSignature, typename Lambda>
  struct apply {
    using LambdaReturnType = SignatureType(FunctionSignature(Lambda));
    using type =
        If(Not(IsValidSignature(DecoratedSignature)), ConstructError(NotASignatureErrorTag, DecoratedSignature),
           PropagateError(
               CheckInjectableType(RemoveAnnotations(SignatureType(DecoratedSignature))),
               PropagateError(
                   CheckInjectableTypeVector(
                       RemoveAnnotationsFromVector(RemoveAssisted(SignatureArgs(DecoratedSignature)))),
                   If(IsAbstract(RemoveAnnotations(SignatureType(DecoratedSignature))),
                      // We error out early in this case. Calling RegisterFactoryHelper would also produce an error, but
                      // it'd be
                      // much less user-friendly.
                      ConstructError(CannotConstructAbstractClassErrorTag,
                                     RemoveAnnotations(SignatureType(DecoratedSignature))),
                      If(Not(Or(IsEmpty(Lambda), IsValidSignature(Lambda))),
                         ConstructError(LambdaWithCapturesErrorTag, Lambda),
                         If(Not(Or(IsTriviallyCopyable(Lambda), IsValidSignature(Lambda))),
                            ConstructError(NonTriviallyCopyableLambdaErrorTag, Lambda),
                            If(And(IsUniquePtr(LambdaReturnType),
                                   And(IsAbstract(RemoveUniquePtr(LambdaReturnType)),
                                       Not(HasVirtualDestructor(RemoveUniquePtr(LambdaReturnType))))),
                               ConstructError(RegisterFactoryForUniquePtrOfAbstractClassWithNoVirtualDestructorErrorTag,
                                              RemoveUniquePtr(LambdaReturnType)),
                               RegisterFactoryHelper(
                                   Comp, DecoratedSignature, Lambda,
                                   InjectedSignatureForAssistedFactory(DecoratedSignature),
                                   RequiredLambdaSignatureForAssistedFactory(DecoratedSignature),
                                   RemoveAssisted(SignatureArgs(DecoratedSignature)),
                                   RemoveAnnotationsFromVector(RemoveAssisted(SignatureArgs(DecoratedSignature))),
                                   GenerateIntSequence(
                                       VectorSize(RequiredLambdaArgsForAssistedFactory(DecoratedSignature)))))))))));
  };
};

struct PostProcessRegisterConstructor;

template <typename AnnotatedSignature, typename OptionalAnnotatedI>
struct PostProcessRegisterConstructorHelper;

template <typename AnnotatedSignature, typename AnnotatedI>
struct PostProcessRegisterConstructorHelper;

template <typename AnnotatedSignature, typename AnnotatedI>
struct PostProcessRegisterConstructorHelper<AnnotatedSignature, Type<AnnotatedI>> {
  inline void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
    entries.push_back(
        InjectorStorage::createComponentStorageEntryForCompressedConstructor<AnnotatedSignature, AnnotatedI>());
    entries.push_back(InjectorStorage::createComponentStorageEntryForConstructor<AnnotatedSignature>());
  }
  std::size_t numEntries() {
    return 2;
  }
};

template <typename AnnotatedSignature>
struct PostProcessRegisterConstructorHelper<AnnotatedSignature, None> {
  inline void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
    entries.push_back(InjectorStorage::createComponentStorageEntryForConstructor<AnnotatedSignature>());
  }
  std::size_t numEntries() {
    return 1;
  }
};

struct PostProcessRegisterConstructor {
  template <typename Comp, typename AnnotatedSignature>
  struct apply {
    struct type {
      using AnnotatedC = NormalizeType(SignatureType(AnnotatedSignature));
      using Result = Comp;
      using Helper =
          PostProcessRegisterConstructorHelper<UnwrapType<AnnotatedSignature>,
                                               Eval<FindValueInMap(typename Comp::InterfaceBindings, AnnotatedC)>>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        Helper()(entries);
      }
      std::size_t numEntries() {
        return Helper().numEntries();
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
    using AnnotatedT = SignatureType(AnnotatedSignature);
    using AnnotatedArgs = SignatureArgs(AnnotatedSignature);
    using AnnotatedC = NormalizeType(AnnotatedT);
    using CDeps = NormalizeTypeVector(AnnotatedArgs);
    using CNonConstDeps = NormalizedNonConstTypesIn(AnnotatedArgs);
    using R = AddProvidedType(Comp, AnnotatedC, Bool<true>, CDeps, CNonConstDeps);
    using type = If(
        Not(IsValidSignature(AnnotatedSignature)), ConstructError(NotASignatureErrorTag, AnnotatedSignature),
        PropagateError(CheckInjectableType(RemoveAnnotations(C)),
                       PropagateError(CheckInjectableTypeVector(RemoveAnnotationsFromVector(Args)),
                                      If(IsAbstract(RemoveAnnotations(SignatureType(AnnotatedSignature))),
                                         ConstructError(CannotConstructAbstractClassErrorTag,
                                                        RemoveAnnotations(SignatureType(AnnotatedSignature))),
                                         If(Not(IsConstructibleWithVector(C, Args)),
                                            ConstructError(NoConstructorMatchingInjectSignatureErrorTag, C, Signature),
                                            PropagateError(R, ComponentFunctorIdentity(R)))))));
  };
};

struct DeferredRegisterConstructor {
  template <typename Comp, typename AnnotatedSignature>
  struct apply {
    using Comp1 = AddDeferredBinding(Comp, ComponentFunctor(PostProcessRegisterConstructor, AnnotatedSignature));
    using type = PreProcessRegisterConstructor(Comp1, AnnotatedSignature);
  };
};

struct RegisterInstance {
  template <typename Comp, typename AnnotatedC, typename C, typename IsNonConst>
  struct apply {
    using R = AddProvidedType(Comp, AnnotatedC, IsNonConst, Vector<>, Vector<>);
    struct Op {
      using Result = Eval<R>;
      void operator()(FixedSizeVector<ComponentStorageEntry>&) {}
      std::size_t numEntries() {
        return 0;
      }
    };
    using type = PropagateError(
        CheckNormalizedTypes(ConsVector(RemoveAnnotations(AnnotatedC))),
        PropagateError(
            CheckNormalizedTypes(ConsVector(C)),
            If(Not(IsSame(C, NormalizeType(C))), ConstructError(NonClassTypeErrorTag, C, NormalizeUntilStable(C)),
               If(Not(IsSame(RemoveAnnotations(AnnotatedC), NormalizeType(RemoveAnnotations(AnnotatedC)))),
                  ConstructError(NonClassTypeErrorTag, RemoveAnnotations(AnnotatedC),
                                 NormalizeUntilStable(RemoveAnnotations(C))),
                  // The IsSame check is not redundant because IsBaseOf returns false for non-class types (e.g. int).
                  If(Not(Or(IsSame(RemoveAnnotations(AnnotatedC), C), IsBaseOf(RemoveAnnotations(AnnotatedC), C))),
                     ConstructError(TypeMismatchInBindInstanceErrorTag, RemoveAnnotations(AnnotatedC), C),
                     PropagateError(R, Op))))));
  };
};

struct RegisterConstructorAsValueFactory {
  template <typename Comp, typename DecoratedSignature,
            typename RequiredSignature = Eval<RequiredLambdaSignatureForAssistedFactory(DecoratedSignature)>>
  struct apply;

  template <typename Comp, typename DecoratedSignature, typename NakedT, typename... NakedArgs>
  struct apply<Comp, DecoratedSignature, Type<NakedT(NakedArgs...)>> {
    using RequiredSignature = Type<NakedT(NakedArgs...)>;
    using Op1 = RegisterFactory(Comp, DecoratedSignature, RequiredSignature);
    struct Op {
      using Result = Eval<GetResult(Op1)>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        auto provider = [](NakedArgs... args) { return NakedT(std::forward<NakedArgs>(args)...); };
        using RealOp = RegisterFactory(Comp, DecoratedSignature, Type<decltype(provider)>);
        FruitStaticAssert(IsSame(GetResult(Op1), GetResult(RealOp)));
        Eval<RealOp>()(entries);
      }
      std::size_t numEntries() {
#if FRUIT_EXTRA_DEBUG
        auto provider = [](NakedArgs... args) { return NakedT(std::forward<NakedArgs>(args)...); };
        using RealOp = RegisterFactory(Comp, DecoratedSignature, Type<decltype(provider)>);
        FruitAssert(Eval<Op1>().numEntries() == Eval<RealOp>().numEntries());
#endif
        return Eval<Op1>().numEntries();
      }
    };
    using type = PropagateError(Op1, Op);
  };
};

struct RegisterConstructorAsUniquePtrFactory {
  template <typename Comp, typename DecoratedSignature,
            typename RequiredSignature = Eval<RequiredLambdaSignatureForAssistedFactory(DecoratedSignature)>>
  struct apply;

  template <typename Comp, typename DecoratedSignature, typename NakedT, typename... NakedArgs>
  struct apply<Comp, DecoratedSignature, Type<std::unique_ptr<NakedT>(NakedArgs...)>> {
    using RequiredSignature = Type<std::unique_ptr<NakedT>(NakedArgs...)>;
    using Op1 = RegisterFactory(Comp, DecoratedSignature, RequiredSignature);
    struct Op {
      using Result = Eval<GetResult(Op1)>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        auto provider = [](NakedArgs... args) {
          return std::unique_ptr<NakedT>(new NakedT(std::forward<NakedArgs>(args)...));
        };
        using RealOp = RegisterFactory(Comp, DecoratedSignature, Type<decltype(provider)>);
        FruitStaticAssert(IsSame(GetResult(Op1), GetResult(RealOp)));
        Eval<RealOp>()(entries);
      };
      std::size_t numEntries() {
#if FRUIT_EXTRA_DEBUG
        auto provider = [](NakedArgs... args) {
          return std::unique_ptr<NakedT>(new NakedT(std::forward<NakedArgs>(args)...));
        };
        using RealOp = RegisterFactory(Comp, DecoratedSignature, Type<decltype(provider)>);
        FruitAssert(Eval<Op1>().numEntries() == Eval<RealOp>().numEntries());
#endif
        return Eval<Op1>().numEntries();
      }
    };

    using type = PropagateError(Op1, Op);
  };
};

struct InstallComponent {
  template <typename Comp, typename OtherComp>
  struct apply {
    using new_RsSuperset = SetUnion(typename OtherComp::RsSuperset, typename Comp::RsSuperset);
    using new_Ps = SetUncheckedUnion(typename OtherComp::Ps, typename Comp::Ps);
    using new_NonConstRsPs = SetUnion(typename OtherComp::NonConstRsPs, typename Comp::NonConstRsPs);
#if !FRUIT_NO_LOOP_CHECK
    using new_Deps = ConcatVectors(typename OtherComp::Deps, typename Comp::Deps);
#endif
    FruitStaticAssert(IsSame(typename OtherComp::InterfaceBindings, Vector<>));
    using new_InterfaceBindings = typename Comp::InterfaceBindings;

    FruitStaticAssert(IsSame(typename OtherComp::DeferredBindingFunctors, EmptyList));
    using new_DeferredBindingFunctors = typename Comp::DeferredBindingFunctors;

    using R = ConsComp(new_RsSuperset, new_Ps, new_NonConstRsPs,
#if !FRUIT_NO_LOOP_CHECK
                       new_Deps,
#endif
                       new_InterfaceBindings, new_DeferredBindingFunctors);
    struct Op {
      using Result = Eval<R>;
      void operator()(FixedSizeVector<ComponentStorageEntry>&) {}
      std::size_t numEntries() {
        return 0;
      }
    };
    using InterfacePs = VectorToSetUnchecked(GetMapKeys(typename Comp::InterfaceBindings));
    using AllPs = SetUncheckedUnion(InterfacePs, typename Comp::Ps);
    using DuplicateTypes = SetIntersection(typename OtherComp::Ps, AllPs);
    using CompConstPs = SetDifference(typename Comp::Ps, typename Comp::NonConstRsPs);
    using CompRs = SetDifference(typename Comp::RsSuperset, typename Comp::Ps);
    using CompNonConstRs = SetIntersection(CompRs, typename Comp::NonConstRsPs);

    using OtherCompConstPs = SetDifference(typename OtherComp::Ps, typename OtherComp::NonConstRsPs);
    using OtherCompRs = SetDifference(typename OtherComp::RsSuperset, typename OtherComp::Ps);
    using OtherCompNonConstRs = SetIntersection(OtherCompRs, typename OtherComp::NonConstRsPs);

    using type = If(Not(IsDisjoint(typename OtherComp::Ps, AllPs)),
                    ConstructErrorWithArgVector(DuplicateTypesInComponentErrorTag, SetToVector(DuplicateTypes)),
                    If(Not(IsDisjoint(CompConstPs, OtherCompNonConstRs)),
                       ConstructError(NonConstBindingRequiredButConstBindingProvidedErrorTag,
                                      GetArbitrarySetElement(SetIntersection(CompConstPs, OtherCompNonConstRs))),
                       If(Not(IsDisjoint(CompNonConstRs, OtherCompConstPs)),
                          ConstructError(NonConstBindingRequiredButConstBindingProvidedErrorTag,
                                         GetArbitrarySetElement(SetIntersection(CompNonConstRs, OtherCompConstPs))),
                          Op)));
  };
};

struct InstallComponentHelper {
  template <typename Comp, typename... OtherCompParams>
  struct apply {
    using OtherComp = ConstructComponentImpl(OtherCompParams...);
    using type = InstallComponent(Comp, OtherComp);
  };
};

struct InstallComponentFunctions {
    template <typename Comp, typename... ComponentFunctions>
    struct apply;

    template <typename Comp>
    struct apply<Comp> {
      using type = ComponentFunctorIdentity(Comp);
    };

    template <typename Comp, typename... ComponentParams, typename... ComponentFunctionArgs, typename... ComponentFunctions>
    struct apply<Comp, Type<fruit::ComponentFunction<fruit::Component<ComponentParams...>, ComponentFunctionArgs...>>, ComponentFunctions...> {
      using type =
          Call(
              Compose2ComponentFunctors(
                  ComponentFunctor(InstallComponent, ConstructComponentImpl(Type<ComponentParams>...)),
                  ComponentFunctor(InstallComponentFunctions, ComponentFunctions...)),
              Comp);
    };

    template <typename Comp, typename T, typename... ComponentFunctions>
    struct apply<Comp, T, ComponentFunctions...> {
        using type = ConstructError(IncorrectArgTypePassedToInstallComponentFuntionsErrorTag, T);
    };
};

// CatchAll(PropagateError(Expr, Bool<false>), IsErrorExceptionHandler) evaluates to Bool<true> if Expr throws an error,
// and Bool<false> otherwise.
struct IsErrorExceptionHandler {
  template <typename E>
  struct apply {
    using type = Bool<true>;
  };
};

struct ConvertComponent {
  template <typename SourceComp, typename DestComp>
  struct apply {
    using SourcePs = typename SourceComp::Ps;
    using DestPs = typename DestComp::Ps;
    using SourceRs = SetDifference(typename SourceComp::RsSuperset, typename SourceComp::Ps);
    using DestRs = SetDifference(typename DestComp::RsSuperset, typename DestComp::Ps);
    using NonConstSourceRs = SetIntersection(SourceRs, typename SourceComp::NonConstRsPs);
    using NonConstDestPs = SetIntersection(DestPs, typename DestComp::NonConstRsPs);
    using NonConstDestRs = SetIntersection(DestRs, typename DestComp::NonConstRsPs);

    using ConstSourcePs = SetDifference(SourcePs, typename SourceComp::NonConstRsPs);
    using ConstDestRs = SetDifference(DestRs, typename DestComp::NonConstRsPs);

    // We need to register:
    // * All the types provided by the new component
    // * All the types required by the old component
    // except:
    // * The ones already provided by the old component (if they have the right constness).
    // * The ones required by the new one (if they have the right constness).
    using ToRegister = SetUnion(
        // The types that we must provide and aren't currently provided
        SetDifference(SetUnion(DestPs, SourceRs), SetUnion(DestRs, SourcePs)),
        // And the ones that are currently provided as const but that we need to provide as non-const
        SetIntersection(SetUnion(NonConstDestPs, NonConstSourceRs), SetUnion(ConstDestRs, ConstSourcePs)));
    using NonConstTypesToRegister = SetIntersection(ToRegister, SetUnion(typename SourceComp::NonConstRsPs,
                                                                         typename DestComp::NonConstRsPs));
    using type = EnsureProvidedTypes(SourceComp, DestRs, NonConstDestRs, SetToVector(ToRegister),
                                     NonConstTypesToRegister);

// Not needed, just double-checking.
// Uses FruitStaticAssert instead of FruitDelegateCheck so that it's checked only in debug mode.
#if FRUIT_EXTRA_DEBUG
    FruitDelegateCheck(
        If(CatchAll(PropagateError(type, PropagateError(Id<GetResult(type)>, Bool<false>)), IsErrorExceptionHandler),
           // We're going to return an error soon anyway, we don't want to interfere by reporting this one.
           None, CheckComponentEntails(GetResult(type), DestComp)));
#endif // FRUIT_EXTRA_DEBUG
  };
};

struct ProcessDeferredBindings {
  template <typename Comp>
  struct apply;

  template <typename RsSupersetParam, typename PsParam, typename NonConstRsPsParam,
#if !FRUIT_NO_LOOP_CHECK
            typename DepsParam,
#endif
            typename InterfaceBindingsParam, typename DeferredBindingFunctors>
  struct apply<Comp<RsSupersetParam, PsParam, NonConstRsPsParam,
#if !FRUIT_NO_LOOP_CHECK
                    DepsParam,
#endif
                    InterfaceBindingsParam, DeferredBindingFunctors>> {
    // Comp1 is the same as Comp, but without the DeferredBindingFunctors.
    using Comp1 = ConsComp(RsSupersetParam, PsParam, NonConstRsPsParam,
#if !FRUIT_NO_LOOP_CHECK
                           DepsParam,
#endif
                           InterfaceBindingsParam, EmptyList);
    using type = Call(FoldList(DeferredBindingFunctors, Compose2ComponentFunctors, ComponentFunctorIdentity), Comp1);
  };
};

template <typename AnnotatedCFunctor, typename AnnotatedCUniquePtrFunctor>
struct AutoRegisterFactoryHelperErrorHandler {
  template <typename E>
  struct apply {
    using type = E;
  };

  template <typename T>
  struct apply<Error<NoBindingFoundErrorTag, T>> {
    using type = If(IsSame(Type<T>, AnnotatedCFunctor), ConstructNoBindingFoundError(AnnotatedCUniquePtrFunctor),
                    ConstructError(NoBindingFoundErrorTag, Type<T>));
  };

  template <typename T1, typename T2>
  struct apply<Error<NoBindingFoundForAbstractClassErrorTag, T1, T2>> {
    using type = If(IsSame(Type<T1>, AnnotatedCFunctor), ConstructNoBindingFoundError(AnnotatedCUniquePtrFunctor),
                    ConstructError(NoBindingFoundForAbstractClassErrorTag, Type<T1>, Type<T2>));
  };
};

struct AutoRegisterFactoryHelper {

  // General case, no way to bind it.
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename InterfaceBinding,
            typename has_inject_annotation, typename is_abstract, typename C, typename AnnotatedSignature,
            typename... Args>
  struct apply {
    using AnnotatedC = SignatureType(AnnotatedSignature);
    using CFunctor = ConsStdFunction(RemoveAnnotationsFromSignature(AnnotatedSignature));
    using AnnotatedCFunctor = CopyAnnotation(AnnotatedC, CFunctor);
    using type = If(IsAbstract(C), ConstructError(NoBindingFoundForAbstractClassErrorTag, AnnotatedCFunctor, C),
                    ConstructError(NoBindingFoundErrorTag, AnnotatedCFunctor));
  };

  // No way to bind it (we need this specialization too to ensure that the specialization below
  // is not chosen for AnnotatedC=None).
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename unused1,
            typename unused2, typename NakedI, typename AnnotatedSignature, typename... Args>
  struct apply<Comp, TargetRequirements, TargetNonConstRequirements, None, unused1, unused2,
               Type<std::unique_ptr<NakedI>>, AnnotatedSignature, Args...> {
    using AnnotatedC = SignatureType(AnnotatedSignature);
    using CFunctor = ConsStdFunction(RemoveAnnotationsFromSignature(AnnotatedSignature));
    using AnnotatedCFunctor = CopyAnnotation(AnnotatedC, CFunctor);
    using type = If(IsAbstract(Type<NakedI>),
                    ConstructError(NoBindingFoundForAbstractClassErrorTag, AnnotatedCFunctor, Type<NakedI>),
                    ConstructError(NoBindingFoundErrorTag, AnnotatedCFunctor));
  };

  // AnnotatedI has an interface binding, use it and look for a factory that returns the type that AnnotatedI is bound
  // to.
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename AnnotatedC,
            typename unused1, typename unused2, typename NakedI, typename AnnotatedSignature, typename... Args>
  struct apply<Comp, TargetRequirements, TargetNonConstRequirements, AnnotatedC, unused1, unused2,
               Type<std::unique_ptr<NakedI>>, AnnotatedSignature, Args...> {
    using I = Type<NakedI>;
    using AnnotatedI = CopyAnnotation(SignatureType(AnnotatedSignature), I);
    using C = RemoveAnnotations(AnnotatedC);
    using IFunctor = ConsStdFunction(ConsSignature(ConsUniquePtr(I), Args...));
    using CFunctor = ConsStdFunction(ConsSignature(ConsUniquePtr(C), Args...));
    using AnnotatedIFunctor = CopyAnnotation(AnnotatedI, IFunctor);
    using AnnotatedCFunctor = CopyAnnotation(AnnotatedC, CFunctor);

    using ProvidedSignature = ConsSignature(AnnotatedIFunctor,
                                            CopyAnnotation(AnnotatedC, ConsConstReference(CFunctor)));
    using LambdaSignature = ConsSignature(IFunctor, ConsConstReference(CFunctor));

    using F1 = ComponentFunctor(EnsureProvidedType, TargetRequirements, TargetNonConstRequirements, AnnotatedCFunctor,
                                Bool<false>);
    using F2 = ComponentFunctor(PreProcessRegisterProvider, ProvidedSignature, LambdaSignature);
    using F3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, LambdaSignature);
    using R = Call(ComposeFunctors(F1, F2, F3), Comp);
    struct Op {
      using Result = Eval<GetResult(R)>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        using NakedC = UnwrapType<Eval<C>>;
        auto provider = [](const UnwrapType<Eval<CFunctor>>& fun) {
          return UnwrapType<Eval<IFunctor>>([=](typename TypeUnwrapper<Args>::type... args) {
            NakedC* c = fun(args...).release();
            NakedI* i = static_cast<NakedI*>(c);
            return std::unique_ptr<NakedI>(i);
          });
        };
        using RealF2 = ComponentFunctor(PreProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealF3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealOp = Call(ComposeFunctors(F1, RealF2, RealF3), Comp);
        FruitStaticAssert(IsSame(GetResult(RealOp), GetResult(R)));
        Eval<RealOp>()(entries);
      }
      std::size_t numEntries() {
#if FRUIT_EXTRA_DEBUG
        using NakedC = UnwrapType<Eval<C>>;
        auto provider = [](const UnwrapType<Eval<CFunctor>>& fun) {
          return UnwrapType<Eval<IFunctor>>([=](typename TypeUnwrapper<Args>::type... args) {
            NakedC* c = fun(args...).release();
            NakedI* i = static_cast<NakedI*>(c);
            return std::unique_ptr<NakedI>(i);
          });
        };
        using RealF2 = ComponentFunctor(PreProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealF3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealOp = Call(ComposeFunctors(F1, RealF2, RealF3), Comp);
        FruitAssert(Eval<R>().numEntries() == Eval<RealOp>().numEntries());
#endif
        return Eval<R>().numEntries();
      }
    };
    using type = PropagateError(R, If(Not(HasVirtualDestructor(I)),
                                      ConstructError(FactoryBindingForUniquePtrOfClassWithNoVirtualDestructorErrorTag,
                                                     IFunctor, CFunctor),
                                      Op));
  };

  // C doesn't have an interface binding as interface, nor an INJECT annotation, and is not an abstract class.
  // Bind std::function<unique_ptr<C>(Args...)> to std::function<C(Args...)> (possibly with annotations).
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename NakedC,
            typename AnnotatedSignature, typename... Args>
  struct apply<Comp, TargetRequirements, TargetNonConstRequirements, None, Bool<false>, Bool<false>,
               Type<std::unique_ptr<NakedC>>, AnnotatedSignature, Args...> {
    using C = Type<NakedC>;
    using CFunctor = ConsStdFunction(ConsSignature(C, Args...));
    using CUniquePtrFunctor = ConsStdFunction(ConsSignature(ConsUniquePtr(C), Args...));
    using AnnotatedCUniquePtr = SignatureType(AnnotatedSignature);
    using AnnotatedC = CopyAnnotation(AnnotatedCUniquePtr, C);
    using AnnotatedCFunctor = CopyAnnotation(AnnotatedCUniquePtr, CFunctor);
    using AnnotatedCUniquePtrFunctor = CopyAnnotation(AnnotatedCUniquePtr, CUniquePtrFunctor);
    using AnnotatedCFunctorRef = CopyAnnotation(AnnotatedCUniquePtr, ConsConstReference(CFunctor));

    using ProvidedSignature = ConsSignature(AnnotatedCUniquePtrFunctor, AnnotatedCFunctorRef);
    using LambdaSignature = ConsSignature(CUniquePtrFunctor, ConsConstReference(CFunctor));

    using F1 = ComponentFunctor(EnsureProvidedType, TargetRequirements, TargetNonConstRequirements, AnnotatedCFunctor,
                                Bool<false>);
    using F2 = ComponentFunctor(PreProcessRegisterProvider, ProvidedSignature, LambdaSignature);
    using F3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, LambdaSignature);
    using R = Call(ComposeFunctors(F1, F2, F3), Comp);
    struct Op {
      using Result = Eval<GetResult(R)>;
      void operator()(FixedSizeVector<ComponentStorageEntry>& entries) {
        auto provider = [](const UnwrapType<Eval<CFunctor>>& fun) {
          return UnwrapType<Eval<CUniquePtrFunctor>>([=](typename TypeUnwrapper<Args>::type... args) {
            NakedC* c = new NakedC(fun(args...));
            return std::unique_ptr<NakedC>(c);
          });
        };
        using RealF2 = ComponentFunctor(PreProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealF3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealOp = Call(ComposeFunctors(F1, RealF2, RealF3), Comp);
        FruitStaticAssert(IsSame(GetResult(RealOp), GetResult(R)));
        Eval<RealOp>()(entries);
      }
      std::size_t numEntries() {
#if FRUIT_EXTRA_DEBUG
        auto provider = [](const UnwrapType<Eval<CFunctor>>& fun) {
          return UnwrapType<Eval<CUniquePtrFunctor>>([=](typename TypeUnwrapper<Args>::type... args) {
            NakedC* c = new NakedC(fun(args...));
            return std::unique_ptr<NakedC>(c);
          });
        };
        using RealF2 = ComponentFunctor(PreProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealF3 = ComponentFunctor(PostProcessRegisterProvider, ProvidedSignature, Type<decltype(provider)>);
        using RealOp = Call(ComposeFunctors(F1, RealF2, RealF3), Comp);
        FruitAssert(Eval<R>().numEntries() == Eval<RealOp>().numEntries());
#endif
        return Eval<R>().numEntries();
      }
    };

    using ErrorHandler =
        AutoRegisterFactoryHelperErrorHandler<Eval<AnnotatedCFunctor>, Eval<AnnotatedCUniquePtrFunctor>>;

    // If we are about to report a NoBindingFound/NoBindingFoundForAbstractClass error for AnnotatedCFunctor,
    // report one for std::function<std::unique_ptr<C>(Args...)> instead,
    // otherwise we'd report an error about a type that the user doesn't expect.
    using type = PropagateError(Catch(Catch(R, NoBindingFoundErrorTag, ErrorHandler),
                                      NoBindingFoundForAbstractClassErrorTag, ErrorHandler),
                                Op);
  };

  // C has an Inject typedef, use it. unique_ptr case.
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename unused,
            typename NakedC, typename AnnotatedSignature, typename... Args>
  struct apply<Comp, TargetRequirements, TargetNonConstRequirements, None, Bool<true>, unused,
               Type<std::unique_ptr<NakedC>>, AnnotatedSignature, Args...> {
    using AnnotatedCUniquePtr = SignatureType(AnnotatedSignature);
    using AnnotatedC = CopyAnnotation(AnnotatedCUniquePtr, RemoveUniquePtr(RemoveAnnotations(AnnotatedCUniquePtr)));
    using DecoratedSignatureReturningValue = GetInjectAnnotation(AnnotatedC);
    using DecoratedSignature = ConsSignatureWithVector(AnnotatedCUniquePtr,
                                                       SignatureArgs(DecoratedSignatureReturningValue));
    using DecoratedSignatureArgs = SignatureArgs(DecoratedSignature);
    using ActualSignatureInInjectionTypedef = ConsSignatureWithVector(SignatureType(DecoratedSignature),
                                                                      RemoveNonAssisted(DecoratedSignatureArgs));
    using NonAssistedArgs = RemoveAssisted(DecoratedSignatureArgs);

    using F1 = ComponentFunctor(RegisterConstructorAsUniquePtrFactory, DecoratedSignature);
    using F2 = ComponentFunctor(EnsureProvidedTypes, TargetRequirements, TargetNonConstRequirements,
                                NormalizeTypeVector(NonAssistedArgs), NormalizedNonConstTypesIn(NonAssistedArgs));

    using type = If(Not(IsSame(AnnotatedSignature, ActualSignatureInInjectionTypedef)),
                    ConstructError(FunctorSignatureDoesNotMatchErrorTag, AnnotatedSignature,
                                   ActualSignatureInInjectionTypedef),
                    Call(ComposeFunctors(F1, F2), Comp));
  };

  // C has an Inject typedef, use it. Value (not unique_ptr) case.
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename unused,
            typename NakedC, typename AnnotatedSignature, typename... Args>
  struct apply<Comp, TargetRequirements, TargetNonConstRequirements, None, Bool<true>, unused, Type<NakedC>,
               AnnotatedSignature, Args...> {
    using AnnotatedC = SignatureType(AnnotatedSignature);
    using DecoratedSignature = GetInjectAnnotation(AnnotatedC);
    using DecoratedSignatureArgs = SignatureArgs(DecoratedSignature);
    using ActualSignatureInInjectionTypedef = ConsSignatureWithVector(SignatureType(DecoratedSignature),
                                                                      RemoveNonAssisted(DecoratedSignatureArgs));
    using NonAssistedArgs = RemoveAssisted(DecoratedSignatureArgs);

    using F1 = ComponentFunctor(RegisterConstructorAsValueFactory, DecoratedSignature);
    using F2 = ComponentFunctor(EnsureProvidedTypes, TargetRequirements, TargetNonConstRequirements,
                                NormalizeTypeVector(NonAssistedArgs), NormalizedNonConstTypesIn(NonAssistedArgs));

    using type = If(Not(IsSame(AnnotatedSignature, ActualSignatureInInjectionTypedef)),
                    ConstructError(FunctorSignatureDoesNotMatchErrorTag, AnnotatedSignature,
                                   ActualSignatureInInjectionTypedef),
                    Call(ComposeFunctors(F1, F2), Comp));
  };
};

struct AutoRegister {
  // The types in TargetRequirements will not be auto-registered.
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename AnnotatedC>
  struct apply;

  // Tries to register C by looking for a typedef called Inject inside C.
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename AnnotatedC>
  struct apply {
    using CHasInjectAnnotation = HasInjectAnnotation(RemoveAnnotations(AnnotatedC));
    using Inject = GetInjectAnnotation(AnnotatedC);
    using CRequirements = NormalizeTypeVector(SignatureArgs(Inject));
    using CNonConstRequirements = NormalizedNonConstTypesIn(SignatureArgs(Inject));
    using F = ComposeFunctors(ComponentFunctor(PreProcessRegisterConstructor, Inject),
                              ComponentFunctor(PostProcessRegisterConstructor, Inject),
                              ComponentFunctor(EnsureProvidedTypes, TargetRequirements, TargetNonConstRequirements,
                                               CRequirements, CNonConstRequirements));
    using type = If(CHasInjectAnnotation, Call(F, Comp), ConstructNoBindingFoundError(AnnotatedC));
  };

  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename NakedC,
            typename... NakedArgs>
  struct apply<Comp, TargetRequirements, TargetNonConstRequirements, Type<std::function<NakedC(NakedArgs...)>>> {
    using type = AutoRegisterFactoryHelper(Comp, TargetRequirements, TargetNonConstRequirements,
                                           FindInMap(typename Comp::InterfaceBindings, Type<NakedC>),
                                           HasInjectAnnotation(Type<NakedC>), IsAbstract(Type<NakedC>), Type<NakedC>,
                                           Type<NakedC(NakedArgs...)>, Id<RemoveAnnotations(Type<NakedArgs>)>...);
  };

  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename NakedC,
            typename... NakedArgs>
  struct apply<Comp, TargetRequirements, TargetNonConstRequirements,
               Type<std::function<std::unique_ptr<NakedC>(NakedArgs...)>>> {
    using type = AutoRegisterFactoryHelper(Comp, TargetRequirements, TargetNonConstRequirements,
                                           FindInMap(typename Comp::InterfaceBindings, Type<NakedC>),
                                           HasInjectAnnotation(Type<NakedC>), IsAbstract(Type<NakedC>),
                                           Type<std::unique_ptr<NakedC>>, Type<std::unique_ptr<NakedC>(NakedArgs...)>,
                                           Id<RemoveAnnotations(Type<NakedArgs>)>...);
  };

  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename Annotation,
            typename NakedC, typename... NakedArgs>
  struct apply<Comp, TargetRequirements, TargetNonConstRequirements,
               Type<fruit::Annotated<Annotation, std::function<NakedC(NakedArgs...)>>>> {
    using type = AutoRegisterFactoryHelper(Comp, TargetRequirements, TargetNonConstRequirements,
                                           FindInMap(typename Comp::InterfaceBindings,
                                                     Type<fruit::Annotated<Annotation, NakedC>>),
                                           HasInjectAnnotation(Type<NakedC>), IsAbstract(Type<NakedC>), Type<NakedC>,
                                           Type<fruit::Annotated<Annotation, NakedC>(NakedArgs...)>,
                                           Id<RemoveAnnotations(Type<NakedArgs>)>...);
  };

  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename Annotation,
            typename NakedC, typename... NakedArgs>
  struct apply<Comp, TargetRequirements, TargetNonConstRequirements,
               Type<fruit::Annotated<Annotation, std::function<std::unique_ptr<NakedC>(NakedArgs...)>>>> {
    using type = AutoRegisterFactoryHelper(Comp, TargetRequirements, TargetNonConstRequirements,
                                           FindInMap(typename Comp::InterfaceBindings,
                                                     Type<fruit::Annotated<Annotation, NakedC>>),
                                           HasInjectAnnotation(Type<NakedC>), IsAbstract(Type<NakedC>),
                                           Type<std::unique_ptr<NakedC>>,
                                           Type<fruit::Annotated<Annotation, std::unique_ptr<NakedC>>(NakedArgs...)>,
                                           Id<RemoveAnnotations(Type<NakedArgs>)>...);
  };
};

template <typename AnnotatedT>
struct EnsureProvidedTypeErrorHandler {
  template <typename E>
  struct apply {
    using type = E;
  };

  template <typename T>
  struct apply<Error<NoBindingFoundErrorTag, T>> {
    using type = If(IsSame(Type<T>, AnnotatedT),
                    ConstructError(ConstBindingDeclaredAsRequiredButNonConstBindingRequiredErrorTag, AnnotatedT),
                    ConstructError(NoBindingFoundErrorTag, Type<T>));
  };

  template <typename T1, typename T2>
  struct apply<Error<NoBindingFoundForAbstractClassErrorTag, T1, T2>> {
    using type = If(IsSame(Type<T1>, AnnotatedT),
                    ConstructError(ConstBindingDeclaredAsRequiredButNonConstBindingRequiredErrorTag, AnnotatedT),
                    ConstructError(NoBindingFoundForAbstractClassErrorTag, Type<T1>, Type<T2>));
  };
};

struct EnsureProvidedType {
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename AnnotatedT,
            typename NonConstBindingRequired>
  struct apply {
    using AnnotatedC = NormalizeType(AnnotatedT);
    using AnnotatedCImpl = FindInMap(typename Comp::InterfaceBindings, AnnotatedC);
    using AutoRegisterResult = AutoRegister(Comp, TargetRequirements, TargetNonConstRequirements, AnnotatedC);
    using ErrorHandler = EnsureProvidedTypeErrorHandler<AnnotatedT>;
    using type = If(
        IsInSet(AnnotatedC, typename Comp::Ps),
        If(And(NonConstBindingRequired, Not(IsInSet(AnnotatedC, typename Comp::NonConstRsPs))),
           ConstructError(NonConstBindingRequiredButConstBindingProvidedErrorTag, AnnotatedC),
           ComponentFunctorIdentity(Comp)),
        If(And(IsInSet(AnnotatedC, TargetRequirements),
               Or(Not(NonConstBindingRequired), IsInSet(AnnotatedC, TargetNonConstRequirements))),
           // The type is already in the target requirements with the desired constness, nothing to do.
           ComponentFunctorIdentity(Comp),
           If(Not(IsNone(AnnotatedCImpl)),
              // Has an interface binding.
              Call(ComposeFunctors(ComponentFunctor(ProcessInterfaceBinding, AnnotatedC, AnnotatedCImpl,
                                                    NonConstBindingRequired),
                                   ComponentFunctor(EnsureProvidedType, TargetRequirements, TargetNonConstRequirements,
                                                    AnnotatedCImpl, NonConstBindingRequired)),
                   Comp),
              // If we are about to report a NoBindingFound/NoBindingFoundForAbstractClass error for AnnotatedT and the
              // target
              // component has a Required<const T>, we can report a more specific error (rather than the usual
              // "binding not found").
              If(And(NonConstBindingRequired, IsInSet(AnnotatedC, TargetRequirements)),
                 Catch(Catch(AutoRegisterResult, NoBindingFoundErrorTag, ErrorHandler),
                       NoBindingFoundForAbstractClassErrorTag, ErrorHandler),
                 AutoRegisterResult))));
  };
};

struct EnsureProvidedTypes {
  template <typename Comp, typename TargetRequirements, typename TargetNonConstRequirements, typename TypesToProvide,
            typename NonConstTypesToProvide>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename T>
      struct apply {
        using type = Compose2ComponentFunctors(ComponentFunctor(EnsureProvidedType, TargetRequirements,
                                                                TargetNonConstRequirements, T,
                                                                IsInSet(T, NonConstTypesToProvide)),
                                               CurrentResult);
      };
    };

    using type = Call(FoldVector(TypesToProvide, Helper, ComponentFunctorIdentity), Comp);
  };
};

struct ProcessBinding {
  template <typename Binding>
  struct apply;

  template <typename I, typename C>
  struct apply<fruit::impl::Bind<I, C>> {
    using type = ComponentFunctor(AddDeferredInterfaceBinding, Type<I>, Type<C>);
  };

  template <typename Signature>
  struct apply<fruit::impl::RegisterConstructor<Signature>> {
    using type = ComponentFunctor(DeferredRegisterConstructor, Type<Signature>);
  };

  template <typename AnnotatedC, typename C>
  struct apply<fruit::impl::BindInstance<AnnotatedC, C>> {
    using type = ComponentFunctor(RegisterInstance, Type<AnnotatedC>, Type<C>, Bool<true>);
  };

  template <typename AnnotatedC, typename C>
  struct apply<fruit::impl::BindConstInstance<AnnotatedC, C>> {
    using type = ComponentFunctor(RegisterInstance, Type<AnnotatedC>, Type<C>, Bool<false>);
  };

  template <typename Lambda>
  struct apply<fruit::impl::RegisterProvider<Lambda>> {
    using type = ComponentFunctor(DeferredRegisterProvider, Type<Lambda>);
  };

  template <typename AnnotatedSignature, typename Lambda>
  struct apply<fruit::impl::RegisterProvider<AnnotatedSignature, Lambda>> {
    using type = ComponentFunctor(DeferredRegisterProviderWithAnnotations, Type<AnnotatedSignature>, Type<Lambda>);
  };

  template <typename AnnotatedC>
  struct apply<fruit::impl::AddInstanceMultibinding<AnnotatedC>> {
    using type = ComponentFunctorIdentity;
  };

  template <typename AnnotatedC>
  struct apply<fruit::impl::AddInstanceVectorMultibindings<AnnotatedC>> {
    using type = ComponentFunctorIdentity;
  };

  template <typename I, typename C>
  struct apply<fruit::impl::AddMultibinding<I, C>> {
    using type = ComponentFunctor(AddInterfaceMultibinding, Type<I>, Type<C>);
  };

  template <typename Lambda>
  struct apply<fruit::impl::AddMultibindingProvider<Lambda>> {
    using type = ComponentFunctor(RegisterMultibindingProvider, Type<Lambda>);
  };

  template <typename AnnotatedSignature, typename Lambda>
  struct apply<fruit::impl::AddMultibindingProvider<AnnotatedSignature, Lambda>> {
    using type = ComponentFunctor(RegisterMultibindingProviderWithAnnotations, Type<AnnotatedSignature>, Type<Lambda>);
  };

  template <typename DecoratedSignature, typename Lambda>
  struct apply<fruit::impl::RegisterFactory<DecoratedSignature, Lambda>> {
    using type = ComponentFunctor(RegisterFactory, Type<DecoratedSignature>, Type<Lambda>);
  };

  template <typename... Params, typename... Args>
  struct apply<fruit::impl::InstallComponent<fruit::Component<Params...>(Args...)>> {
    using type = ComponentFunctor(InstallComponentHelper, Type<Params>...);
  };

  template <typename... ComponentFunctions>
  struct apply<fruit::impl::InstallComponentFunctions<ComponentFunctions...>> {
    using type = ComponentFunctor(InstallComponentFunctions, Type<ComponentFunctions>...);
  };

  template <typename GetReplacedComponent, typename GetReplacementComponent>
  struct apply<fruit::impl::ReplaceComponent<GetReplacedComponent, GetReplacementComponent>> {
    using type = ComponentFunctorIdentity;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_COMPONENT_FUNCTORS_DEFN_H
