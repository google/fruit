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

#ifndef FRUIT_META_COMPONENT_H
#define FRUIT_META_COMPONENT_H

#include <fruit/fruit_forward_decls.h>
#include <fruit/impl/fruit_internal_forward_decls.h>
#include <fruit/impl/meta/algos.h>
#include <fruit/impl/meta/set.h>
#include <fruit/impl/meta/map.h>
#include <fruit/impl/meta/list.h>
#include <fruit/impl/meta/metaprogramming.h>
#include <fruit/impl/meta/errors.h>
#include <fruit/impl/meta/proof_trees.h>
#include <fruit/impl/meta/wrappers.h>
#include <fruit/impl/meta/signatures.h>
#include <fruit/impl/injection_debug_errors.h>

#include <memory>

namespace fruit {
namespace impl {
namespace meta {

//********************************************************************************************************************************
// Part 1: Simple type functors (no ConsComp involved).
//********************************************************************************************************************************

// Given a type T, returns the class that should be injected to ensure that T is provided at runtime (if any).
struct GetClassForType {
  // General case, if none of the following apply.
  // When adding a specialization here, make sure that the ComponentStorage
  // can actually get<> the specified type when the class was registered.
  template <typename T>
  struct apply;

  template <typename T>
  struct apply<Type<T>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<const T>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<T*>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<T&>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<const T*>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<const T&>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<std::shared_ptr<T>>> {using type = Type<T>;};
  
  template <typename T>
  struct apply<Type<Assisted<T>>> {using type = None;};
  
  template <typename T>
  struct apply<Type<Provider<T>>> {using type = Type<T>;};
  
  template <typename Annotation, typename T>
  struct apply<Type<fruit::Annotated<Annotation, T>>> {using type = Type<T>;};
};

struct GetClassForTypeVector {
  template <typename V>
  struct apply {
    using type = TransformVector(V, GetClassForType);
  };
};

// Given a type T, returns the type in the injection graph that corresponds to T.
struct NormalizeType {
  // General case, if none of the following apply.
  // When adding a specialization here, make sure that the ComponentStorage
  // can actually get<> the specified type when the class was registered.
  template <typename T>
  struct apply;
  
  template <typename T>
  struct apply<Type<T>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<const T>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<T*>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<T&>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<const T*>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<const T&>> {using type = Type<T>;};

  template <typename T>
  struct apply<Type<std::shared_ptr<T>>> {using type = Type<T>;};
  
  template <typename T>
  struct apply<Type<Assisted<T>>> {using type = None;};
  
  template <typename T>
  struct apply<Type<Provider<T>>> {using type = Type<T>;};
  
  template <typename Annotation, typename T>
  struct apply<Type<fruit::Annotated<Annotation, T>>> {using type = Type<fruit::Annotated<Annotation, UnwrapType<Eval<NormalizeType(Type<T>)>>>>;};
};

struct NormalizeTypeVector {
  template <typename V>
  struct apply {
    using type = TransformVector(V, NormalizeType);
  };
};

// Returns U wrapped in the same annotations in AnnotatedT (if any).
struct CopyAnnotation {
  template <typename AnnotatedT, typename U>
  struct apply;
  
  template <typename T, typename U>
  struct apply {
    using type = U;
  };
  
  template <typename Annotation, typename T, typename U>
  struct apply<Type<fruit::Annotated<Annotation, T>>, Type<U>> {
    using type = Type<fruit::Annotated<Annotation, U>>;
  };
};

struct IsValidSignature {
  template <typename Signature>
  struct apply {
    using type = Bool<false>;
  };

  template <typename T, typename... Args>
  struct apply<Type<T(Args...)>> {
    using type = Bool<true>;
  };
};

// Removes the Annotation (if any) wrapping a type T.
struct RemoveAnnotations {
  template <typename T>
  struct apply;
  
  template <typename T>
  struct apply<Type<T>> {
    using type = Type<T>;
  };
  
  template <typename Annotation, typename T>
  struct apply<Type<fruit::Annotated<Annotation, T>>> {
    using type = Type<T>;
  };
};

// Removes the Annotation(s) (if any) wrapping the types in AnnotatedSignature.
struct RemoveAnnotationsFromSignature {
  template <typename AnnotatedSignature>
  struct apply {
    using type = ConstructError(NotASignatureErrorTag, AnnotatedSignature);
  };
  
  template <typename AnnotatedT, typename... AnnotatedArgs>
  struct apply<Type<AnnotatedT(AnnotatedArgs...)>> {
    using type = ConsSignature(RemoveAnnotations(Type<AnnotatedT>),
                               Id<RemoveAnnotations(Type<AnnotatedArgs>)>...);
  };
};

// Removes the Annotation(s) (if any) wrapping the types in the Vector V.
struct RemoveAnnotationsFromVector {
  template <typename V>
  struct apply {
    using type = TransformVector(V, RemoveAnnotations);
  };
};

// Maps T->T* in a possibly-annotated type.
struct AddPointerInAnnotatedType {
  template <typename T>
  struct apply;
  
  template <typename T>
  struct apply<Type<T>> {
    using type = Type<T*>;
  };
  
  template <typename Annotation, typename T>
  struct apply<Type<fruit::Annotated<Annotation, T>>> {
    using type = Type<fruit::Annotated<Annotation, T*>>;
  };
};

// TODO: This also does UnlabelAssisted<>. Consider renaming and/or removing that logic (and
// letting callers do the unlabeling when desired).
struct RemoveNonAssisted {
  template <typename V>
  struct apply {
    struct Helper {
      // Non-assisted case
      template <typename CurrentResult, typename T>
      struct apply {
        using type = CurrentResult;
      };

      template <typename CurrentResult, typename T>
      struct apply<CurrentResult, Type<Assisted<T>>> {
        using type = PushBack(CurrentResult, Type<T>);
      };
    };
    
    using type = FoldVector(V, Helper, Vector<>);
  };
};

struct RemoveAssisted {
  template <typename V>
  struct apply {
    struct Helper {
      // Non-assisted case
      template <typename CurrentResult, typename T>
      struct apply {
        using type = PushBack(CurrentResult, T);
      };

      // Assisted case
      template <typename CurrentResult, typename T>
      struct apply<CurrentResult, Type<Assisted<T>>> {
        using type = CurrentResult;
      };
    };
    
    using type = FoldVector(V, Helper, Vector<>);
  };
};

struct UnlabelAssistedSingleType {
  template <typename T>
  struct apply;
  
  template <typename T>
  struct apply<Type<T>> {
    using type = Type<T>;
  };

  template <typename T>
  struct apply<Type<Assisted<T>>> {
    using type = Type<T>;
  };
};

struct UnlabelAssisted {
  template <typename V>
  struct apply {
    using type = TransformVector(V, UnlabelAssistedSingleType);
  };
};

struct RequiredLambdaArgsForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = RemoveAnnotationsFromVector(UnlabelAssisted(SignatureArgs(AnnotatedSignature)));
  };
};

struct RequiredLambdaSignatureForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = ConsSignatureWithVector(RemoveAnnotations(SignatureType(AnnotatedSignature)),
                                         RequiredLambdaArgsForAssistedFactory(AnnotatedSignature));
  };
};

struct InjectedFunctionArgsForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = RemoveNonAssisted(SignatureArgs(AnnotatedSignature));
  };
};

struct InjectedSignatureForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = ConsSignatureWithVector(RemoveAnnotations(SignatureType(AnnotatedSignature)),
                                         InjectedFunctionArgsForAssistedFactory(AnnotatedSignature));
  };
};

struct IsAssisted {
  template <typename T>
  struct apply {
    using type = Bool<false>;
  };
  
  template <typename T>
  struct apply<Type<Assisted<T>>> {
    using type = Bool<true>;
  };
};

struct NumAssisted {
  template <typename V>
  struct apply;
  
  template <typename... Types>
  struct apply<Vector<Types...>> {
    static constexpr int sum = staticSum(IsAssisted::apply<Types>::type::value...);
    using type = Int<sum>;
  };
};

// Counts the number of Assisted<> types in V before the given index.
struct NumAssistedBefore {
  template <typename Index, typename V>
  struct apply;
  
  template <typename V>
  struct apply<Int<0>, V> {
    using type = Int<0>;
  };
  
  template <int n, typename V>
  struct apply<Int<n>, V> {
    using type = Minus(NumAssisted(V),
                       NumAssisted(VectorRemoveFirstN(V, Int<n>)));
  };
};

// Checks whether C is auto-injectable thanks to an Inject typedef.
struct HasInjectAnnotation {
  template <typename C>
  struct apply;
  
  template <typename C>
  struct apply<Type<C>> {
    template <typename C1>
    static Bool<true> test(typename C1::Inject*);

    template <typename>
    static Bool<false> test(...);
    
    using type = decltype(test<C>(nullptr));
  };
};

struct DoGetInjectAnnotation {
  template <typename C>
  struct apply;
  
  template <typename C>
  struct apply<Type<C>> {
    using type = Type<typename C::Inject>;
  };
};

struct GetInjectAnnotation {
  template <typename AnnotatedC>
  struct apply {
    using C = RemoveAnnotations(AnnotatedC);
    using DecoratedS = DoGetInjectAnnotation(C);
    using SResult = SignatureType(DecoratedS);
    using AnnotatedSArgs = SignatureArgs(DecoratedS);
    using SArgs = RemoveAnnotationsFromVector(UnlabelAssisted(AnnotatedSArgs));
    // We replace the non-annotated return type with the potentially-annotated AnnotatedC.
    using AnnotatedDecoratedS = ConsSignatureWithVector(AnnotatedC, AnnotatedSArgs);
    using type = If(IsAbstract(C),
                    ConstructError(CannotConstructAbstractClassErrorTag, C),
                 If(Not(IsValidSignature(DecoratedS)),
                    ConstructError(InjectTypedefNotASignatureErrorTag, C, DecoratedS),
                 If(Not(IsSame(SResult, RemoveAnnotations(SResult))),
                   ConstructError(InjectTypedefWithAnnotationErrorTag, C),
                 If(Not(IsSame(C, SResult)),
                   ConstructError(InjectTypedefForWrongClassErrorTag, C, SResult),
                 If(Not(IsConstructibleWithVector(C, SArgs)),
                    ConstructError(NoConstructorMatchingInjectSignatureErrorTag, C, ConsSignatureWithVector(SResult, SArgs)),
                 AnnotatedDecoratedS)))));
  };
};

// Takes a vector of args, possibly including Provider<>s, and returns the set of required types.
struct ExpandProvidersInParams {
  template <typename V>
  struct apply {
    struct Helper {
      template <typename AnnotatedT>
      struct apply;

      // AnnotatedT is not of the form Provider<C>
      template <typename AnnotatedT>
      struct apply<Type<AnnotatedT>> {
        using type = Type<AnnotatedT>;
      };

      // Type of the form Provider<C>
      template <typename C>
      struct apply<Type<fruit::Provider<C>>> {
        using type = Type<C>;
      };

      // Type of the form Annotated<Annotation, Provider<C>>
      template <typename Annotation, typename C>
      struct apply<Type<fruit::Annotated<Annotation, fruit::Provider<C>>>> {
        using type = Type<fruit::Annotated<Annotation, C>>;
      };
    };
    
    using type = TransformVector(V, Helper);
  };
};

//********************************************************************************************************************************
// Part 2: Type functors involving at least one ConsComp.
//********************************************************************************************************************************

template <typename RsSupersetParam, 
          typename PsParam, 
#ifndef FRUIT_NO_LOOP_CHECK
          typename DepsParam, 
#endif
          typename InterfaceBindingsParam, 
          typename DeferredBindingFunctorsParam>
struct Comp {
  // The actual set of requirements is SetDifference(RsSuperset, Ps)
  // We don't store Rs explicitly because we'd need to remove elements very often (and that's slow).
  using RsSuperset = RsSupersetParam;
  
  using Ps = PsParam;
#ifndef FRUIT_NO_LOOP_CHECK  
  using Deps = DepsParam;
#endif
  using InterfaceBindings = InterfaceBindingsParam;
  using DeferredBindingFunctors = DeferredBindingFunctorsParam;
  
  // Invariants:
  // * all types appearing as arguments of Deps are in Rs
  // * all types in Ps are at the head of one (and only one) Dep.
  //   (note that the types in Rs can appear in deps any number of times, 0 is also ok)
  // * Deps is of the form Vector<Dep...> with each Dep of the form T(Args...) and where Vector<Args...> is a set (no repetitions).
  // * Bindings is a proof tree forest, with injected classes as formulas.
  // * Each element X of the list DeferredBindingFunctors has:
  //   - a default-constructible X::apply<Comp> type
  //   - a void X::apply<Comp>::operator(ComponentStorage&)
  //   - an X::apply<Comp>::Result type
};

// Using ConsComp instead of Comp<...> in a meta-expression allows the types to be evaluated.
// See ConsVector for more details.
struct ConsComp {
  template <typename RsSupersetParam,
            typename PsParam,
#ifndef FRUIT_NO_LOOP_CHECK
            typename DepsParam,
#endif
            typename InterfaceBindingsParam,
            typename DeferredBindingFunctorsParam>
  struct apply {
    using type = Comp<
        RsSupersetParam, PsParam, 
#ifndef FRUIT_NO_LOOP_CHECK
        DepsParam, 
#endif
        InterfaceBindingsParam, DeferredBindingFunctorsParam>;
  };
};

struct GetComponentDeps {
  template <typename Comp>
  struct apply {
    using type = typename Comp::Deps;
  };
};

struct GetComponentPs {
  template <typename Comp>
  struct apply {
    using type = typename Comp::Ps;
  };
};

struct GetComponentRsSuperset {
  template <typename Comp>
  struct apply {
    using type = typename Comp::RsSuperset;
  };
};

// Checks that Types... are normalized types. If not it returns an appropriate error.
// If they are all normalized types this returns Result.
struct CheckNormalizedTypes {
  template <typename... Types>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename T>
      struct apply {
        using NormalizedType = NormalizeType(T);
        using type = If(Not(IsSame(NormalizedType, T)),
                        ConstructError(NonClassTypeErrorTag, RemoveAnnotations(T), RemoveAnnotations(NormalizedType)),
                     CurrentResult);
      };
    };
    
    using type = Fold(Helper, None, Types...);
  };
};

// Checks that Types... are not annotated types. If they have an annotation it returns an appropriate error.
// If none of them is annotated, this returns None.
struct CheckNotAnnotatedTypes {
  template <typename... Types>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename T>
      struct apply {
        using TypeWithoutAnnotations = RemoveAnnotations(T);
        using type = If(Not(IsSame(TypeWithoutAnnotations, T)),
                        ConstructError(AnnotatedTypeErrorTag, T, TypeWithoutAnnotations),
                     CurrentResult);
      };
    };
    
    using type = Fold(Helper, None, Types...);
  };
};

// Checks that there are no repetitions in Types. If there are, it returns an appropriate error.
// If there are no repetitions it returns None.
struct CheckNoRepeatedTypes {
  template <typename... Types>
  struct apply {
    using type = If(HasDuplicates(Vector<Types...>),
                    ConstructError(RepeatedTypesErrorTag, Types...),
                    None);
  };
};

struct ConstructComponentImpl {
  // Non-specialized case: no requirements.
  template <typename... Ps>
  struct apply {
    using type = PropagateError(CheckNoRepeatedTypes(Ps...),
                 PropagateError(CheckNormalizedTypes(Ps...),
                 ConsComp(EmptySet,
                          VectorToSetUnchecked(Vector<Ps...>),
#ifndef FRUIT_NO_LOOP_CHECK                          
                          Vector<Pair<Ps, Vector<>>...>,
#endif
                          Vector<>,
                          EmptyList)));
  };

  // With requirements.
  template <typename... Rs, typename... Ps>
  struct apply<Type<Required<Rs...>>, Ps...> {
    using type1 = PropagateError(CheckNoRepeatedTypes(Type<Rs>..., Ps...),
                  PropagateError(CheckNormalizedTypes(Type<Rs>...),
                  PropagateError(CheckNormalizedTypes(Ps...),
                  ConsComp(VectorToSetUnchecked(Vector<Type<Rs>...>),
                           VectorToSetUnchecked(Vector<Ps...>),
#ifndef FRUIT_NO_LOOP_CHECK
                           Vector<Pair<Ps, Vector<Type<Rs>...>>...>,
#endif
                           Vector<>,
                           EmptyList))));
    
#if !defined(FRUIT_NO_LOOP_CHECK) && defined(FRUIT_EXTRA_DEBUG)
    using Loop = ProofForestFindLoop(GetComponentDeps(type1));
    using type = If(IsNone(Loop),
                    type1,
                    ConstructErrorWithArgVector(SelfLoopErrorTag, Loop));
#else // defined(FRUIT_NO_LOOP_CHECK) || !defined(FRUIT_EXTRA_DEBUG)
    using type = type1;
#endif // defined(FRUIT_NO_LOOP_CHECK) || !defined(FRUIT_EXTRA_DEBUG)
  };
};

// Adds the types in L to the requirements (unless they are already provided/required).
// The caller must convert the types to the corresponding class type and expand any Provider<>s.
struct AddRequirements {
  template <typename Comp, typename ArgVector>
  struct apply {
    using type = ConsComp(FoldVector(ArgVector, AddToSet, typename Comp::RsSuperset),
                          typename Comp::Ps,
#ifndef FRUIT_NO_LOOP_CHECK
                          typename Comp::Deps,
#endif
                          typename Comp::InterfaceBindings,
                          typename Comp::DeferredBindingFunctors);
  };
};

// Similar to AddProvidedType, but doesn't report an error if a Bind<C, CImpl> was present.
struct AddProvidedTypeIgnoringInterfaceBindings {
  template <typename Comp, typename C, typename ArgV>
  struct apply {
    using Comp1 = ConsComp(FoldVector(ArgV, AddToSet, typename Comp::RsSuperset),
                           AddToSetUnchecked(typename Comp::Ps, C),
#ifndef FRUIT_NO_LOOP_CHECK
                           PushFront(typename Comp::Deps, Pair<C, ArgV>),
#endif
                           typename Comp::InterfaceBindings,
                           typename Comp::DeferredBindingFunctors);
    using type = If(IsInSet(C, typename Comp::Ps),
                    ConstructError(TypeAlreadyBoundErrorTag, C),
                 Comp1);
  };
};

// Adds C to the provides and removes it from the requirements (if it was there at all).
// Also checks that it wasn't already provided.
// Moreover, adds the requirements of C to the requirements, unless they were already provided/required.
// The caller must convert the types to the corresponding class type and expand any Provider<>s.
struct AddProvidedType {
  template <typename Comp, typename C, typename ArgV>
  struct apply {
    using type = If(Not(IsNone(FindInMap(typename Comp::InterfaceBindings, C))),
                    ConstructError(TypeAlreadyBoundErrorTag, C),
                 AddProvidedTypeIgnoringInterfaceBindings(Comp, C, ArgV));
  };
};

struct AddDeferredBinding {
  template <typename Comp, typename DeferredBinding>
  struct apply {
    using new_DeferredBindingFunctors = Cons<DeferredBinding,
                                             typename Comp::DeferredBindingFunctors>;
    using type = ConsComp(typename Comp::RsSuperset,
                          typename Comp::Ps,
#ifndef FRUIT_NO_LOOP_CHECK
                          typename Comp::Deps,
#endif
                          typename Comp::InterfaceBindings,
                          new_DeferredBindingFunctors);
  };
};

struct CheckNoLoopInDeps {
  template <typename Comp>
  struct apply {
    using Loop = ProofForestFindLoop(typename Comp::Deps);
    using type = If(IsNone(Loop),
                    Bool<true>,
                 ConstructErrorWithArgVector(SelfLoopErrorTag, Loop));
  };
};

#if defined(FRUIT_EXTRA_DEBUG) || defined(FRUIT_IN_META_TEST)
struct CheckComponentEntails {
  template <typename Comp, typename EntailedComp>
  struct apply {
    using         CompRs = SetDifference(typename         Comp::RsSuperset, typename         Comp::Ps);
    using EntailedCompRs = SetDifference(typename EntailedComp::RsSuperset, typename EntailedComp::Ps);
    using type = If(Not(IsContained(typename EntailedComp::Ps, typename Comp::Ps)),
                    ConstructErrorWithArgVector(ComponentDoesNotEntailDueToProvidesErrorTag,
                                                SetToVector(SetDifference(typename EntailedComp::Ps,
                                                                          typename Comp::Ps))),
                 If(Not(IsVectorContained(typename EntailedComp::InterfaceBindings, 
                                    typename Comp::InterfaceBindings)),
                    ConstructErrorWithArgVector(ComponentDoesNotEntailDueToInterfaceBindingsErrorTag,
                                                SetToVector(SetDifference(typename EntailedComp::InterfaceBindings, 
                                                                          typename Comp::InterfaceBindings))),
                 If(Not(IsContained(CompRs, EntailedCompRs)),
                    ConstructErrorWithArgVector(ComponentDoesNotEntailDueToRequirementsErrorTag,
                                                SetToVector(SetDifference(CompRs, EntailedCompRs))),
                 Bool<true>)));
    static_assert(true || sizeof(typename CheckIfError<Eval<type>>::type), "");
  };
};
#endif // defined(FRUIT_EXTRA_DEBUG) || defined(FRUIT_IN_META_TEST)

// This calls ConstructError(NoBindingFoundErrorTag, ...) or
// ConstructError(NoBindingFoundForAbstractClassErrorTag, ...) as appropriate.
// Call this when we're unable to auto-inject a type AnnotatedC and we're giving up.
struct ConstructNoBindingFoundError {
  template <typename AnnotatedC>
  struct apply {
    using type = If(IsAbstract(RemoveAnnotations(AnnotatedC)),
                    ConstructError(NoBindingFoundForAbstractClassErrorTag, AnnotatedC, RemoveAnnotations(AnnotatedC)),
                    ConstructError(NoBindingFoundErrorTag, AnnotatedC));
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_COMPONENT_H
