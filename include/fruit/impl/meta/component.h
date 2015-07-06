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
#include "wrappers.h"
#include "signatures.h"
#include "../injection_debug_errors.h"

#include <memory>

namespace fruit {
namespace impl {
namespace meta {

//********************************************************************************************************************************
// Part 1: Simple type functors (no ConsComp involved).
//********************************************************************************************************************************

template <typename AnnotatedI, typename AnnotatedC>
struct InterfaceBinding {
  using Interface = AnnotatedI;
  using Impl = AnnotatedC;
};

// Using ConsInterfaceBinding(MetaExpr...) instead of InterfaceBinding<MetaExpr...> in a 
// meta-expression allows the types to be evaluated. Avoid using InterfaceBinding<...> directly in a
// meta-expression, unless you're sure that the arguments have already been evaluated (e.g. if
// I, C are arguments of a metafunction, InterfaceBinding<I, C> is ok but 
// InterfaceBinding<MyFunction(I), C> is wrong.
struct ConsInterfaceBinding {
  template <typename AnnotatedI, typename AnnotatedC>
  struct apply {
    using type = InterfaceBinding<AnnotatedI, AnnotatedC>;
  };
};


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
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = ConsVector(Id<GetClassForType(Ts)>
                            ...);
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
  struct apply<Type<fruit::Annotated<Annotation, T>>> {using type = Type<fruit::Annotated<Annotation, EvalType<NormalizeType(Type<T>)>>>;};
};

struct NormalizeTypeVector {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = ConsVector(Id<NormalizeType(Ts)>
                            ...);
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
  struct apply;
  
  template <typename AnnotatedT, typename... AnnotatedArgs>
  struct apply<Type<AnnotatedT(AnnotatedArgs...)>> {
    using type = ConsSignature(RemoveAnnotations(Type<AnnotatedT>),
                               Id<RemoveAnnotations(Type<AnnotatedArgs>)>...);
  };
};

// Removes the Annotation(s) (if any) wrapping the types in the Vector V.
struct RemoveAnnotationsFromVector {
  template <typename V>
  struct apply;
  
  template <typename... AnnotatedTypes>
  struct apply<Vector<AnnotatedTypes...>> {
    using type = ConsVector(Id<RemoveAnnotations(AnnotatedTypes)>...);
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

struct RemoveNonAssistedHelper {
  // No args.
  template <typename... Ts>
  struct apply {
    using type = Vector<>;
  };

  // Non-assisted case
  template <typename T, typename... Ts>
  struct apply<Type<T>, Ts...> {
    using type = RemoveNonAssistedHelper(Ts...);
  };

  // Assisted case
  template <typename T, typename... Ts>
  struct apply<Type<Assisted<T>>, Ts...> {
    using type = PushFront(RemoveNonAssistedHelper(Ts...),
                           Type<T>);
  };
};

// TODO: This also does UnlabelAssisted<>. Consider renaming and/or removing that logic (and
// letting callers do the unlabeling when desired).
struct RemoveNonAssisted {
  template <typename V>
  struct apply {
    using type = CallWithVector(RemoveNonAssistedHelper, V);
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
  struct apply<Type<T>, Ts...> {
    using type = PushFront(RemoveAssistedHelper(Ts...),
                           Type<T>);
  };

  // Assisted case
  template <typename T, typename... Ts>
  struct apply<Type<Assisted<T>>, Ts...> {
    using type = RemoveAssistedHelper(Ts...);
  };
};

struct RemoveAssisted {
  template <typename V>
  struct apply {
    using type = CallWithVector(RemoveAssistedHelper, V);
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
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = ConsVector(Id<UnlabelAssistedSingleType(Ts)>...);
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

struct NumAssistedBeforeHelper {
  template <typename Index, typename... Ts>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<Int<0>, Type<T>, Ts...> {
    using type = Int<0>;
  };

  // This is needed because the previous is not more specialized than the specialization with assisted T.
  template <typename T, typename... Ts>
  struct apply<Int<0>, Type<Assisted<T>>, Ts...> {
    using type = Int<0>;
  };

  // Non-assisted T, index!=0.
  template <int index, typename T, typename... Ts>
  struct apply<Int<index>, Type<T>, Ts...> {
    using type = NumAssistedBeforeHelper(Int<index - 1>, Ts...);
  };

  // Assisted T, index!=0.
  template <int index, typename T, typename... Ts>
  struct apply<Int<index>, Type<Assisted<T>>, Ts...> {
    using type = Plus(NumAssistedBeforeHelper(Int<index - 1>, Ts...),
                      Int<1>);
  };
};

struct NumAssistedBefore {
  template <typename Index, typename V>
  struct apply;
  
  template <typename Index, typename... Ts>
  struct apply<Index, Vector<Ts...>> {
    using type = NumAssistedBeforeHelper(Index, Ts...);
  };
};

// Checks whether C is auto-injectable thanks to an Inject typedef.
struct HasInjectAnnotation {
  template <typename C>
  struct apply;
  
  template <typename C>
  struct apply<Type<C>> {
    typedef char yes[1];
    typedef char no[2];

    template <typename C1>
    static yes& test(typename C1::Inject*);

    template <typename>
    static no& test(...);
    
    using type = Bool<sizeof(test<C>(0)) == sizeof(yes)>;
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
    using AnnotatedSResult = SignatureType(DecoratedS);
    using SResult = RemoveAnnotations(AnnotatedSResult);
    using SArgs = RemoveAnnotationsFromVector(UnlabelAssisted(SignatureArgs(DecoratedS)));
    using type = If(Not(IsValidSignature(DecoratedS)),
                    ConstructError(InjectTypedefNotASignatureErrorTag, C, DecoratedS),
                 If(Not(IsSame(C, SResult)),
                   ConstructError(InjectTypedefForWrongClassErrorTag, C, SResult),
                 If(Not(IsSame(AnnotatedC, AnnotatedSResult)),
                   ConstructError(InjectTypedefWithDifferentAnnotationErrorTag, AnnotatedC, AnnotatedSResult),
                 If(Not(IsConstructibleWithVector(C, SArgs)),
                    ConstructError(NoConstructorMatchingInjectSignatureErrorTag, C, ConsSignatureWithVector(SResult, SArgs)),
                 DecoratedS))));
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
  struct apply<Type<AnnotatedT>, OtherAnnotatedTs...> {
    using type = AddToSet(Type<AnnotatedT>, ExpandProvidersInParamsHelper(OtherAnnotatedTs...));
  };

  // Non-empty vector, type of the form Provider<C>
  template <typename C, typename... OtherAnnotatedTs>
  struct apply<Type<fruit::Provider<C>>, OtherAnnotatedTs...> {
    using type = AddToSet(Type<C>, ExpandProvidersInParamsHelper(OtherAnnotatedTs...));
  };

  // Non-empty vector, type of the form Annotated<Annotation, Provider<C>>
  template <typename Annotation, typename C, typename... OtherAnnotatedTs>
  struct apply<Type<fruit::Annotated<Annotation, fruit::Provider<C>>>, OtherAnnotatedTs...> {
    using type = AddToSet(Type<fruit::Annotated<Annotation, C>>, 
                          ExpandProvidersInParamsHelper(OtherAnnotatedTs...));
  };
};

struct ExpandProvidersInParams {
  template <typename V>
  struct apply;

  template <typename... AnnotatedTs>
  struct apply<Vector<AnnotatedTs...>> {
    using type = ExpandProvidersInParamsHelper(AnnotatedTs...);
  };
};

struct HasInterfaceBinding {
  template <typename AnnotatedI, typename InterfaceBindings>
  struct apply;

  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply<AnnotatedI, Vector<InterfaceBindings...>> {
    using type = Or(Id<IsSame(AnnotatedI, typename InterfaceBindings::Interface)>...);
  };
};

struct GetInterfaceBindingHelper {
  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply;

  template <typename AnnotatedI, typename AnnotatedC, typename... InterfaceBindings>
  struct apply<AnnotatedI, InterfaceBinding<AnnotatedI, AnnotatedC>, InterfaceBindings...> {
    using type = AnnotatedC;
  };

  template <typename AnnotatedI, typename OtherInterfaceBinding, typename... InterfaceBindings>
  struct apply<AnnotatedI, OtherInterfaceBinding, InterfaceBindings...> {
    using type = GetInterfaceBindingHelper(AnnotatedI, InterfaceBindings...);
  };
};

struct GetInterfaceBinding {
  template <typename AnnotatedI, typename InterfaceBindings>
  struct apply;

  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply<AnnotatedI, Vector<InterfaceBindings...>> {
    using type = GetInterfaceBindingHelper(AnnotatedI, InterfaceBindings...);
  };
};

// True if C has at least 1 reverse binding.
struct HasBindingToInterface {
  template <typename AnnotatedC, typename... InterfaceBindings>
  struct apply {
    using type = Or(Id<IsSame(AnnotatedC, typename InterfaceBindings::Impl)>...);
  };
};

struct GetBindingToInterfaceHelper {
  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply {
    using type = None;
  };

  template <typename AnnotatedC, typename AnnotatedI, typename... InterfaceBindings>
  struct apply<AnnotatedC, InterfaceBinding<AnnotatedI, AnnotatedC>, InterfaceBindings...> {
    // TODO: Consider turning this check into an assert and returning AnnotatedI unconditionally.
    using type = If(HasBindingToInterface(AnnotatedC, InterfaceBindings...),
                    None,
                    AnnotatedI);
  };

  template <typename AnnotatedI, typename OtherInterfaceBindings, typename... InterfaceBindings>
  struct apply<AnnotatedI, OtherInterfaceBindings, InterfaceBindings...> {
    using type = GetBindingToInterfaceHelper(AnnotatedI, InterfaceBindings...);
  };
};

// If there's a single interface I bound to C, returns I.
// If there is no interface bound to C, or if there are multiple, returns None.
struct GetBindingToInterface {
  template <typename AnnotatedI, typename InterfaceBindings>
  struct apply;

  template <typename AnnotatedI, typename... InterfaceBindings>
  struct apply<AnnotatedI, Vector<InterfaceBindings...>> {
    using type = GetBindingToInterfaceHelper(AnnotatedI, InterfaceBindings...);
  };
};

//********************************************************************************************************************************
// Part 2: Type functors involving at least one ConsComp.
//********************************************************************************************************************************

template <typename RsParam, typename PsParam, typename DepsParam, typename InterfaceBindingsParam, 
          typename DeferredBindingFunctorsParam>
struct Comp {
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
#ifdef FRUIT_EXTRA_DEBUG
  FruitDelegateCheck(fruit::impl::meta::CheckDepsNormalized(AddProofTreeVectorToForest(Deps, EmptyProofForest, Vector<>), Deps));
#endif // FRUIT_EXTRA_DEBUG
#endif // !FRUIT_NO_LOOP_CHECK
};

// Using ConsComp instead of Comp<...> in a meta-expression allows the types to be evaluated.
// See ConsVector for more details.
struct ConsComp {
  template <typename RsParam, typename PsParam, typename DepsParam, typename InterfaceBindingsParam, 
            typename DeferredBindingFunctorsParam>
  struct apply {
    using type = Comp<RsParam, PsParam, DepsParam, InterfaceBindingsParam, DeferredBindingFunctorsParam>;
  };
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
  template <typename... Types>
  struct apply {
    using type = None;
  };
  
  template <typename Type, typename... Types>
  struct apply<Type, Types...> {
    using NormalizedType = NormalizeType(Type);
    using type = If(Not(IsSame(NormalizedType, Type)),
                    ConstructError(NonClassTypeErrorTag, RemoveAnnotations(Type), RemoveAnnotations(NormalizedType)),
                    CheckNormalizedTypes(Types...));
  };
};

// Checks that Types... are not annotated types. If they have an annotation it returns an appropriate error.
// If none of them is annotated, this returns None.
struct CheckNotAnnotatedTypes {
  template <typename... Types>
  struct apply {
    using type = None;
  };
  
  template <typename Type, typename... Types>
  struct apply<Type, Types...> {
    using TypeWithoutAnnotations = RemoveAnnotations(Type);
    using type = If(Not(IsSame(TypeWithoutAnnotations, Type)),
                    ConstructError(AnnotatedTypeErrorTag, Type, TypeWithoutAnnotations),
                    CheckNotAnnotatedTypes(Types...));
  };
};

// Checks that there are no repetitions in Types. If there are, it returns an appropriate error.
// If there are no repetitions it returns None.
struct CheckNoRepeatedTypes {
  template <typename... Types>
  struct apply {
    using type = If(Not(IsSame(VectorSize(VectorToSet(Vector<Types...>)), Int<sizeof...(Types)>)),
                    ConstructError(RepeatedTypesErrorTag, Types...),
                    None);
  };
};

struct ConstructComponentImpl {
  // Non-specialized case: no requirements.
  template <typename... Ps>
  struct apply {
    using Comp = ConsComp(Vector<>,
                          Vector<Ps...>,
                          ConstructProofForest(Vector<>, Ps...),
                          Vector<>,
                          Vector<>);
    using E1 = CheckNoRepeatedTypes(Ps...);
    using E2 = CheckNormalizedTypes(Ps...);
    using type = If(IsError(E1),
                    E1,
                 If(IsError(E2),
                    E2,
                 Comp));
#ifndef FRUIT_NO_LOOP_CHECK
#ifdef FRUIT_EXTRA_DEBUG
    FruitDelegateCheck(If(IsError(type),
                          None, // No check, we'll report a user error soon.
                          CheckDepsNormalized(
                            AddProofTreeVectorToForest(GetComponentDeps(type),
                                                       EmptyProofForest,
                                                       Vector<>),
                            GetComponentDeps(type))));
#endif // FRUIT_EXTRA_DEBUG
#endif // !FRUIT_NO_LOOP_CHECK
  };

  // With requirements.
  template <typename... Rs, typename... Ps>
  struct apply<Type<Required<Rs...>>, Ps...> {
    using Comp = ConsComp(Vector<Type<Rs>...>,
                          Vector<Ps...>,
                          ConstructProofForest(Vector<Type<Rs>...>, Ps...),
                          Vector<>,
                          Vector<>);
    using E1 = CheckNoRepeatedTypes(Type<Rs>..., Ps...);
    using E2 = CheckNormalizedTypes(Type<Rs>...);
    using E3 = CheckNormalizedTypes(Ps...);
    using type = If(IsError(E1),
                    E1,
                 If(IsError(E2),
                    E2,
                 If(IsError(E3),
                    E3,
                 Comp)));

#ifndef FRUIT_NO_LOOP_CHECK
#ifdef FRUIT_EXTRA_DEBUG
    FruitDelegateCheck(If(IsError(type),
                          None, // No check, we'll report a user error soon.
                          CheckDepsNormalized(
                            AddProofTreeVectorToForest(
                              GetComponentDeps(type),
                              EmptyProofForest,
                              Vector<>),
                            GetComponentDeps(type))));
#endif // FRUIT_EXTRA_DEBUG
#endif // !FRUIT_NO_LOOP_CHECK
  };
};

// Adds the types in L to the requirements (unless they are already provided/required).
// The caller must convert the types to the corresponding class type and expand any Provider<>s.
struct AddRequirements {
  template <typename Comp, typename ArgSet>
  struct apply {
    using type = ConsComp(SetUnion(SetDifference(ArgSet, typename Comp::Ps),
                                   typename Comp::Rs),
                          typename Comp::Ps,
                          typename Comp::Deps,
                          typename Comp::InterfaceBindings,
                          typename Comp::DeferredBindingFunctors);
  };
};

// Adds C to the provides and removes it from the requirements (if it was there at all).
// Also checks that it wasn't already provided.
// Moreover, adds the requirements of C to the requirements, unless they were already provided/required.
// The caller must convert the types to the corresponding class type and expand any Provider<>s.
struct AddProvidedType {
  template <typename Comp, typename C, typename ArgSet>
  struct apply {
    using newDeps = AddProofTreeToForest(ConsProofTree(ArgSet, C),
                                         typename Comp::Deps,
                                         typename Comp::Ps);
    using Comp1 = ConsComp(SetUnion(SetDifference(ArgSet, typename Comp::Ps),
                                    RemoveFromVector(C, typename Comp::Rs)),
                           PushFront(typename Comp::Ps, C),
                           newDeps,
                           typename Comp::InterfaceBindings,
                           typename Comp::DeferredBindingFunctors);
    using type = If(IsSame(newDeps, None),
                    ConstructErrorWithArgVector(SelfLoopErrorTag, ArgSet, C),
                 If(IsInVector(C, typename Comp::Ps),
                    ConstructError(TypeAlreadyBoundErrorTag, C),
                 Comp1));
  };
};

struct AddDeferredBinding {
  template <typename Comp, typename DeferredBinding>
  struct apply {
    using new_DeferredBindingFunctors = PushBack(typename Comp::DeferredBindingFunctors,
                                                 DeferredBinding);
    using type = ConsComp(typename Comp::Rs,
                          typename Comp::Ps,
                          typename Comp::Deps,
                          typename Comp::InterfaceBindings,
                          new_DeferredBindingFunctors);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_COMPONENT_H
