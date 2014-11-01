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

#ifndef FRUIT_COMPONENT_IMPL_H
#define FRUIT_COMPONENT_IMPL_H

#include "../fruit_forward_decls.h"
#include "fruit_assert.h"
#include "metaprogramming/set.h"
#include "metaprogramming/metaprogramming.h"
#include "injection_debug_errors.h"

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


// General case, if none of the following apply.
// When adding a specialization here, make sure that the ComponentStorage
// can actually get<> the specified type when the class was registered.
template <typename T>
struct GetClassForTypeHelper {using type = T;};

template <typename T>
struct GetClassForTypeHelper<const T> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<T*> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<T&> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<const T*> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<const T&> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<std::shared_ptr<T>> {using type = T;};

template <typename T>
using GetClassForType = typename GetClassForTypeHelper<T>::type;

template <typename L>
struct GetClassForTypeListHelper {}; // Not used.

template <typename... Ts>
struct GetClassForTypeListHelper<List<Ts...>> {
  using type = List<GetClassForType<Ts>...>;
};

template <typename L>
using GetClassForTypeList = typename GetClassForTypeListHelper<L>::type;

template <typename Signature>
struct IsValidSignature : std::false_type {};

template <typename T, typename... Args>
struct IsValidSignature<T(Args...)> : public static_and<!is_list<T>::value, !is_list<Args>::value...> {};

// Non-assisted case
template <typename T>
struct ExtractRequirementFromAssistedParamHelper {
  using type = GetClassForType<T>;
};

template <typename T>
struct ExtractRequirementFromAssistedParamHelper<Assisted<T>> {
  using type = None;
};

template <typename L>
struct ExtractRequirementsFromAssistedParamsHelper {};

template <typename... Ts>
struct ExtractRequirementsFromAssistedParamsHelper<List<Ts...>> {
  using type = List<typename ExtractRequirementFromAssistedParamHelper<Ts>::type...>;
};

// Takes a list of types, considers only the assisted ones, transforms them to classes with
// GetClassForType and returns the resulting list. Note: the output list might contain some None elements.
template <typename L>
using ExtractRequirementsFromAssistedParams = typename ExtractRequirementsFromAssistedParamsHelper<L>::type;

template <typename L>
struct RemoveNonAssistedHelper {};

template <>
struct RemoveNonAssistedHelper<List<>> {
  using type = List<>;
};

// Non-assisted case
template <typename T, typename... Ts>
struct RemoveNonAssistedHelper<List<T, Ts...>> {
  using type = typename RemoveNonAssistedHelper<List<Ts...>>::type;
};

// Assisted case
template <typename T, typename... Ts>
struct RemoveNonAssistedHelper<List<Assisted<T>, Ts...>> {
  using type = add_to_list<T, typename RemoveNonAssistedHelper<List<Ts...>>::type>;
};

template <typename L>
using RemoveNonAssisted = typename RemoveNonAssistedHelper<L>::type;

template <typename L>
struct RemoveAssistedHelper {};

template <>
struct RemoveAssistedHelper<List<>> {
  using type = List<>;
};

// Non-assisted case
template <typename T, typename... Ts>
struct RemoveAssistedHelper<List<T, Ts...>> {
  using type = add_to_list<T, typename RemoveAssistedHelper<List<Ts...>>::type>;
};

// Assisted case
template <typename T, typename... Ts>
struct RemoveAssistedHelper<List<Assisted<T>, Ts...>> {
  using type = typename RemoveAssistedHelper<List<Ts...>>::type;
};

template <typename L>
using RemoveAssisted = typename RemoveAssistedHelper<L>::type;

template <typename T>
struct UnlabelAssistedSingleType {
  using type = T;
};

template <typename T>
struct UnlabelAssistedSingleType<Assisted<T>> {
  using type = T;
};

template <typename L>
struct UnlabelAssistedHelper {};

template <typename... Ts>
struct UnlabelAssistedHelper<List<Ts...>> {
  using type = List<typename UnlabelAssistedSingleType<Ts>::type...>;
};

template <typename L>
using UnlabelAssisted = typename UnlabelAssistedHelper<L>::type;

template <typename AnnotatedSignature>
using RequiredArgsForAssistedFactory = UnlabelAssisted<SignatureArgs<AnnotatedSignature>>;

template <typename AnnotatedSignature>
using RequiredSignatureForAssistedFactory = ConstructSignature<SignatureType<AnnotatedSignature>,
                                                               RequiredArgsForAssistedFactory<AnnotatedSignature>>;

template <typename AnnotatedSignature>
using InjectedFunctionArgsForAssistedFactory = RemoveNonAssisted<SignatureArgs<AnnotatedSignature>>;

template <typename AnnotatedSignature>
using InjectedSignatureForAssistedFactory = ConstructSignature<SignatureType<AnnotatedSignature>,
                                                               InjectedFunctionArgsForAssistedFactory<AnnotatedSignature>>;
template <int index, typename Ts>
class NumProvidersBefore {}; // Not used. Instantiated only if index is out of bounds.

template <typename T, typename... Ts>
class NumProvidersBefore<0, List<T, Ts...>> : public std::integral_constant<int, 0> {};

// This is needed because the previous is not more specialized than the specialization with a provider ans generic index.
template <typename... ProviderArgs, typename... Ts>
class NumProvidersBefore<0, List<Provider<ProviderArgs...>, Ts...>> : public std::integral_constant<int, 0> {};

// Non-assisted T, index!=0.
template <int index, typename T, typename... Ts>
class NumProvidersBefore<index, List<T, Ts...>> : public NumProvidersBefore<index-1, List<Ts...>> {};

// Assisted T, index!=0.
template <int index, typename... ProviderArgs, typename... Ts>
class NumProvidersBefore<index, List<Provider<ProviderArgs...>, Ts...>>: public std::integral_constant<int, 1 + NumProvidersBefore<index-1, List<Ts...>>::value> {};


template <int index, typename L>
class NumAssistedBefore {}; // Not used. Instantiated only if index is out of bounds.

template <typename T, typename... Ts>
class NumAssistedBefore<0, List<T, Ts...>> : public std::integral_constant<int, 0> {};

// This is needed because the previous is not more specialized than the specialization with assisted T.
template <typename T, typename... Ts>
class NumAssistedBefore<0, List<Assisted<T>, Ts...>> : public std::integral_constant<int, 0> {};

// Non-assisted T, index!=0.
template <int index, typename T, typename... Ts>
class NumAssistedBefore<index, List<T, Ts...>> : public NumAssistedBefore<index-1, List<Ts...>> {};

// Assisted T, index!=0.
template <int index, typename T, typename... Ts>
class NumAssistedBefore<index, List<Assisted<T>, Ts...>> : public std::integral_constant<int, 1 + NumAssistedBefore<index-1, List<Ts...>>::value> {};

// Exposes a bool `value' (whether C is injectable with annotation)
template <typename C>
struct HasInjectAnnotation {
    typedef char yes[1];
    typedef char no[2];

    template <typename C1>
    static yes& test(typename C1::Inject*);

    template <typename>
    static no& test(...);
    
    static const bool value = sizeof(test<C>(0)) == sizeof(yes);
};

template <typename C>
struct GetInjectAnnotation {
    using S = typename C::Inject;
    FruitDelegateCheck(InjectTypedefNotASignature<C, S>);
    using A = SignatureArgs<S>;
    FruitDelegateCheck(InjectTypedefForWrongClass<C, SignatureType<S>>);
    FruitDelegateCheck(NoConstructorMatchingInjectSignature<C, S>);
    static constexpr bool ok = true
        && IsValidSignature<S>::value
        && std::is_same<C, SignatureType<S>>::value
        && is_constructible_with_list<C, UnlabelAssisted<A>>::value;
    // Don't even provide them if the asserts above failed. Otherwise the compiler goes ahead and may go into a long loop,
    // e.g. with an Inject=int(C) in a class C.
    using Signature = typename std::enable_if<ok, S>::type;
    using Args = typename std::enable_if<ok, A>::type;
};

template <typename C, typename Dep>
using RemoveRequirementFromDep = ConsDep<typename Dep::Type, remove_from_list<C, typename Dep::Requirements>>;

template <typename C, typename Deps>
struct RemoveRequirementFromDepsHelper {}; // Not used

template <typename C, typename... Deps>
struct RemoveRequirementFromDepsHelper<C, List<Deps...>> {
  using type = List<RemoveRequirementFromDep<C, Deps>...>;
};

template <typename C, typename Deps>
using RemoveRequirementFromDeps = typename RemoveRequirementFromDepsHelper<C, Deps>::type;

template <typename P, typename Rs>
using ConstructDep = ConsDep<P, list_to_set<Rs>>;

template <typename Rs, typename... P>
using ConstructDeps = List<ConstructDep<P, Rs>...>;

template <typename Dep>
struct HasSelfLoop : is_in_list<typename Dep::Type, typename Dep::Requirements> {
};

template <typename Requirements, typename D1>
using CanonicalizeDepRequirementsWithDep = replace_with_set<Requirements, typename D1::Type, typename D1::Requirements>;

template <typename D, typename D1>
using CanonicalizeDepWithDep = ConsDep<typename D::Type, CanonicalizeDepRequirementsWithDep<typename D::Requirements, D1>>;

template <typename Deps, typename Dep>
struct CanonicalizeDepsWithDep {}; // Not used.

template <typename... Deps, typename Dep>
struct CanonicalizeDepsWithDep<List<Deps...>, Dep> {
  using type = List<CanonicalizeDepWithDep<Deps, Dep>...>;
};

template <typename Requirements, typename Deps>
struct CanonicalizeDepRequirementsWithDeps {}; // Not used.

template <typename Requirements>
struct CanonicalizeDepRequirementsWithDeps<Requirements, List<>> {
  using type = Requirements;
};

template <typename Requirements, typename D1, typename... Ds>
struct CanonicalizeDepRequirementsWithDeps<Requirements, List<D1, Ds...>> {
  using recursion_result = typename CanonicalizeDepRequirementsWithDeps<Requirements, List<Ds...>>::type;
  using type = CanonicalizeDepRequirementsWithDep<recursion_result, D1>;
};

template <typename Requirements, typename OtherDeps>
struct CanonicalizeDepRequirementsWithDepsHelper {}; // Not used.

template <typename Requirements, typename... OtherDeps>
struct CanonicalizeDepRequirementsWithDepsHelper<Requirements, List<OtherDeps...>> {
  using type = List<typename std::conditional<is_in_list<typename OtherDeps::Type, Requirements>::value,
                                              typename OtherDeps::Requirements,
                                              List<>>::type...>;
};

template <typename Dep, typename Deps, typename DepsTypes>
using CanonicalizeDepWithDeps = ConsDep<typename Dep::Type,
  set_union<
    list_of_sets_union<
      typename CanonicalizeDepRequirementsWithDepsHelper<typename Dep::Requirements, Deps>::type
    >,
    set_difference<
      typename Dep::Requirements,
      DepsTypes
    >
  >
>;

template <typename Dep, typename Deps, typename DepsTypes>
struct AddDepHelper {
  using CanonicalizedDep = CanonicalizeDepWithDeps<Dep, Deps, DepsTypes>;
  FruitDelegateCheck(CheckHasNoSelfLoop<!HasSelfLoop<CanonicalizedDep>::value, typename CanonicalizedDep::Type>);
  // At this point CanonicalizedDep doesn't have as arguments any types appearing as heads in Deps,
  // but the head of CanonicalizedDep might appear as argument of some Deps.
  // A single replacement step is sufficient.
  using type = add_to_list<CanonicalizedDep, typename CanonicalizeDepsWithDep<Deps, CanonicalizedDep>::type>;
};

template <typename Dep, typename Deps, typename DepsTypes>
using AddDep = typename AddDepHelper<Dep, Deps, DepsTypes>::type;

template <typename Deps, typename OtherDeps, typename OtherDepsTypes>
struct AddDepsHelper {};

template <typename OtherDepsList, typename OtherDepsTypes>
struct AddDepsHelper<List<>, OtherDepsList, OtherDepsTypes> {
  using type = OtherDepsList;
};

template <typename Dep, typename... Deps, typename OtherDepList, typename OtherDepsTypes>
struct AddDepsHelper<List<Dep, Deps...>, OtherDepList, OtherDepsTypes> {
  using step = AddDep<Dep, OtherDepList, OtherDepsTypes>;
  using type = typename AddDepsHelper<List<Deps...>, step, add_to_list<typename Dep::Type, OtherDepsTypes>>::type;
};

template <typename Deps, typename OtherDeps, typename OtherDepsTypes>
using AddDeps = typename AddDepsHelper<Deps, OtherDeps, OtherDepsTypes>::type;

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
  static_assert(is_empty_list<set_difference<List<D1Args...>, List<DArgs...>>>::value, "Error, the args in the new dep are not a superset of the ones in the old one");
};

// General case: DepsSubset is empty.
template <typename DepsSubset, typename Deps>
struct CheckDepsSubset {
  static_assert(is_empty_list<DepsSubset>::value, "");
};

template <typename D1, typename... D, typename Deps>
struct CheckDepsSubset<List<D1, D...>, Deps> : CheckDepsSubset<List<D...>, Deps> {
  FruitDelegateCheck(CheckDepEntailed<D1, Deps>);
};

// General case: DepsSubset is empty.
template <typename Comp, typename EntailedComp>
struct CheckComponentEntails {
  using AdditionalProvidedTypes = set_difference<typename EntailedComp::Ps, typename Comp::Ps>;
  FruitDelegateCheck(CheckNoAdditionalProvidedTypes<AdditionalProvidedTypes>);
  using AdditionalBindings = set_difference<typename EntailedComp::Bindings, typename Comp::Bindings>;
  FruitDelegateCheck(CheckNoAdditionalBindings<AdditionalBindings>);
  using NoLongerRequiredTypes = set_difference<typename Comp::Rs, typename EntailedComp::Rs>;
  FruitDelegateCheck(CheckNoTypesNoLongerRequired<NoLongerRequiredTypes>);
  FruitDelegateCheck(CheckDepsSubset<typename EntailedComp::Deps, typename Comp::Deps>);
};

#endif // FRUIT_EXTRA_DEBUG

// This MUST NOT use None, otherwise None will get into the runtime dependency graph.
template <typename L>
struct RemoveProvidersFromListHelper {
  using type = List<>;  
};

template <typename... Types, typename... Tail>
struct RemoveProvidersFromListHelper<List<Provider<Types...>, Tail...>> {
  using type = typename RemoveProvidersFromListHelper<List<Tail...>>::type;
};

template <typename T, typename... Tail>
struct RemoveProvidersFromListHelper<List<T, Tail...>> {
  using type = add_to_list<T, typename RemoveProvidersFromListHelper<List<Tail...>>::type>;
};

template <typename L>
using RemoveProvidersFromList = typename RemoveProvidersFromListHelper<L>::type;

template <typename... Ts>
struct ExpandProvidersInParamsHelper {};

template <>
struct ExpandProvidersInParamsHelper<> {
  using type = List<>;
};

// Non-empty list, T is not of the form Provider<Ts...>
template <typename T, typename... OtherTs>
struct ExpandProvidersInParamsHelper<T, OtherTs...> {
  using recursion_result = typename ExpandProvidersInParamsHelper<OtherTs...>::type;
  using type = add_to_set<T, recursion_result>;
};

// Non-empty list, type of the form Provider<Ts...>
template <typename... Ts, typename... OtherTs>
struct ExpandProvidersInParamsHelper<fruit::Provider<Ts...>, OtherTs...> {
  using recursion_result = typename ExpandProvidersInParamsHelper<OtherTs...>::type;
  using type = add_to_set_multiple<recursion_result, Ts...>;
};

template <typename L>
struct ExpandProvidersInParamsImpl {};

// Non-empty list, T is not of the form Provider<Ts...>
template <typename... Ts>
struct ExpandProvidersInParamsImpl<List<Ts...>> {
  using type = typename ExpandProvidersInParamsHelper<Ts...>::type;
};

template <typename L>
using ExpandProvidersInParams = typename ExpandProvidersInParamsImpl<L>::type;

template <typename I, typename Bindings>
struct HasBinding {};

template <typename I, typename... Bindings>
struct HasBinding<I, List<Bindings...>> {
  static constexpr bool value = static_or<std::is_same<I, typename Bindings::Interface>::value...>::value;
};

template <typename I, typename... Bindings>
struct GetBindingHelper {};

template <typename I, typename C, typename... Bindings>
struct GetBindingHelper<I, ConsBinding<I, C>, Bindings...> {
  using type = C;
};

template <typename I, typename OtherBinding, typename... Bindings>
struct GetBindingHelper<I, OtherBinding, Bindings...> {
  using type = typename GetBindingHelper<I, Bindings...>::type;
};

template <typename I, typename Bindings>
struct GetBindingImpl {};

template <typename I, typename... Bindings>
struct GetBindingImpl<I, List<Bindings...>> {
  using type = typename GetBindingHelper<I, Bindings...>::type;
};

template <typename I, typename Bindings>
using GetBinding = typename GetBindingImpl<I, Bindings>::type;

//********************************************************************************************************************************
// Part 2: Type functors involving at least one ConsComp.
//********************************************************************************************************************************

template <typename RsParam, typename PsParam, typename DepsParam, typename BindingsParam>
struct ConsComp {
  // Just for convenience.
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
  
  FruitStaticAssert(true || sizeof(CheckDepsNormalized<AddDeps<Deps, List<>, List<>>, Deps>), "");
};

// Non-specialized case: no requirements.
template <typename... Ps>
struct ConstructComponentImplHelper {
  FruitDelegateCheck(CheckNoRepeatedTypes<Ps...>);
  FruitDelegateChecks(CheckClassType<Ps, GetClassForType<Ps>>);
  using type = ConsComp<List<>,
                        List<Ps...>,
                        ConstructDeps<List<>, Ps...>,
                        List<>>;
};

// Non-specialized case: no requirements.
template <typename... Rs, typename... Ps>
struct ConstructComponentImplHelper<Required<Rs...>, Ps...> {
  FruitDelegateCheck(CheckNoRepeatedTypes<Rs..., Ps...>);
  FruitDelegateChecks(CheckClassType<Rs, GetClassForType<Rs>>);
  FruitDelegateChecks(CheckClassType<Ps, GetClassForType<Ps>>);
  using type = ConsComp<List<Rs...>,
                        List<Ps...>,
                        ConstructDeps<List<Rs...>, Ps...>,
                        List<>>;
};

template <typename... Types>
using ConstructComponentImpl = typename ConstructComponentImplHelper<Types...>::type;

template <typename Comp, typename L>
struct AddRequirementsHelper;

// Adds the types in L to the requirements (unless they are already provided/required).
template <typename Comp, typename L>
using AddRequirements = typename AddRequirementsHelper<Comp, L>::type;

template <typename Comp, bool is_already_present, typename C>
struct AddRequirementHelper; // Not used.

// Already present, nothing to check and nothing to add.
template <typename Comp, typename C>
struct AddRequirementHelper<Comp, true, C> {
  using type = Comp;
};

// Not present, add (general case).
template <typename Comp, typename C>
struct AddRequirementHelper<Comp, false, C> {
  using type = ConsComp<add_to_list<C, typename Comp::Rs>, typename Comp::Ps, typename Comp::Deps, typename Comp::Bindings>;
};

// Adds C to the requirements (unless it's already provided/required).
template <typename Comp, typename C>
using AddRequirement = typename AddRequirementHelper<
    Comp,
    is_in_list<C, typename Comp::Ps>::value || is_in_list<C, typename Comp::Rs>::value,
    C>::type;    

// Empty list.
template <typename Comp, typename L>
struct AddRequirementsHelper {
  FruitStaticAssert(is_empty_list<L>::value, "Implementation error: L not a list");
  using type = Comp;
};

template <typename Comp, typename OtherR, typename... OtherRs>
struct AddRequirementsHelper<Comp, List<OtherR, OtherRs...>> {
  using Comp1 = typename AddRequirementsHelper<Comp, List<OtherRs...>>::type;
  using type = AddRequirement<Comp1, OtherR>;
};

// Removes the requirement, assumes that the type is now bound.
template <typename Comp, typename C>
using RemoveRequirement = ConsComp<remove_from_list<C, typename Comp::Rs>, typename Comp::Ps, RemoveRequirementFromDeps<C, typename Comp::Deps>, typename Comp::Bindings>;

template <typename Comp, typename C, typename ArgList>
struct AddProvideHelper {
  // Note: this should be before the FruitDelegateCheck so that we fail here in case of a loop.
  using newDeps = AddDep<ConstructDep<C, ArgList>, typename Comp::Deps, typename Comp::Ps>;
  FruitDelegateCheck(CheckTypeAlreadyBound<!is_in_list<C, typename Comp::Ps>::value, C>);
  using Comp1 = ConsComp<typename Comp::Rs, add_to_list<C, typename Comp::Ps>, newDeps, typename Comp::Bindings>;
  using type = RemoveRequirement<Comp1, C>;
};

// Adds C to the provides and removes it from the requirements (if it was there at all).
// Also checks that it wasn't already provided.
template <typename Comp, typename C, typename ArgList>
using AddProvide = typename AddProvideHelper<Comp, C, ArgList>::type;

} // namespace impl
} // namespace fruit

//********************************************************************************************************************************
// Part 3: Functors to transform components.
//********************************************************************************************************************************


#include "component_storage.h"
#include "injection_errors.h"

namespace fruit {
namespace impl {

template <typename Comp, typename TargetRequirements, typename L>
struct EnsureProvidedTypes;

template <typename Comp>
struct Identity {
  using Result = Comp;
  void operator()(ComponentStorage& storage) {
    (void)storage;
  }
};

// Doesn't actually bind in ComponentStorage. The binding is added later (if needed) using BindNonFactory.
template <typename Comp, typename I, typename C>
struct Bind {
  using NewBindings = add_to_set<ConsBinding<I, C>, typename Comp::Bindings>;
  using Comp1 = ConsComp<typename Comp::Rs, typename Comp::Ps, typename Comp::Deps, NewBindings>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage) {
    (void)storage;
  };
};

template <typename Comp, typename I, typename C>
struct BindNonFactory {
  FruitDelegateCheck(CheckClassType<I, GetClassForType<I>>);
  FruitDelegateCheck(CheckClassType<C, GetClassForType<C>>);
  FruitDelegateCheck(NotABaseClassOf<I, C>);
  using Comp1 = AddRequirement<Comp, C>;
  using Comp2 = AddProvide<Comp1, I, List<C>>;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    storage.template bind<I, C>();
  };
};

template <typename Comp, typename I, typename C>
struct AddMultibinding {
  FruitDelegateCheck(CheckClassType<I, GetClassForType<I>>);
  FruitDelegateCheck(CheckClassType<C, GetClassForType<C>>);
  FruitDelegateCheck(NotABaseClassOf<I, C>);
  using Comp1 = AddRequirement<Comp, C>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage) {
    storage.template addMultibinding<I, C>();
  };
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerProvider() overloads in ComponentStorage.
template <typename Comp, typename Function>
struct RegisterProvider {
  using Signature = FunctionSignature<Function>;
  using SignatureRequirements = ExpandProvidersInParams<GetClassForTypeList<SignatureArgs<Signature>>>;
  using Comp1 = AddRequirements<Comp, SignatureRequirements>;
  using Comp2 = AddProvide<Comp1, GetClassForType<SignatureType<Signature>>, SignatureRequirements>;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    storage.registerProvider<Function>();
  }
};

// T can't be any injectable type, it must match the return type of the provider in one of
// the registerMultibindingProvider() overloads in ComponentStorage.
template <typename Comp, typename Function>
struct RegisterMultibindingProvider {
  using SignatureRequirements = ExpandProvidersInParams<GetClassForTypeList<SignatureArgs<FunctionSignature<Function>>>>;
  using Comp1 = AddRequirements<Comp, SignatureRequirements>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage) {
    storage.registerMultibindingProvider<Function>();
  }
};

template <typename Comp, typename AnnotatedSignature, typename Function>
struct RegisterFactory {
  using InjectedFunctionType = ConstructSignature<SignatureType<AnnotatedSignature>, InjectedFunctionArgsForAssistedFactory<AnnotatedSignature>>;
  using RequiredSignature = ConstructSignature<SignatureType<AnnotatedSignature>, RequiredArgsForAssistedFactory<AnnotatedSignature>>;
  FruitDelegateCheck(FunctorSignatureDoesNotMatch<RequiredSignature, FunctionSignature<Function>>);
  FruitDelegateCheck(FactoryReturningPointer<std::is_pointer<SignatureType<AnnotatedSignature>>::value, AnnotatedSignature>);
  using NewRequirements = ExpandProvidersInParams<ExtractRequirementsFromAssistedParams<SignatureArgs<AnnotatedSignature>>>;
  using Comp1 = AddRequirements<Comp, NewRequirements>;
  using Comp2 = AddProvide<Comp1, std::function<InjectedFunctionType>, NewRequirements>;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    storage.template registerFactory<AnnotatedSignature, Function>();
  }
};

template <typename Comp, typename Signature>
struct RegisterConstructor {
  // Something is wrong. We provide a Result and an operator() here to avoid backtracking with SFINAE, and to allow the function that
  // instantiated this class to return the appropriate error.
  // This method is not implemented.
  using Result = int;
  int operator()(Comp&& m);
};

template <typename Comp, typename T, typename... Args>
struct RegisterConstructor<Comp, T(Args...)> {
  using Signature = T(Args...);
  using SignatureRequirements = ExpandProvidersInParams<List<GetClassForType<Args>...>>;
  using Comp1 = AddRequirements<Comp, SignatureRequirements>;
  using Comp2 = AddProvide<Comp1, GetClassForType<T>, SignatureRequirements>;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    storage.template registerConstructor<T, Args...>();
  }
};

template <typename Comp, typename C>
struct RegisterInstance {
  using Comp1 = AddProvide<Comp, C, List<>>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage, C& instance) {
    storage.bindInstance(instance);
  };
};

template <typename Comp, typename C>
struct AddInstanceMultibinding {
  using Result = Comp;
  void operator()(ComponentStorage& storage, C& instance) {
    storage.addInstanceMultibinding(instance);
  };
};

template <typename Comp, typename AnnotatedSignature, typename RequiredSignature>
struct RegisterConstructorAsValueFactoryHelper {};

template <typename Comp, typename AnnotatedSignature, typename T, typename... Args>
struct RegisterConstructorAsValueFactoryHelper<Comp, AnnotatedSignature, T(Args...)> {
  void operator()(ComponentStorage& storage) {
    auto provider = [](Args... args) {
      return T(std::forward<Args>(args)...);
    };
    using RealRegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, decltype(provider)>;
    RealRegisterFactoryOperation()(storage);
  }
};

template <typename Comp, typename AnnotatedSignature>
struct RegisterConstructorAsValueFactory {
  using RequiredSignature = ConstructSignature<SignatureType<AnnotatedSignature>, RequiredArgsForAssistedFactory<AnnotatedSignature>>;
  using RegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, RequiredSignature>;
  using Comp1 = typename RegisterFactoryOperation::Result;
  using Result = Comp1;
  void operator()(ComponentStorage& storage) {
    RegisterConstructorAsValueFactoryHelper<Comp, AnnotatedSignature, RequiredSignature>()(storage);
  };
};

template <typename Comp, typename AnnotatedSignature, typename RequiredSignature>
struct RegisterConstructorAsPointerFactoryHelper {};

template <typename Comp, typename AnnotatedSignature, typename T, typename... Args>
struct RegisterConstructorAsPointerFactoryHelper<Comp, AnnotatedSignature, std::unique_ptr<T>(Args...)> {
  void operator()(ComponentStorage& storage) {
    auto provider = [](Args... args) {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    };
    using RealRegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, decltype(provider)>;
    RealRegisterFactoryOperation()(storage);
  }
};

template <typename Comp, typename AnnotatedSignature>
struct RegisterConstructorAsPointerFactory {
  using RequiredSignature = ConstructSignature<std::unique_ptr<SignatureType<AnnotatedSignature>>, RequiredArgsForAssistedFactory<AnnotatedSignature>>;
  using RegisterFactoryOperation = RegisterFactory<Comp, AnnotatedSignature, RequiredSignature>;
  using Comp1 = typename RegisterFactoryOperation::Result;
  using Result = Comp1;
  void operator()(ComponentStorage& storage) {
    RegisterConstructorAsPointerFactoryHelper<Comp, AnnotatedSignature, RequiredSignature>()(storage);
  };
};

template <typename Comp, typename OtherComp>
struct InstallComponent {
  FruitDelegateCheck(DuplicatedTypesInComponentError<set_intersection<typename OtherComp::Ps, typename Comp::Ps>>);
  using new_Ps = concat_lists<typename OtherComp::Ps, typename Comp::Ps>;
  using new_Rs = set_difference<set_union<typename OtherComp::Rs, typename Comp::Rs>, new_Ps>;
  using new_Deps = AddDeps<typename OtherComp::Deps, typename Comp::Deps, typename Comp::Ps>;
  using new_Bindings = set_union<typename OtherComp::Bindings, typename Comp::Bindings>;
  using Comp1 = ConsComp<new_Rs, new_Ps, new_Deps, new_Bindings>;
  using Result = Comp1;
  void operator()(ComponentStorage& storage, ComponentStorage&& otherStorage) {
    storage.install(std::move(otherStorage));
  }
};

template <typename DestComp, typename SourceComp>
struct ConvertComponent {
  // We need to register:
  // * All the types provided by the new component
  // * All the types required by the old component
  // except:
  // * The ones already provided by the old component.
  // * The ones required by the new one.
  using ToRegister = set_difference<set_union<typename DestComp::Ps, typename SourceComp::Rs>,
                                    set_union<typename DestComp::Rs, typename SourceComp::Ps>>;
  using Helper = EnsureProvidedTypes<SourceComp, typename DestComp::Rs, ToRegister>;
  
  // Not needed, just double-checking.
  // Uses FruitStaticAssert instead of FruitDelegateCheck so that it's checked only in debug mode.
  FruitStaticAssert(true || sizeof(CheckComponentEntails<typename Helper::Result, DestComp>), "");
  
  void operator()(ComponentStorage& storage) {
    Helper()(storage);
  }
};

// The types in TargetRequirements will not be auto-registered.
template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegister;

template <typename Comp, typename TargetRequirements, bool has_inject_annotation, typename C>
struct AutoRegisterHelper {}; // Not used.

// C has an Inject typedef, use it.
template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegisterHelper<Comp, TargetRequirements, true, C> {
  using RegisterC = RegisterConstructor<Comp, typename GetInjectAnnotation<C>::Signature>;
  using Comp1 = typename RegisterC::Result;
  using RegisterArgs = EnsureProvidedTypes<Comp1, TargetRequirements, ExpandProvidersInParams<typename GetInjectAnnotation<C>::Args>>;
  using Comp2 = typename RegisterArgs::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    RegisterC()(storage);
    RegisterArgs()(storage);
  }
};

template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegisterHelper<Comp, TargetRequirements, false, C> {
  FruitDelegateCheck(NoBindingFoundError<C>);
};

template <typename Comp, typename TargetRequirements, bool has_binding, bool has_inject_annotation, typename C, typename... Args>
struct AutoRegisterFactoryHelper {}; // Not used.

// I has a binding, use it and look for a factory that returns the type that I is bound to.
template <typename Comp, typename TargetRequirements, bool unused, typename I, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, true, unused, std::unique_ptr<I>, Argz...> {
  using C = GetBinding<I, typename Comp::Bindings>;
  using AutoRegisterCFactory = EnsureProvidedTypes<Comp, TargetRequirements, List<std::function<std::unique_ptr<C>(Argz...)>>>;
  using Comp1 = typename AutoRegisterCFactory::Result;
  using BindFactory = RegisterProvider<Comp1, std::function<std::unique_ptr<I>(Argz...)>*(std::function<std::unique_ptr<C>(Argz...)>*)>;
  using Comp2 = typename BindFactory::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    AutoRegisterCFactory()(storage);
    auto provider = [](std::function<std::unique_ptr<C>(Argz...)>* fun) {
      return new std::function<std::unique_ptr<I>(Argz...)>([=](Argz... args) {
        C* c = (*fun)(args...).release();
        I* i = static_cast<I*>(c);
        return std::unique_ptr<I>(i);
      });
    };
    using RealBindFactory = RegisterProvider<Comp1, decltype(provider)>;
    static_assert(std::is_same<typename BindFactory::Result, typename RealBindFactory::Result>::value,
                  "Fruit bug, BindFactory and RealBindFactory out of sync.");
    RealBindFactory()(storage);
  }
};

// C doesn't have a binding as interface, nor an INJECT annotation.
// Bind std::function<unique_ptr<C>(Args...)> to std::function<C(Args...)>.
template <typename Comp, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, false, std::unique_ptr<C>, Argz...> {
  using AutoRegisterCFactory = EnsureProvidedTypes<Comp, TargetRequirements, List<std::function<C(Argz...)>>>;
  using Comp1 = typename AutoRegisterCFactory::Result;
  using BindFactory = RegisterProvider<Comp1, std::function<std::unique_ptr<C>(Argz...)>*(std::function<C(Argz...)>*)>;
  using Comp2 = typename BindFactory::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    AutoRegisterCFactory()(storage);
    auto provider = [](std::function<C(Argz...)>* fun) {
      return new std::function<std::unique_ptr<C>(Argz...)>([=](Argz... args) {
        C* c = new C((*fun)(args...));
        return std::unique_ptr<C>(c);
      });
    };
    using RealBindFactory = RegisterProvider<Comp1, decltype(provider)>;
    static_assert(std::is_same<typename BindFactory::Result, typename RealBindFactory::Result>::value,
                  "Fruit bug, BindFactory and RealBindFactory out of sync.");
    RealBindFactory()(storage);
  }
};

// C has an Inject typedef, use it. unique_ptr case.
// TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
template <typename Comp, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, true, std::unique_ptr<C>, Argz...> {
  using AnnotatedSignature = typename GetInjectAnnotation<C>::Signature;
  FruitDelegateCheck(CheckSameSignatureInInjectionTypedef<
    ConstructSignature<C, List<Argz...>>,
    ConstructSignature<C, RemoveNonAssisted<SignatureArgs<AnnotatedSignature>>>>);
  using NonAssistedArgs = RemoveAssisted<SignatureArgs<AnnotatedSignature>>;
  using RegisterC = RegisterConstructorAsPointerFactory<Comp, AnnotatedSignature>;
  using Comp1 = typename RegisterC::Result;
  using AutoRegisterArgs = EnsureProvidedTypes<Comp1, TargetRequirements, ExpandProvidersInParams<NonAssistedArgs>>;
  using Comp2 = typename AutoRegisterArgs::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    RegisterC()(storage);
    AutoRegisterArgs()(storage);
  }
};

// C has an Inject typedef, use it. Value (not unique_ptr) case.
// TODO: Doesn't work after renaming Argz->Args, consider minimizing the test case and filing a bug.
template <typename Comp, typename TargetRequirements, typename C, typename... Argz>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, true, C, Argz...> {
  using AnnotatedSignature = typename GetInjectAnnotation<C>::Signature;
  FruitDelegateCheck(CheckSameSignatureInInjectionTypedef<
    ConstructSignature<C, List<Argz...>>,
    ConstructSignature<C, RemoveNonAssisted<SignatureArgs<AnnotatedSignature>>>>);
  using NonAssistedArgs = RemoveAssisted<SignatureArgs<AnnotatedSignature>>;
  using RegisterC = RegisterConstructorAsValueFactory<Comp, AnnotatedSignature>;
  using Comp1 = typename RegisterC::Result;
  using AutoRegisterArgs = EnsureProvidedTypes<Comp1, TargetRequirements, ExpandProvidersInParams<NonAssistedArgs>>;
  using Comp2 = typename AutoRegisterArgs::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    RegisterC()(storage);
    AutoRegisterArgs()(storage);
  }
};

template <typename Comp, typename TargetRequirements, typename C, typename... Args>
struct AutoRegisterFactoryHelper<Comp, TargetRequirements, false, false, C, Args...> {
  FruitDelegateCheck(NoBindingFoundError<std::function<C(Args...)>>);
};

// Tries to registers C by looking for a typedef called Inject inside C.
template <typename Comp, typename TargetRequirements, typename C>
struct AutoRegister : public AutoRegisterHelper<
      Comp,
      TargetRequirements,
      HasInjectAnnotation<C>::value,
      C
>{};

template <typename Comp, typename TargetRequirements, typename C, typename... Args>
struct AutoRegister<Comp, TargetRequirements, std::function<C(Args...)>> : public AutoRegisterFactoryHelper<
      Comp,
      TargetRequirements,
      HasBinding<C, typename Comp::Bindings>::value,
      HasInjectAnnotation<C>::value,
      C,
      Args...
>{};

template <typename Comp, typename TargetRequirements, typename C, typename... Args>
struct AutoRegister<Comp, TargetRequirements, std::function<std::unique_ptr<C>(Args...)>> : public AutoRegisterFactoryHelper<
      Comp,
      TargetRequirements,
      HasBinding<C, typename Comp::Bindings>::value,
      false,
      std::unique_ptr<C>,
      Args...
>{};

template <typename Comp, typename TargetRequirements, bool is_already_provided_or_in_target_requirements, bool has_binding, typename C>
struct EnsureProvidedType {}; // Not used.

// Already provided or in target requirements, ok.
template <typename Comp, typename TargetRequirements, bool unused, typename C>
struct EnsureProvidedType<Comp, TargetRequirements, true, unused, C> : public Identity<Comp> {};  

// Has a binding.
template <typename Comp, typename TargetRequirements, typename I>
struct EnsureProvidedType<Comp, TargetRequirements, false, true, I> {
  using C = GetBinding<I, typename Comp::Bindings>;
  using Binder = BindNonFactory<Comp, I, C>;
  using Comp1 = typename Binder::Result;
  using EnsureImplProvided = EnsureProvidedTypes<Comp1, TargetRequirements, List<C>>;
  using Comp2 = typename EnsureImplProvided::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    Binder()(storage);
    EnsureImplProvided()(storage);
  }
};

// Not yet provided, nor in target requirements, nor in bindings. Try auto-registering.
template <typename Comp, typename TargetRequirements, typename C>
struct EnsureProvidedType<Comp, TargetRequirements, false, false, C> : public AutoRegister<Comp, TargetRequirements, C> {};

// General case, empty list.
template <typename Comp, typename TargetRequirements, typename L>
struct EnsureProvidedTypes : public Identity<Comp> {
  FruitStaticAssert(is_empty_list<L>::value, "Implementation error");
};

template <typename Comp, typename TargetRequirements, typename... Ts>
struct EnsureProvidedTypes<Comp, TargetRequirements, List<None, Ts...>> 
  : public EnsureProvidedTypes<Comp, TargetRequirements, List<Ts...>> {
};

template <typename Comp, typename TargetRequirements, typename T, typename... Ts>
struct EnsureProvidedTypes<Comp, TargetRequirements, List<T, Ts...>> {
  using C = GetClassForType<T>;
  using ProcessT = EnsureProvidedType<Comp,
    TargetRequirements,
    is_in_list<C, typename Comp::Ps>::value
    || is_in_list<C, TargetRequirements>::value,
    HasBinding<C, typename Comp::Bindings>::value,
    C>;
  using Comp1 = typename ProcessT::Result;
  using ProcessTs = EnsureProvidedTypes<Comp1, TargetRequirements, List<Ts...>>;
  using Comp2 = typename ProcessTs::Result;
  using Result = Comp2;
  void operator()(ComponentStorage& storage) {
    ProcessT()(storage);
    ProcessTs()(storage);
  }
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_COMPONENT_IMPL_H
