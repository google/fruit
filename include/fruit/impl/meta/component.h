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
#include "proof_trees.h"
#include "../injection_debug_errors.h"

#include <memory>

namespace fruit {
namespace impl {
namespace meta {

//********************************************************************************************************************************
// Part 1: Simple type functors (no ConsComp involved).
//********************************************************************************************************************************

template <typename I, typename C>
struct ConsInterfaceBinding {
  using Interface = I;
  using Impl = C;
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
};

struct GetClassForTypeVector {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = Vector<Apply<GetClassForType, Ts>...>;
  };
};

struct IsValidSignature {
  template <typename Signature>
  struct apply : std::false_type {};

  template <typename T, typename... Args>
  struct apply<T(Args...)> : std::true_type {};
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
    using type = Apply<AddToVector,
                       T,
                       Apply<RemoveNonAssistedHelper, Ts...>>;
  };
};

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
    using type = Apply<AddToVector,
                       T,
                       Apply<RemoveAssistedHelper, Ts...>>;
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

struct RequiredArgsForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = Apply<UnlabelAssisted, Apply<SignatureArgs, AnnotatedSignature>>;
  };
};

struct RequiredSignatureForAssistedFactory {
  template <typename AnnotatedSignature>
  struct apply {
    using type = Apply<ConstructSignature, 
                       Apply<SignatureType, AnnotatedSignature>,
                       Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>>;
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
                       Apply<SignatureType, AnnotatedSignature>,
                       Apply<InjectedFunctionArgsForAssistedFactory, AnnotatedSignature>>;
  };
};

struct NumAssistedBeforeHelper {
  template <int index, typename... Ts>
  class apply;

  template <typename T, typename... Ts>
  class apply<0, T, Ts...> : public std::integral_constant<int, 0> {};

  // This is needed because the previous is not more specialized than the specialization with assisted T.
  template <typename T, typename... Ts>
  class apply<0, Assisted<T>, Ts...> : public std::integral_constant<int, 0> {};

  // Non-assisted T, index!=0.
  template <int index, typename T, typename... Ts>
  class apply<index, T, Ts...> : public apply<index-1, Ts...> {};

  // Assisted T, index!=0.
  template <int index, typename T, typename... Ts>
  class apply<index, Assisted<T>, Ts...> : public std::integral_constant<int, 1 + apply<index-1, Ts...>::value> {};
};

template <int index, typename V>
struct NumAssistedBefore;

template <int index, typename... Ts>
struct NumAssistedBefore<index, Vector<Ts...>> : public NumAssistedBeforeHelper::template apply<index, Ts...> {
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
    
    static const bool value = sizeof(test<C>(0)) == sizeof(yes);
  };
};

struct GetInjectAnnotation {
  template <typename C>
  struct apply {
    using S = typename C::Inject;
    FruitDelegateCheck(InjectTypedefNotASignature<C, S>);
    FruitDelegateCheck(InjectTypedefForWrongClass<C, Apply<SignatureType, S>>);
    FruitDelegateCheck(NoConstructorMatchingInjectSignature<C, S>);
    static constexpr bool ok = true
        && ApplyC<IsValidSignature, S>::value
        && std::is_same<C, Apply<SignatureType, S>>::value
        && ApplyC<IsConstructibleWithVector, C, Apply<UnlabelAssisted, Apply<SignatureArgs, S>>>::value;
    // Don't even provide it if the asserts above failed. Otherwise the compiler goes ahead and may go into a long loop,
    // e.g. with an Inject=int(C) in a class C.
    using type = typename std::enable_if<ok, S>::type;
  };
};

// This MUST NOT use None, otherwise None will get into the runtime dependency graph.
struct RemoveProvidersFromVectorHelper {
  template <typename... Ts>
  struct apply {
    using type = Vector<>;  
  };

  template <typename... Types, typename... Tail>
  struct apply<Provider<Types...>, Tail...> : public apply<Tail...> {
  };

  template <typename T, typename... Tail>
  struct apply<T, Tail...> {
    using type = Apply<AddToVector,
                       T, 
                       Apply<RemoveProvidersFromVectorHelper, Tail...>>;
  };
};

struct RemoveProvidersFromVector {
  template <typename V>
  struct apply {
    using type = ApplyWithVector<RemoveProvidersFromVectorHelper, V>;
  };
};

// Takes a vector of args, possibly including Provider<>s, and returns the set of required types.
struct ExpandProvidersInParamsHelper {
  // Empty vector.
  template <typename... Ts>
  struct apply {
    using type = Vector<>;
  };

  // Non-empty vector, T is not of the form Provider<C>
  template <typename T, typename... OtherTs>
  struct apply<T, OtherTs...> {
    using type = Apply<AddToSet, T, Apply<ExpandProvidersInParamsHelper, OtherTs...>>;
  };

  // Non-empty vector, type of the form Provider<C>
  template <typename C, typename... OtherTs>
  struct apply<fruit::Provider<C>, OtherTs...> {
    using type = Apply<AddToSet, C, Apply<ExpandProvidersInParamsHelper, OtherTs...>>;
  };
};

struct ExpandProvidersInParams {
  template <typename V>
  struct apply;

  template <typename... Ts>
  struct apply<Vector<Ts...>> {
    using type = Apply<ExpandProvidersInParamsHelper, Ts...>;
  };
};

struct HasInterfaceBinding {
  template <typename I, typename InterfaceBindings>
  struct apply;

  template <typename I, typename... InterfaceBindings>
  struct apply<I, Vector<InterfaceBindings...>> {
    static constexpr bool value = StaticOr<std::is_same<I, typename InterfaceBindings::Interface>::value...>::value;
  };
};

struct GetInterfaceBindingHelper {
  template <typename I, typename... InterfaceBindings>
  struct apply;

  template <typename I, typename C, typename... InterfaceBindings>
  struct apply<I, ConsInterfaceBinding<I, C>, InterfaceBindings...> {
    using type = C;
  };

  template <typename I, typename OtherInterfaceBinding, typename... InterfaceBindings>
  struct apply<I, OtherInterfaceBinding, InterfaceBindings...> : public apply<I, InterfaceBindings...> {
  };
};

struct GetInterfaceBinding {
  template <typename I, typename InterfaceBindings>
  struct apply;

  template <typename I, typename... InterfaceBindings>
  struct apply<I, Vector<InterfaceBindings...>> {
    using type = Apply<GetInterfaceBindingHelper, I, InterfaceBindings...>;
  };
};

// True if C has at least 1 reverse binding.
struct HasBindingToInterface {
  template <typename C, typename... InterfaceBindings>
  struct apply {
    static constexpr bool value = StaticOr<std::is_same<C, typename InterfaceBindings::Impl>::value...>::value;
  };
};

struct GetBindingToInterfaceHelper {
  template <typename I, typename... InterfaceBindings>
  struct apply {
    using type = None;
  };

  template <typename C, typename I, typename... InterfaceBindings>
  struct apply<C, ConsInterfaceBinding<I, C>, InterfaceBindings...> {
    using type = Eval<std::conditional<ApplyC<HasBindingToInterface, C, InterfaceBindings...>::value,
                                       None,
                                       I>>;
  };

  template <typename I, typename OtherInterfaceBindings, typename... InterfaceBindings>
  struct apply<I, OtherInterfaceBindings, InterfaceBindings...> : public apply<I, InterfaceBindings...> {
  };
};

// If there's a single interface I bound to C, returns I.
// If there is no interface bound to C, or if there are multiple, returns None.
struct GetBindingToInterface {
  template <typename I, typename InterfaceBindings>
  struct apply;

  template <typename I, typename... InterfaceBindings>
  struct apply<I, Vector<InterfaceBindings...>> {
    using type = Apply<GetBindingToInterfaceHelper, I, InterfaceBindings...>;
  };
};

//********************************************************************************************************************************
// Part 2: Type functors involving at least one ConsComp.
//********************************************************************************************************************************

template <typename RsParam, typename PsParam, typename DepsParam, typename InterfaceBindingsParam>
struct ConsComp {
  using Rs = RsParam;
  using Ps = PsParam;
  using Deps = DepsParam;
  using InterfaceBindings = InterfaceBindingsParam;
  
  // Invariants:
  // * all types appearing as arguments of Deps are in Rs
  // * all types in Ps are at the head of one (and only one) Dep.
  //   (note that the types in Rs can appear in deps any number of times, 0 is also ok)
  // * Deps is of the form Vector<Dep...> with each Dep of the form T(Args...) and where Vector<Args...> is a set (no repetitions).
  // * Bindings is a proof tree forest, with injected classes as formulas.
  
#ifndef FRUIT_NO_LOOP_CHECK
  FruitStaticAssert(true || sizeof(CheckDepsNormalized<Apply<AddProofTreeVectorToForest, Deps, EmptyProofForest, Vector<>>, Deps>), "");
#endif // !FRUIT_NO_LOOP_CHECK
};

struct ConstructComponentImpl {
  // Non-specialized case: no requirements.
  template <typename... Ps>
  struct apply {
    FruitDelegateCheck(CheckNoRepeatedTypes<Ps...>);
    FruitDelegateChecks(CheckClassType<Ps, Apply<GetClassForType, Ps>>);
    using type = ConsComp<Vector<>,
                          Vector<Ps...>,
                          Apply<ConstructProofForest, Vector<>, Ps...>,
                          Vector<>>;
#ifndef FRUIT_NO_LOOP_CHECK
    FruitStaticAssert(true || sizeof(CheckDepsNormalized<Apply<AddProofTreeVectorToForest, typename type::Deps, EmptyProofForest, Vector<>>, typename type::Deps>), "");
#endif // !FRUIT_NO_LOOP_CHECK
  };

  // With requirements.
  template <typename... Rs, typename... Ps>
  struct apply<Required<Rs...>, Ps...> {
    FruitDelegateCheck(CheckNoRepeatedTypes<Rs..., Ps...>);
    FruitDelegateChecks(CheckClassType<Rs, Apply<GetClassForType, Rs>>);
    FruitDelegateChecks(CheckClassType<Ps, Apply<GetClassForType, Ps>>);
    using type = ConsComp<Vector<Rs...>,
                          Vector<Ps...>,
                          Apply<ConstructProofForest, Vector<Rs...>, Ps...>,
                          Vector<>>;
#ifndef FRUIT_NO_LOOP_CHECK
    FruitStaticAssert(true || sizeof(CheckDepsNormalized<Apply<AddProofTreeVectorToForest, typename type::Deps, EmptyProofForest, Vector<>>, typename type::Deps>), "");
#endif // !FRUIT_NO_LOOP_CHECK
  };
};

// Adds the types in L to the requirements (unless they are already provided/required).
// Takes care of converting the types to the corresponding class type and expands any Provider<>s.
struct AddRequirements {
  template <typename Comp, typename ArgVector>
  struct apply {
    // TODO: Pass down a set of requirements to this metafunction instead.
    using ArgSet = Apply<ExpandProvidersInParams, 
                         Apply<GetClassForTypeVector, ArgVector>>;
    using newRequirements = Apply<SetUnion,
                                  Apply<SetDifference, ArgSet, typename Comp::Ps>,
                                  typename Comp::Rs>;
    using type = ConsComp<newRequirements,
                          typename Comp::Ps,
                          typename Comp::Deps,
                          typename Comp::InterfaceBindings>;
  };
};

// Adds C to the provides and removes it from the requirements (if it was there at all).
// Also checks that it wasn't already provided.
// Moreover, adds the requirements of C to the requirements, unless they were already provided/required.
// Takes care of converting the types to the corresponding class type and expands any Provider<>s.
struct AddProvidedType {
  template <typename Comp, typename C, typename ArgVector>
  struct apply {
    // TODO: Pass down a set of requirements to this metafunction instead.
    using ArgSet = Apply<ExpandProvidersInParams, 
                         Apply<GetClassForTypeVector, ArgVector>>;
    using newDeps = Apply<AddProofTreeToForest,
                          ConsProofTree<ArgSet, C>,
                          typename Comp::Deps,
                          typename Comp::Ps>;
    // Note: this should be before the rest so that we fail here in case of a loop.
    FruitDelegateCheck(CheckHasNoSelfLoopHelper<!std::is_same<newDeps, None>::value, C, ArgVector>);
    FruitDelegateCheck(CheckTypeAlreadyBound<!ApplyC<IsInVector, C, typename Comp::Ps>::value, C>);
    using newRequirements = Apply<SetUnion,
                                  Apply<SetDifference, ArgSet, typename Comp::Ps>,
                                  Apply<RemoveFromVector, C, typename Comp::Rs>>;
    using newProvides     = Apply<AddToVector, C, typename Comp::Ps>;
    using type = ConsComp<newRequirements,
                          newProvides,
                          newDeps,
                          typename Comp::InterfaceBindings>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_COMPONENT_H
