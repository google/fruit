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

#include "../../fruit_forward_decls.h"
#include "../fruit_internal_forward_decls.h"
#include "set.h"
#include "metaprogramming.h"
#include "errors.h"
#include "proof_trees.h"
#include "../injection_debug_errors.h"

#include <memory>

namespace fruit {
namespace impl {
namespace meta {

//********************************************************************************************************************************
// Part 1: Simple type functors (no ConsComp involved).
//********************************************************************************************************************************

template <typename AnnotatedI, typename AnnotatedC>
struct ConsInterfaceBinding {
  using Interface = AnnotatedI;
  using Impl = AnnotatedC;
};

// Given a type T, returns the class that should be injected to ensure that T is provided at runtime (if any).
struct GetClassForType {
  // General case, if none of the following apply.
  // When adding a specialization here, make sure that the ComponentStorage
  // can actually get<> the specified type when the class was registered.
  template <typename T>
  struct apply {using type = T;};

  template <typename T>
  struct apply<const T> {using type = T;};

  template <typename T>
  struct apply<T*> {using type = T;};

  template <typename T>
  struct apply<T&> {using type = T;};

  template <typename T>
  struct apply<const T*> {using type = T;};

  template <typename T>
  struct apply<const T&> {using type = T;};

  template <typename T>
  struct apply<std::shared_ptr<T>> {using type = T;};
  
  template <typename T>
  struct apply<Assisted<T>> {using type = None;};
  
  template <typename T>
  struct apply<Provider<T>> {using type = T;};
  
  template <typename Annotation, typename T>
  struct apply<fruit::Annotated<Annotation, T>> {using type = T;};
};

struct GetClassForTypeVector {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = Vector<Apply<GetClassForType, Ts>...>;
  };
};

// Given a type T, returns the type in the injection graph that corresponds to T.
struct NormalizeType {
  // General case, if none of the following apply.
  // When adding a specialization here, make sure that the ComponentStorage
  // can actually get<> the specified type when the class was registered.
  template <typename T>
  struct apply {using type = T;};

  template <typename T>
  struct apply<const T> {using type = T;};

  template <typename T>
  struct apply<T*> {using type = T;};

  template <typename T>
  struct apply<T&> {using type = T;};

  template <typename T>
  struct apply<const T*> {using type = T;};

  template <typename T>
  struct apply<const T&> {using type = T;};

  template <typename T>
  struct apply<std::shared_ptr<T>> {using type = T;};
  
  template <typename T>
  struct apply<Assisted<T>> {using type = None;};
  
  template <typename T>
  struct apply<Provider<T>> {using type = T;};
  
  template <typename Annotation, typename T>
  struct apply<fruit::Annotated<Annotation, T>> {using type = fruit::Annotated<Annotation, Apply<NormalizeType, T>>;};
};

struct NormalizeTypeVector {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = Vector<Apply<NormalizeType, Ts>...>;
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
  struct apply<fruit::Annotated<Annotation, T>, U> {
    using type = fruit::Annotated<Annotation, U>;
  };
};

struct IsValidSignature {
  template <typename Signature>
  struct apply {
    using type = Bool<false>;
  };

  template <typename T, typename... Args>
  struct apply<T(Args...)> {
    using type = Bool<true>;
  };
};

// Removes the Annotation (if any) wrapping a type T.
struct RemoveAnnotations {
  template <typename T>
  struct apply {
    using type = T;
  };
  
  template <typename Annotation, typename T>
  struct apply<fruit::Annotated<Annotation, T>> {
    using type = T;
  };
};

// Removes the Annotation(s) (if any) wrapping the types in AnnotatedSignature.
struct RemoveAnnotationsFromSignature {
  template <typename AnnotatedSignature>
  struct apply;
  
  template <typename AnnotatedT, typename... AnnotatedArgs>
  struct apply<AnnotatedT(AnnotatedArgs...)> {
    using type = Apply<RemoveAnnotations, AnnotatedT>(Apply<RemoveAnnotations, AnnotatedArgs>...);
  };
};

// Removes the Annotation(s) (if any) wrapping the types in the Vector V.
struct RemoveAnnotationsFromVector {
  template <typename V>
  struct apply;
  
  template <typename... AnnotatedTypes>
  struct apply<meta::Vector<AnnotatedTypes...>> {
    using type = meta::Vector<Apply<RemoveAnnotations, AnnotatedTypes>...>;
  };
};

// Maps T->T* in a possibly-annotated type.
struct AddPointerInAnnotatedType {
  template <typename T>
  struct apply {
    using type = T*;
  };
  
  template <typename Annotation, typename T>
  struct apply<fruit::Annotated<Annotation, T>> {
    using type = fruit::Annotated<Annotation, T*>;
  };
};

struct RemoveNonAssistedHelper {
  // No args.
  template <typename... Ts>
  struct apply {
    using type = Vector<>;
  };

  // Non-assisted case
  template <typename T, typename... Ts>
  struct apply<T, Ts...> {
    using type = Apply<RemoveNonAssistedHelper, Ts...>;
  };

  // Assisted case
  template <typename T, typename... Ts>
  struct apply<Assisted<T>, Ts...> {
    using type = Apply<PushFront,
                       Apply<RemoveNonAssistedHelper, Ts...>,
                       T>;
  };
};

// TODO: This also does UnlabelAssisted<>. Consider renaming and/or removing that logic (and
// letting callers do the unlabeling when desired).
struct RemoveNonAssisted {
  template <typename V>
  struct apply {
    using type = ApplyWithVector<RemoveNonAssistedHelper, V>;
  };
};

struct RemoveAssistedHelper {
  // Empty vector.
  template <typename... Ts>
  struct apply {
    using type = Vector<>;
  };

  // Non-assisted case
  template <typename T, typename... Ts>
  struct apply<T, Ts...> {
    using type = Apply<PushFront,
                       Apply<RemoveAssistedHelper, Ts...>,
                       T>;
  };

  // Assisted case
  template <typename T, typename... Ts>
  struct apply<Assisted<T>, Ts...> : public apply<Ts...> {
  };
};

struct RemoveAssisted {
  template <typename V>
  struct apply {
    using type = ApplyWithVector<RemoveAssistedHelper, V>;
  };
};

struct UnlabelAssistedSingleType {
  template <typename T>
  struct apply {
    using type = T;
  };

  template <typename T>
  struct apply<Assisted<T>> {
    using type = T;
  };
};

struct UnlabelAssisted {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = Vector<Apply<UnlabelAssistedSingleType, Ts>...>;
  };
};

struct RequiredLambdaArgsForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = Apply<RemoveAnnotationsFromVector, Apply<UnlabelAssisted, Apply<SignatureArgs, AnnotatedSignature>>>;
  };
};

struct RequiredLambdaSignatureForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = Apply<ConstructSignature, 
                       Apply<RemoveAnnotations, Apply<SignatureType, AnnotatedSignature>>,
                       Apply<RequiredLambdaArgsForAssistedFactory, AnnotatedSignature>>;
  };
};

struct InjectedFunctionArgsForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = Apply<RemoveNonAssisted, Apply<SignatureArgs, AnnotatedSignature>>;
  };
};

struct InjectedSignatureForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = Apply<ConstructSignature, 
                       Apply<RemoveAnnotations, Apply<SignatureType, AnnotatedSignature>>,
                       Apply<InjectedFunctionArgsForAssistedFactory, AnnotatedSignature>>;
  };
};

struct NumAssistedBeforeHelper {
  template <typename Index, typename... Ts>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<Int<0>, T, Ts...> {
    using type = Int<0>;
  };

  // This is needed because the previous is not more specialized than the specialization with assisted T.
  template <typename T, typename... Ts>
  struct apply<Int<0>, Assisted<T>, Ts...> {
    using type = Int<0>;
  };

  // Non-assisted T, index!=0.
  template <int index, typename T, typename... Ts>
  struct apply<Int<index>, T, Ts...> {
    using type = Apply<NumAssistedBeforeHelper, Int<index - 1>, Ts...>;
  };

  // Assisted T, index!=0.
  template <int index, typename T, typename... Ts>
  struct apply<Int<index>, Assisted<T>, Ts...> {
    using type = Int<1 + Apply<NumAssistedBeforeHelper, Int<index - 1>, Ts...>::value>;
  };
};

struct NumAssistedBefore {
  template <typename Index, typename V>
  struct apply;
  
  template <typename Index, typename... Ts>
  struct apply<Index, Vector<Ts...>> {
    using type = Apply<NumAssistedBeforeHelper, Index, Ts...>;
  };
};

// Checks whether C is auto-injectable thanks to an Inject typedef.
struct HasInjectAnnotation {
  template <typename C>
  struct apply {
    typedef char yes[1];
    typedef char no[2];

    template <typename C1>
    static yes& test(typename C1::Inject*);

    template <typename>
    static no& test(...);
    
    using type = Bool<sizeof(test<C>(0)) == sizeof(yes)>;
  };
};

struct GetInjectAnnotation {
  template <typename C>
  struct apply {
    using S = typename C::Inject;
    // if !validSignature(S)
    //     return Error(InjectTypedefNotASignatureErrorTag, C, S)
    // if !isSame(C, signatureType(S))
    //     return Error(InjectTypedefForWrongClassErrorTag, C, signatureType(S))
    // if !isConstructibleWithVector(C, unlablelAssisted(signatureArgs(S)))
    //     return Error(NoConstructorMatchingInjectSignatureErrorTag, C, S)
    // return S
    using LazySignatureType = Apply<LazyFunctor<SignatureType>, Lazy<S>>;
    using LazySignatureArgs = Apply<LazyFunctor<SignatureArgs>, Lazy<S>>;
    using type = Eval<Conditional<Lazy<Apply<Not, Apply<IsValidSignature, S>>>,
                                  Apply<LazyFunctor<ConstructError>, Lazy<InjectTypedefNotASignatureErrorTag>, Lazy<C>, Lazy<S>>,
                                  Conditional<Apply<LazyFunctor<Not>, Apply<LazyFunctor<IsSame>, Lazy<C>, LazySignatureType>>,
                                              Apply<LazyFunctor<ConstructError>, Lazy<InjectTypedefForWrongClassErrorTag>, Lazy<C>, LazySignatureType>,
                                              Conditional<Apply<LazyFunctor<Not>, Apply<LazyFunctor<IsConstructibleWithVector>, Lazy<C>, Apply<LazyFunctor<UnlabelAssisted>, LazySignatureArgs>>>,
                                                          Apply<LazyFunctor<ConstructError>, Lazy<NoConstructorMatchingInjectSignatureErrorTag>, Lazy<C>, Lazy<S>>,
                                                          Lazy<S>
                                                          >
                                              >
                                  >>;
  };
};

// Takes a vector of args, possibly including Provider<>s, and returns the set of required types.
struct ExpandProvidersInParamsHelper {
  // Empty vector.
  template <typename... AnnotatedTs>
  struct apply {
    using type = Vector<>;
  };

  // Non-empty vector, AnnotatedT is not of the form Provider<C>
  template <typename AnnotatedT, typename... OtherAnnotatedTs>
  struct apply<AnnotatedT, OtherAnnotatedTs...> {
    using type = Apply<AddToSet, AnnotatedT, Apply<ExpandProvidersInParamsHelper, OtherAnnotatedTs...>>;
  };

  // Non-empty vector, type of the form Provider<C>
  template <typename C, typename... OtherAnnotatedTs>
  struct apply<fruit::Provider<C>, OtherAnnotatedTs...> {
    using type = Apply<AddToSet, C, Apply<ExpandProvidersInParamsHelper, OtherAnnotatedTs...>>;
  };

  // Non-empty vector, type of the form Annotated<Annotation, Provider<C>>
  template <typename Annotation, typename C, typename... OtherAnnotatedTs>
  struct apply<fruit::Annotated<Annotation, fruit::Provider<C>>, OtherAnnotatedTs...> {
    using type = Apply<AddToSet, fruit::Annotated<Annotation, C>, Apply<ExpandProvidersInParamsHelper, OtherAnnotatedTs...>>;
  };
};

struct ExpandProvidersInParams {
  template <typename V>
  struct apply;

  template <typename... AnnotatedTs>
  struct apply<Vector<AnnotatedTs...>> {
    using type = Apply<ExpandProvidersInParamsHelper, AnnotatedTs...>;
  };
};

struct HasInterfaceBinding {
  template <typename AnnotatedI, typename InterfaceBindings>
  struct apply;

  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply<AnnotatedI, Vector<InterfaceBindings...>> {
    using type = Bool<StaticOr<std::is_same<AnnotatedI, typename InterfaceBindings::Interface>::value...>::value>;
  };
};

struct GetInterfaceBindingHelper {
  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply;

  template <typename AnnotatedI, typename AnnotatedC, typename... InterfaceBindings>
  struct apply<AnnotatedI, ConsInterfaceBinding<AnnotatedI, AnnotatedC>, InterfaceBindings...> {
    using type = AnnotatedC;
  };

  template <typename AnnotatedI, typename OtherInterfaceBinding, typename... InterfaceBindings>
  struct apply<AnnotatedI, OtherInterfaceBinding, InterfaceBindings...> {
    using type = Apply<GetInterfaceBindingHelper, AnnotatedI, InterfaceBindings...>;
  };
};

struct GetInterfaceBinding {
  template <typename AnnotatedI, typename InterfaceBindings>
  struct apply;

  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply<AnnotatedI, Vector<InterfaceBindings...>> {
    using type = Apply<GetInterfaceBindingHelper, AnnotatedI, InterfaceBindings...>;
  };
};

// True if C has at least 1 reverse binding.
struct HasBindingToInterface {
  template <typename AnnotatedC, typename... InterfaceBindings>
  struct apply {
    using type = Bool<StaticOr<std::is_same<AnnotatedC, typename InterfaceBindings::Impl>::value...>::value>;
  };
};

struct GetBindingToInterfaceHelper {
  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply {
    using type = None;
  };

  template <typename AnnotatedC, typename AnnotatedI, typename... InterfaceBindings>
  struct apply<AnnotatedC, ConsInterfaceBinding<AnnotatedI, AnnotatedC>, InterfaceBindings...> {
    using type = Eval<std::conditional<Apply<HasBindingToInterface, AnnotatedC, InterfaceBindings...>::value,
                                       None,
                                       AnnotatedI>>;
  };

  template <typename AnnotatedI, typename OtherInterfaceBindings, typename... InterfaceBindings>
  struct apply<AnnotatedI, OtherInterfaceBindings, InterfaceBindings...> {
    using type = Apply<GetBindingToInterfaceHelper, AnnotatedI, InterfaceBindings...>;
  };
};

// If there's a single interface I bound to C, returns I.
// If there is no interface bound to C, or if there are multiple, returns None.
struct GetBindingToInterface {
  template <typename AnnotatedI, typename InterfaceBindings>
  struct apply;

  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply<AnnotatedI, Vector<InterfaceBindings...>> {
    using type = Apply<GetBindingToInterfaceHelper, AnnotatedI, InterfaceBindings...>;
  };
};

//********************************************************************************************************************************
// Part 2: Type functors involving at least one ConsComp.
//********************************************************************************************************************************

template <typename RsParam, typename PsParam, typename DepsParam, typename InterfaceBindingsParam, 
          typename DeferredBindingFunctorsParam>
struct ConsComp {
  using Rs = RsParam;
  using Ps = PsParam;
  using Deps = DepsParam;
  using InterfaceBindings = InterfaceBindingsParam;
  using DeferredBindingFunctors = DeferredBindingFunctorsParam;
  
  // Invariants:
  // * all types appearing as arguments of Deps are in Rs
  // * all types in Ps are at the head of one (and only one) Dep.
  //   (note that the types in Rs can appear in deps any number of times, 0 is also ok)
  // * Deps is of the form Vector<Dep...> with each Dep of the form T(Args...) and where Vector<Args...> is a set (no repetitions).
  // * Bindings is a proof tree forest, with injected classes as formulas.
  // * Each element X of DeferredBindingFunctors has:
  //   - a default-constructible X::apply<Comp> type
  //   - a void X::apply<Comp>::operator(ComponentStorage&)
  //   - a X::apply<Comp>::Result type
  
#ifndef FRUIT_NO_LOOP_CHECK
  FruitStaticAssert(true || sizeof(Apply<CheckDepsNormalized, Apply<AddProofTreeVectorToForest, Deps, EmptyProofForest, Vector<>>, Deps>), "");
#endif // !FRUIT_NO_LOOP_CHECK
};

struct GetComponentDeps {
  template <typename Comp>
  struct apply {
    using type = typename Comp::Deps;
  };
};

// Checks that Types... are normalized types. If not it returns an appropriate error.
// If they are all normalized types this returns Result.
struct CheckNormalizedTypes {
  template <typename Result, typename... Types>
  struct apply;
  
  template <typename Result>
  struct apply<Result> {
    using type = Result;
  };
  
  template <typename Result, typename Type, typename... Types>
  struct apply<Result, Type, Types...> {
    using NormalizedType = Apply<NormalizeType, Type>;
    using type = Eval<Conditional<Lazy<Bool<!std::is_same<NormalizedType, Type>::value>>,
                                  Lazy<Error<NonClassTypeErrorTag, Apply<RemoveAnnotations, Type>, Apply<RemoveAnnotations, NormalizedType>>>,
                                  Apply<LazyFunctor<CheckNormalizedTypes>, Lazy<Result>, Lazy<Types>...>
                                  >>;
  };
};

// Checks that there are no repetitions in Types. If there are, it returns an appropriate error.
// If there are no repetitions it returns Result.
struct CheckNoRepeatedTypes {
  template <typename Result, typename... Types>
  struct apply {
    using type = Eval<Conditional<Lazy<Bool<Apply<VectorSize, Apply<VectorToSet, Vector<Types...>>>::value != sizeof...(Types)>>,
                                  Apply<LazyFunctor<ConstructError>, Lazy<RepeatedTypesErrorTag>, Lazy<Types>...>,
                                  Lazy<Result>
                                  >>;
  };
};

struct ConstructComponentImpl {
  // Non-specialized case: no requirements.
  template <typename... Ps>
  struct apply {
    using Comp = ConsComp<Vector<>,
                          Vector<Ps...>,
                          Apply<ConstructProofForest, Vector<>, Ps...>,
                          Vector<>,
                          Vector<>>;
    using type = Apply<ApplyAndPostponeFirstArgument<CheckNoRepeatedTypes, Ps...>,
                       Apply<ApplyAndPostponeFirstArgument<CheckNormalizedTypes, Ps...>,
                             Comp>>;
#ifndef FRUIT_NO_LOOP_CHECK
    FruitStaticAssert(true || sizeof(Eval<Conditional<Lazy<Apply<IsError, type>>,
                                                      Lazy<int>, // No check, we'll report a user error soon.
                                                      Apply<LazyFunctor<CheckDepsNormalized>,
                                                            Apply<LazyFunctor<AddProofTreeVectorToForest>,
                                                                  Apply<LazyFunctor<GetComponentDeps>, Lazy<type>>,
                                                                  Lazy<EmptyProofForest>,
                                                                  Lazy<Vector<>>
                                                                  >,
                                                            Apply<LazyFunctor<GetComponentDeps>, Lazy<type>>
                                                            >
                                                      >>), "");
#endif // !FRUIT_NO_LOOP_CHECK
  };

  // With requirements.
  template <typename... Rs, typename... Ps>
  struct apply<Required<Rs...>, Ps...> {
    using Comp = ConsComp<Vector<Rs...>,
                          Vector<Ps...>,
                          Apply<ConstructProofForest, Vector<Rs...>, Ps...>,
                          Vector<>,
                          Vector<>>;
    using type = Apply<ApplyAndPostponeFirstArgument<CheckNoRepeatedTypes, Rs..., Ps...>,
                       Apply<ApplyAndPostponeFirstArgument<CheckNormalizedTypes, Rs...>,
                             Apply<ApplyAndPostponeFirstArgument<CheckNormalizedTypes, Ps...>,
                                   Comp>>>;
#ifndef FRUIT_NO_LOOP_CHECK
    FruitStaticAssert(true || sizeof(Eval<Conditional<Lazy<Apply<IsError, type>>,
                                                      Lazy<int>, // No check, we'll report a user error soon.
                                                      Apply<LazyFunctor<CheckDepsNormalized>,
                                                            Apply<LazyFunctor<AddProofTreeVectorToForest>,
                                                                  Apply<LazyFunctor<GetComponentDeps>, Lazy<type>>,
                                                                  Lazy<EmptyProofForest>,
                                                                  Lazy<Vector<>>
                                                                  >,
                                                            Apply<LazyFunctor<GetComponentDeps>, Lazy<type>>
                                                            >
                                                      >>), "");
#endif // !FRUIT_NO_LOOP_CHECK
  };
};

// Adds the types in L to the requirements (unless they are already provided/required).
// The caller must convert the types to the corresponding class type and expand any Provider<>s.
struct AddRequirements {
  template <typename Comp, typename ArgSet>
  struct apply {
    using type = ConsComp<Apply<SetUnion,
                                Apply<SetDifference, ArgSet, typename Comp::Ps>,
                                typename Comp::Rs>,
                          typename Comp::Ps,
                          typename Comp::Deps,
                          typename Comp::InterfaceBindings,
                          typename Comp::DeferredBindingFunctors>;
  };
};

// Adds C to the provides and removes it from the requirements (if it was there at all).
// Also checks that it wasn't already provided.
// Moreover, adds the requirements of C to the requirements, unless they were already provided/required.
// The caller must convert the types to the corresponding class type and expand any Provider<>s.
struct AddProvidedType {
  template <typename Comp, typename C, typename ArgSet>
  struct apply {
    using newDeps = Apply<AddProofTreeToForest,
                          ConsProofTree<ArgSet, C>,
                          typename Comp::Deps,
                          typename Comp::Ps>;
    using Comp1 = ConsComp<Apply<SetUnion,
                                 Apply<SetDifference, ArgSet, typename Comp::Ps>,
                                 Apply<RemoveFromVector, C, typename Comp::Rs>>,
                           Apply<PushFront, typename Comp::Ps, C>,
                           newDeps,
                           typename Comp::InterfaceBindings,
                           typename Comp::DeferredBindingFunctors>;
    using type = Eval<Conditional<Lazy<Bool<std::is_same<newDeps, None>::value>>,
                                  Apply<LazyFunctor<ConstructErrorWithArgVector>, Lazy<SelfLoopErrorTag>, Lazy<ArgSet>, Lazy<C>>,
                                  Conditional<Apply<LazyFunctor<IsInVector>, Lazy<C>, Lazy<typename Comp::Ps>>,
                                              Apply<LazyFunctor<ConstructError>, Lazy<TypeAlreadyBoundErrorTag>, Lazy<C>>,
                                              Lazy<Comp1>
                                              >
                                  >>;
  };
};

struct AddDeferredBinding {
  template <typename Comp, typename DeferredBinding>
  struct apply {
    using new_DeferredBindingFunctors = Apply<PushBack,
        typename Comp::DeferredBindingFunctors,
        DeferredBinding>;
    using type = ConsComp<typename Comp::Rs,
                          typename Comp::Ps,
                          typename Comp::Deps,
                          typename Comp::InterfaceBindings,
                          new_DeferredBindingFunctors>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_COMPONENT_H
