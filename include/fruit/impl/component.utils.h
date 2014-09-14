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

#ifndef FRUIT_COMPONENT_UTILS_H
#define FRUIT_COMPONENT_UTILS_H

#include "../fruit_forward_decls.h"
#include "fruit_assert.h"

#include <memory>

namespace fruit {

namespace impl {

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

template <typename Signature>
struct IsValidSignature : std::false_type {};

template <typename T, typename... Args>
struct IsValidSignature<T(Args...)> : public static_and<!is_list<T>::value, !is_list<Args>::value...> {};

template <typename L>
struct ExtractRequirementsFromAssistedParamsHelper {};

template <>
struct ExtractRequirementsFromAssistedParamsHelper<List<>> {
  using type = List<>;
};

// Assisted case
template <typename T, typename... Ts>
struct ExtractRequirementsFromAssistedParamsHelper<List<Assisted<T>, Ts...>> {
  using type = typename ExtractRequirementsFromAssistedParamsHelper<List<Ts...>>::type;
};

// Non-assisted case
template <typename T, typename... Ts>
struct ExtractRequirementsFromAssistedParamsHelper<List<T, Ts...>> {
  using type = add_to_list<GetClassForType<T>, typename ExtractRequirementsFromAssistedParamsHelper<List<Ts...>>::type>;
};

// Takes a list of types, considers only the assisted ones, transforms them to classes with
// GetClassForType and returns the resulting list.
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

template <typename L>
struct UnlabelAssistedHelper {};

template <>
struct UnlabelAssistedHelper<List<>> {
  using type = List<>;
};

// Non-assisted case
template <typename T, typename... Ts>
struct UnlabelAssistedHelper<List<T, Ts...>> {
  using type = add_to_list<T, typename UnlabelAssistedHelper<List<Ts...>>::type>;
};

// Assisted case
template <typename T, typename... Ts>
struct UnlabelAssistedHelper<List<Assisted<T>, Ts...>> {
  using type = add_to_list<T, typename UnlabelAssistedHelper<List<Ts...>>::type>;
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
    static_assert(std::is_same<C, SignatureType<S>>::value, "The Inject typedef is not of the form C(Args...). Maybe C inherited an Inject annotation from the base class by mistake?");
    static_assert(is_constructible_with_list<C, UnlabelAssisted<A>>::value, "C contains an Inject annotation but it's not constructible with the specified types"); // Tested
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
struct RemoveRequirementFromDepsHelper {
  static_assert(false && sizeof(C*), "");
};

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
using CanonicalizeDepRequirementsWithDep = replace_with_set<typename D1::Type, typename D1::Requirements, Requirements>;

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

template <typename Dep, typename Deps>
using CanonicalizeDepWithDeps = ConsDep<typename Dep::Type, typename CanonicalizeDepRequirementsWithDeps<typename Dep::Requirements, Deps>::type>;

template <typename Dep, typename Deps>
struct AddDepHelper {
  using CanonicalizedDep = CanonicalizeDepWithDeps<Dep, Deps>;
  FruitDelegateCheck(CheckHasNoSelfLoop<!HasSelfLoop<CanonicalizedDep>::value, typename CanonicalizedDep::Type>);
  // At this point CanonicalizedDep doesn't have as arguments any types appearing as heads in Deps,
  // but the head of CanonicalizedDep might appear as argument of some Deps.
  // A single replacement step is sufficient.
  using type = add_to_list<CanonicalizedDep, typename CanonicalizeDepsWithDep<Deps, CanonicalizedDep>::type>;
};

template <typename Dep, typename Deps>
using AddDep = typename AddDepHelper<Dep, Deps>::type;

template <typename Deps, typename OtherDeps>
struct AddDepsHelper {};

template <typename... OtherDeps>
struct AddDepsHelper<List<>, List<OtherDeps...>> {
  using type = List<OtherDeps...>;
};

template <typename Dep, typename... Deps, typename... OtherDeps>
struct AddDepsHelper<List<Dep, Deps...>, List<OtherDeps...>> {
  using recursion_result = typename AddDepsHelper<List<Deps...>, List<OtherDeps...>>::type;
  using type = AddDep<Dep, recursion_result>;
};

template <typename Deps, typename OtherDeps>
using AddDeps = typename AddDepsHelper<Deps, OtherDeps>::type;

template <typename D, typename Deps>
struct CheckDepEntailed {
  static_assert(false && sizeof(D), "bug! should never instantiate this.");
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

template <typename L>
struct ExpandProvidersInParamsHelper {};

template <>
struct ExpandProvidersInParamsHelper<List<>> {
  using type = List<>;
};

// Non-empty list, T is not of the form Provider<Ts...>
template <typename T, typename... OtherTs>
struct ExpandProvidersInParamsHelper<List<T, OtherTs...>> {
  using recursion_result = typename ExpandProvidersInParamsHelper<List<OtherTs...>>::type;
  using type = add_to_list<T, recursion_result>;
};

// Non-empty list, type of the form Provider<Ts...>
template <typename... Ts, typename... OtherTs>
struct ExpandProvidersInParamsHelper<List<fruit::Provider<Ts...>, OtherTs...>> {
  using recursion_result = typename ExpandProvidersInParamsHelper<List<OtherTs...>>::type;
  using type = concat_lists<List<Ts...>, recursion_result>;
};

template <typename L>
using ExpandProvidersInParams = typename ExpandProvidersInParamsHelper<L>::type;

template <typename I, typename Bindings>
struct HasBinding {};

template <typename I>
struct HasBinding<I, List<>> {
  static constexpr bool value = false;
};

template <typename I, typename C, typename... Bindings>
struct HasBinding<I, List<ConsBinding<I, C>, Bindings...>> {
  static constexpr bool value = true;
};

template <typename I, typename I2, typename C, typename... Bindings>
struct HasBinding<I, List<ConsBinding<I2, C>, Bindings...>> {
  static constexpr bool value = HasBinding<I, List<Bindings...>>::value;
};

template <typename I, typename Bindings>
struct GetBindingHelper {};

template <typename I, typename C, typename... Bindings>
struct GetBindingHelper<I, List<ConsBinding<I, C>, Bindings...>> {
  using type = C;
};

template <typename I, typename I2, typename C, typename... Bindings>
struct GetBindingHelper<I, List<ConsBinding<I2, C>, Bindings...>> {
  using type = typename GetBindingHelper<I, List<Bindings...>>::type;
};

template <typename I, typename Bindings>
using GetBinding = typename GetBindingHelper<I, Bindings>::type;

static inline void nopDeleter(void*) {
}

template <typename T>
static inline void standardDeleter(void* p) {
  T* t = reinterpret_cast<T*>(p);
  delete t;
}

template <typename Signature>
struct ConstructorFactoryValueProviderHelper {};

template <typename C, typename... Args>
struct ConstructorFactoryValueProviderHelper<C(Args...)> {
  static C f(Args... args) {
    static_assert(!std::is_pointer<C>::value, "Error, C should not be a pointer");
    static_assert(std::is_constructible<C, Args...>::value, "Error, C should be constructible with Args...");
    return C(std::forward<Args>(args)...);
  }
};

template <typename Signature>
struct ConstructorFactoryValueProvider : public ConstructorFactoryValueProviderHelper<Signature> {};

template <typename Signature>
struct ConstructorFactoryPointerProviderHelper {};

template <typename C, typename... Args>
struct ConstructorFactoryPointerProviderHelper<C(Args...)> {
  static std::unique_ptr<C> f(Args... args) {
    static_assert(!std::is_pointer<C>::value, "Error, C should not be a pointer");
    static_assert(std::is_constructible<C, Args...>::value, "Error, C should be constructible with Args...");
    return std::unique_ptr<C>(std::forward<Args>(args)...);
  }
};

template <typename Signature>
struct ConstructorFactoryPointerProvider : public ConstructorFactoryPointerProviderHelper<Signature> {};


} // namespace impl
} // namespace fruit


#endif // FRUIT_COMPONENT_UTILS_H
