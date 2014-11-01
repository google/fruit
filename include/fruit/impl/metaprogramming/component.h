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

#ifndef FRUIT_METAPROGRAMMING_COMPONENT_H
#define FRUIT_METAPROGRAMMING_COMPONENT_H

#include "../../fruit_forward_decls.h"
#include "set.h"
#include "metaprogramming.h"
#include "../injection_debug_errors.h"

#include <memory>

namespace fruit {
namespace impl {

//********************************************************************************************************************************
// Part 1: Simple type functors (no ConsComp involved).
//********************************************************************************************************************************

// Represents a dependency from the binding of type T to the list of types Rs.
// Rs must be a set.
template <typename T, typename Rs>
struct ConsDep {
  using Type = T;
  using Requirements = Rs;
};

template <typename I, typename C>
struct ConsBinding {
  using Interface = I;
  using Impl = C;
};

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
};

struct GetClassForTypeList {
  template <typename L>
  struct apply;

  template <typename... Ts>
  struct apply<List<Ts...>> {
    using type = List<Apply<GetClassForType, Ts>...>;
  };
};

struct IsValidSignature {
  template <typename Signature>
  struct apply : std::false_type {};

  template <typename T, typename... Args>
  struct apply<T(Args...)> : std::true_type {};
};

struct ExtractRequirementFromAssistedParam {
  // Non-assisted case
  template <typename T>
  struct apply {
    using type = Apply<GetClassForType, T>;
  };

  template <typename T>
  struct apply<Assisted<T>> {
    using type = None;
  };
};

// Takes a list of types, considers only the assisted ones, transforms them to classes with
// GetClassForType and returns the resulting list. Note: the output list might contain some None elements.
struct ExtractRequirementsFromAssistedParams {
  template <typename L>
  struct apply;

  template <typename... Ts>
  struct apply<List<Ts...>> {
    using type = List<Apply<ExtractRequirementFromAssistedParam, Ts>...>;
  };
};

struct RemoveNonAssistedHelper {
  // No args.
  template <typename... Ts>
  struct apply {
    using type = List<>;
  };

  // Non-assisted case
  template <typename T, typename... Ts>
  struct apply<T, Ts...> {
    using type = Apply<RemoveNonAssistedHelper, Ts...>;
  };

  // Assisted case
  template <typename T, typename... Ts>
  struct apply<Assisted<T>, Ts...> {
    using type = Apply<AddToList,
                       T,
                       Apply<RemoveNonAssistedHelper, Ts...>>;
  };
};

struct RemoveNonAssisted {
  template <typename L>
  struct apply {
    using type = ApplyWithList<RemoveNonAssistedHelper, L>;
  };
};

struct RemoveAssistedHelper {
  // Empty list.
  template <typename... Ts>
  struct apply {
    using type = List<>;
  };

  // Non-assisted case
  template <typename T, typename... Ts>
  struct apply<T, Ts...> {
    using type = Apply<AddToList,
                       T,
                       Apply<RemoveAssistedHelper, Ts...>>;
  };

  // Assisted case
  template <typename T, typename... Ts>
  struct apply<Assisted<T>, Ts...> : public apply<Ts...> {
  };
};

struct RemoveAssisted {
  template <typename L>
  struct apply {
    using type = ApplyWithList<RemoveAssistedHelper, L>;
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
  template <typename L>
  struct apply;

  template <typename... Ts>
  struct apply<List<Ts...>> {
    using type = List<Apply<UnlabelAssistedSingleType, Ts>...>;
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

struct NumProvidersBeforeHelper {
  template <int index, typename... Ts>
  class apply;

  template <typename T, typename... Ts>
  class apply<0, T, Ts...> : public std::integral_constant<int, 0> {};

  // This is needed because the previous is not more specialized than the specialization with a provider ans generic index.
  template <typename... ProviderArgs, typename... Ts>
  class apply<0, Provider<ProviderArgs...>, Ts...> : public std::integral_constant<int, 0> {};

  // Non-assisted T, index!=0.
  template <int index, typename T, typename... Ts>
  class apply<index, T, Ts...> : public apply<index-1, Ts...> {};

  // Assisted T, index!=0.
  template <int index, typename... ProviderArgs, typename... Ts>
  class apply<index, Provider<ProviderArgs...>, Ts...>: public std::integral_constant<int, 1 + apply<index-1, Ts...>::value> {};
};

template <int index, typename L>
struct NumProvidersBefore;

template <int index, typename... Ts>
struct NumProvidersBefore<index, List<Ts...>> : public NumProvidersBeforeHelper::template apply<index, Ts...> {
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

template <int index, typename L>
struct NumAssistedBefore;

template <int index, typename... Ts>
struct NumAssistedBefore<index, List<Ts...>> : public NumAssistedBeforeHelper::template apply<index, Ts...> {
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
        && ApplyC<IsConstructibleWithList, C, Apply<UnlabelAssisted, Apply<SignatureArgs, S>>>::value;
    // Don't even provide it if the asserts above failed. Otherwise the compiler goes ahead and may go into a long loop,
    // e.g. with an Inject=int(C) in a class C.
    using type = typename std::enable_if<ok, S>::type;
  };
};

struct RemoveRequirementFromDep {
  template <typename C, typename Dep>
  struct apply {
    using type = ConsDep<typename Dep::Type, Apply<RemoveFromList, C, typename Dep::Requirements>>;
  };
};

struct RemoveRequirementFromDeps {
  template <typename C, typename Deps>
  struct apply;

  template <typename C, typename... Deps>
  struct apply<C, List<Deps...>> {
    using type = List<Apply<RemoveRequirementFromDep, C, Deps>...>;
  };
};

#ifndef FRUIT_NO_LOOP_CHECK

struct ConstructDep {
  template <typename P, typename Rs>
  struct apply {
    using type = ConsDep<P, Apply<ListToSet, Rs>>;
  };
};

struct ConstructDeps {
  template <typename Rs, typename... P>
  struct apply {
    using type = List<Apply<ConstructDep, P, Rs>...>;
  };
};

struct HasSelfLoop {
  template <typename Dep>
  struct apply : ApplyC<IsInList, typename Dep::Type, typename Dep::Requirements> {
  };
};

struct CanonicalizeDepRequirementsWithDep {
  template <typename Requirements, typename D1>
  struct apply {
    using type = Apply<ReplaceWithSet, Requirements, typename D1::Type, typename D1::Requirements>;
  };
};

struct CanonicalizeDepWithDep {
  template <typename D, typename D1>
  struct apply {
    using type = ConsDep<typename D::Type, Apply<CanonicalizeDepRequirementsWithDep, typename D::Requirements, D1>>;
  };
};

struct CanonicalizeDepsWithDep {
  template <typename Deps, typename Dep>
  struct apply;

  template <typename... Deps, typename Dep>
  struct apply<List<Deps...>, Dep> {
    using type = List<Apply<CanonicalizeDepWithDep, Deps, Dep>...>;
  };
};

struct CanonicalizeDepRequirementsWithDepsHelper {
  template <typename Requirements, typename OtherDeps>
  struct apply;

  template <typename Requirements, typename... OtherDeps>
  struct apply<Requirements, List<OtherDeps...>> {
    using type = List<Eval<std::conditional<ApplyC<IsInList, typename OtherDeps::Type, Requirements>::value,
                                            typename OtherDeps::Requirements,
                                            List<>>>...>;
  };
};

// TODO: ListOfSetsUnion is slow, consider optimizing here to avoid it.
struct CanonicalizeDepWithDeps {
  template <typename Dep, typename Deps, typename DepsTypes>
  struct apply {
    using type = ConsDep<typename Dep::Type,
      Apply<SetUnion,
        Apply<ListOfSetsUnion,
          Apply<CanonicalizeDepRequirementsWithDepsHelper, typename Dep::Requirements, Deps>
        >,
        Apply<SetDifference,
          typename Dep::Requirements,
          DepsTypes
        >
      >
    >;
  };
};

struct AddDep {
  template <typename Dep, typename Deps, typename DepsTypes>
  struct apply {
    using CanonicalizedDep = Apply<CanonicalizeDepWithDeps, Dep, Deps, DepsTypes>;
    FruitDelegateCheck(CheckHasNoSelfLoop<!ApplyC<HasSelfLoop, CanonicalizedDep>::value,
                                          typename CanonicalizedDep::Type>);
    // At this point CanonicalizedDep doesn't have as arguments any types appearing as heads in Deps,
    // but the head of CanonicalizedDep might appear as argument of some Deps.
    // A single replacement step is sufficient.
    using type = Apply<AddToList, CanonicalizedDep, Apply<CanonicalizeDepsWithDep, Deps, CanonicalizedDep>>;
  };
};

struct AddDepsHelper {
  // Case with empty Deps....
  template <typename OtherDeps, typename OtherDepsTypes, typename... Deps>
  struct apply {
    using type = OtherDeps;
  };

  template <typename OtherDeps, typename OtherDepsTypes, typename Dep, typename... Deps>
  struct apply<OtherDeps, OtherDepsTypes, Dep, Deps...> {
    using type = Apply<AddDepsHelper,
                       Apply<AddDep, Dep, OtherDeps, OtherDepsTypes>,
                       Apply<AddToList, typename Dep::Type, OtherDepsTypes>,
                       Deps...>;
  };
};

struct AddDeps {
  template <typename Deps, typename OtherDeps, typename OtherDepsTypes>
  struct apply {
    using type = ApplyWithList<AddDepsHelper, Deps, OtherDeps, OtherDepsTypes>;
  };
};

#else // FRUIT_NO_LOOP_CHECK

struct ConstructDep {
  template <typename P, typename Rs>
  struct apply {
    using type = None;
  };
};

struct ConstructDeps {
  template <typename Rs, typename... P>
  struct apply {
    using type = List<>;
  };
};

struct AddDep {
  template <typename Dep, typename Deps, typename DepsTypes>
  struct apply {
    using type = List<>;
  };
};

struct AddDeps {
  template <typename Deps, typename OtherDeps, typename OtherDepsTypes>
  struct apply {
    using type = List<>;
  };
};

#endif // FRUIT_NO_LOOP_CHECK

#ifdef FRUIT_EXTRA_DEBUG

template <typename D, typename Deps>
struct CheckDepEntailed {
  FruitStaticAssert(false && sizeof(D), "bug! should never instantiate this.");
};

template <typename D>
struct CheckDepEntailed<D, List<>> {
  static_assert(false && sizeof(D), "The dep D has no match in Deps");
};

// DType is not D1Type, not the dep that we're looking for.
template <typename DType, typename... DArgs, typename D1Type, typename... D1Args, typename... Ds>
struct CheckDepEntailed<ConsDep<DType, List<DArgs...>>, List<ConsDep<D1Type, List<D1Args...>>, Ds...>> 
: public CheckDepEntailed<ConsDep<DType, List<DArgs...>>, List<Ds...>> {};

// Found the dep that we're looking for, check that the args are a subset.
template <typename DType, typename... DArgs, typename... D1Args, typename... Ds>
struct CheckDepEntailed<ConsDep<DType, List<DArgs...>>, List<ConsDep<DType, List<D1Args...>>, Ds...>> {
  static_assert(ApplyC<IsEmptyList, Apply<SetDifference, List<D1Args...>, List<DArgs...>>>::value, 
                "Error, the args in the new dep are not a superset of the ones in the old one");
};

// General case: DepsSubset is empty.
template <typename DepsSubset, typename Deps>
struct CheckDepsSubset {
  static_assert(ApplyC<IsEmptyList, DepsSubset>::value, "");
};

template <typename D1, typename... D, typename Deps>
struct CheckDepsSubset<List<D1, D...>, Deps> : CheckDepsSubset<List<D...>, Deps> {
  FruitDelegateCheck(CheckDepEntailed<D1, Deps>);
};

// General case: DepsSubset is empty.
template <typename Comp, typename EntailedComp>
struct CheckComponentEntails {
  using AdditionalProvidedTypes = Apply<SetDifference, typename EntailedComp::Ps, typename Comp::Ps>;
  FruitDelegateCheck(CheckNoAdditionalProvidedTypes<AdditionalProvidedTypes>);
  using AdditionalBindings = Apply<SetDifference, typename EntailedComp::Bindings, typename Comp::Bindings>;
  FruitDelegateCheck(CheckNoAdditionalBindings<AdditionalBindings>);
  using NoLongerRequiredTypes = Apply<SetDifference, typename Comp::Rs, typename EntailedComp::Rs>;
  FruitDelegateCheck(CheckNoTypesNoLongerRequired<NoLongerRequiredTypes>);
  FruitDelegateCheck(CheckDepsSubset<typename EntailedComp::Deps, typename Comp::Deps>);
};

#endif // FRUIT_EXTRA_DEBUG

// This MUST NOT use None, otherwise None will get into the runtime dependency graph.
struct RemoveProvidersFromListHelper {
  template <typename... Ts>
  struct apply {
    using type = List<>;  
  };

  template <typename... Types, typename... Tail>
  struct apply<Provider<Types...>, Tail...> : public apply<Tail...> {
  };

  template <typename T, typename... Tail>
  struct apply<T, Tail...> {
    using type = Apply<AddToList,
                       T, 
                       Apply<RemoveProvidersFromListHelper, Tail...>>;
  };
};

struct RemoveProvidersFromList {
  template <typename L>
  struct apply {
    using type = ApplyWithList<RemoveProvidersFromListHelper, L>;
  };
};

struct ExpandProvidersInParamsHelper {
  // Empty list.
  template <typename... Ts>
  struct apply {
    using type = List<>;
  };

  // Non-empty list, T is not of the form Provider<Ts...>
  template <typename T, typename... OtherTs>
  struct apply<T, OtherTs...> {
    using type = Apply<AddToSet, T, Apply<ExpandProvidersInParamsHelper, OtherTs...>>;
  };

  // Non-empty list, type of the form Provider<Ts...>
  template <typename... Ts, typename... OtherTs>
  struct apply<fruit::Provider<Ts...>, OtherTs...> {
    using type = Apply<AddToSetMultiple, Apply<ExpandProvidersInParamsHelper, OtherTs...>, Ts...>;
  };
};

struct ExpandProvidersInParams {
  template <typename L>
  struct apply;

  // Non-empty list, T is not of the form Provider<Ts...>
  template <typename... Ts>
  struct apply<List<Ts...>> {
    using type = Apply<ExpandProvidersInParamsHelper, Ts...>;
  };
};

struct HasBinding {
  template <typename I, typename Bindings>
  struct apply;

  template <typename I, typename... Bindings>
  struct apply<I, List<Bindings...>> {
    static constexpr bool value = StaticOr<std::is_same<I, typename Bindings::Interface>::value...>::value;
  };
};

struct GetBindingHelper {
  template <typename I, typename... Bindings>
  struct apply;

  template <typename I, typename C, typename... Bindings>
  struct apply<I, ConsBinding<I, C>, Bindings...> {
    using type = C;
  };

  template <typename I, typename OtherBinding, typename... Bindings>
  struct apply<I, OtherBinding, Bindings...> : public apply<I, Bindings...> {
  };
};

struct GetBinding {
  template <typename I, typename Bindings>
  struct apply;

  template <typename I, typename... Bindings>
  struct apply<I, List<Bindings...>> {
    using type = Apply<GetBindingHelper, I, Bindings...>;
  };
};

//********************************************************************************************************************************
// Part 2: Type functors involving at least one ConsComp.
//********************************************************************************************************************************

template <typename RsParam, typename PsParam, typename DepsParam, typename BindingsParam>
struct ConsComp {
  using Rs = RsParam;
  using Ps = PsParam;
  using Deps = DepsParam;
  using Bindings = BindingsParam;
  
  // Invariants:
  // * all types appearing as arguments of Deps are in Rs
  // * all types in Ps are at the head of one (and only one) Dep.
  //   (note that the types in Rs can appear in deps any number of times, 0 is also ok)
  // * Deps is of the form List<Dep...> with each Dep of the form T(Args...) and where List<Args...> is a set (no repetitions).
  // * Bindings is of the form List<ConsBinding<I1, C1>, ..., ConsBinding<In, Cn>> and is a set (no repetitions).
  
#ifndef FRUIT_NO_LOOP_CHECK
  FruitStaticAssert(true || sizeof(CheckDepsNormalized<Apply<AddDeps, Deps, List<>, List<>>, Deps>), "");
#endif // !FRUIT_NO_LOOP_CHECK
};

struct ConstructComponentImpl {
  // Non-specialized case: no requirements.
  template <typename... Ps>
  struct apply {
    FruitDelegateCheck(CheckNoRepeatedTypes<Ps...>);
    FruitDelegateChecks(CheckClassType<Ps, Apply<GetClassForType, Ps>>);
    using type = ConsComp<List<>,
                          List<Ps...>,
                          Apply<ConstructDeps, List<>, Ps...>,
                          List<>>;
  };

  // With requirements.
  template <typename... Rs, typename... Ps>
  struct apply<Required<Rs...>, Ps...> {
    FruitDelegateCheck(CheckNoRepeatedTypes<Rs..., Ps...>);
    FruitDelegateChecks(CheckClassType<Rs, Apply<GetClassForType, Rs>>);
    FruitDelegateChecks(CheckClassType<Ps, Apply<GetClassForType, Ps>>);
    using type = ConsComp<List<Rs...>,
                          List<Ps...>,
                          Apply<ConstructDeps, List<Rs...>, Ps...>,
                          List<>>;
  };
};

struct AddRequirements;

struct AddRequirementHelper {
  template <typename Comp, typename C>
  struct apply {
    using type = ConsComp<Apply<AddToList, C, typename Comp::Rs>,
                          typename Comp::Ps,
                          typename Comp::Deps,
                          typename Comp::Bindings>;
  };
};

struct AddRequirement {
  template <typename Comp, typename C>
  struct apply {
    using type = Conditional<ApplyC<IsInList, C, typename Comp::Ps>::value
                          || ApplyC<IsInList, C, typename Comp::Rs>::value,
                             Lazy<Comp>,
                             LazyApply<AddRequirementHelper, Comp, C>>;
  };
};

// Adds the types in L to the requirements (unless they are already provided/required).
struct AddRequirements {
  // Empty list.
  template <typename Comp, typename L>
  struct apply {
    FruitStaticAssert(ApplyC<IsEmptyList, L>::value, "Implementation error: L not a list");
    using type = Comp;
  };

  template <typename Comp, typename OtherR, typename... OtherRs>
  struct apply<Comp, List<OtherR, OtherRs...>> {
    using type = Apply<AddRequirement,
                       Apply<AddRequirements, Comp, List<OtherRs...>>,
                       OtherR>;
  };
};

// Removes the requirement, assumes that the type is now bound.
struct RemoveRequirement {
  template <typename Comp, typename C>
  struct apply {
    using type = ConsComp<Apply<RemoveFromList, C, typename Comp::Rs>,
                          typename Comp::Ps,
                          Apply<RemoveRequirementFromDeps, C, typename Comp::Deps>,
                          typename Comp::Bindings>;
  };
};

// Adds C to the provides and removes it from the requirements (if it was there at all).
// Also checks that it wasn't already provided.
struct AddProvide {
  template <typename Comp, typename C, typename ArgList>
  struct apply {
    // Note: this should be before the FruitDelegateCheck so that we fail here in case of a loop.
    using newDeps = Apply<AddDep,
                          Apply<ConstructDep, C, ArgList>,
                          typename Comp::Deps,
                          typename Comp::Ps>;
    FruitDelegateCheck(CheckTypeAlreadyBound<!ApplyC<IsInList, C, typename Comp::Ps>::value, C>);
    using Comp1 = ConsComp<typename Comp::Rs,
                           Apply<AddToList, C, typename Comp::Ps>,
                           newDeps,
                           typename Comp::Bindings>;
    using type = Apply<RemoveRequirement,
                       Comp1,
                       C>;
  };
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_METAPROGRAMMING_COMPONENT_H
