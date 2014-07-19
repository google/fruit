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

#ifndef FRUIT_METAPROGRAMMING_H
#define FRUIT_METAPROGRAMMING_H

#include <functional>

namespace fruit {
namespace impl {

// Used to pass around a List<Types...>, no meaning per se.
template <typename... Types>
struct List {};

template <int...>
struct IntList {};

// General case: empty.
template <bool... bs>
struct static_and {
  static constexpr bool value = true;
};

template <bool b, bool... bs>
struct static_and<b, bs...> {
  static constexpr bool value = b && static_and<bs...>::value;  
};

template <bool... bs>
struct static_or {
  static constexpr bool value = !static_and<!bs...>::value;
};

// General case: nothing to sum.
template <int... is>
struct static_sum {
  static constexpr int value = 0;
};

template <int i, int... is>
struct static_sum<i, is...> {
  static constexpr int value = i + static_sum<is...>::value;
};

// ****************************
// Implementation of is_list

template <typename T>
struct is_list : std::false_type {};

template <typename... Ts>
struct is_list<List<Ts...>> : std::true_type {};

// ****************************
// Implementation of add_to_list

template <typename T, typename L>
struct add_to_list_impl {}; // Not used.

template <typename T, typename... Ts>
struct add_to_list_impl<T, List<Ts...>> {
  using type = List<T, Ts...>;
};

template <typename T, typename L>
using add_to_list = typename add_to_list_impl<T, L>::type;

// ****************************
// Implementation of concat_lists

template <typename... L>
struct concat_lists_impl {}; // Not used

template <>
struct concat_lists_impl<> {
  using type = List<>;
};

template <typename L>
struct concat_lists_impl<L> {
  using type = L;
};

template <typename... Ts, typename... Us, typename... Ls>
struct concat_lists_impl<List<Ts...>, List<Us...>, Ls...> {
  using type = typename concat_lists_impl<List<Ts..., Us...>, Ls...>::type;
};

template <typename... Ls>
using concat_lists = typename concat_lists_impl<Ls...>::type;

// *********************************
// Implementation of remove_from_list

template <typename T, typename L>
struct remove_from_list_impl {}; // Not used.

template <typename T, typename... Ts>
struct remove_from_list_impl<T, List<Ts...>> {
  using type = concat_lists<
    typename std::conditional<std::is_same<T, Ts>::value, List<>, List<Ts>>::type
    ...>;
};

template <typename T, typename L>
using remove_from_list = typename remove_from_list_impl<T, L>::type;

// ****************************
// Implementation of is_in_list

template <typename T, typename L>
struct is_in_list {}; // Not used

template <typename T, typename... Ts>
struct is_in_list<T, List<Ts...>> : static_or<std::is_same<T, Ts>::value...> {};

// ****************************
// Implementation of list_count

template <typename T, typename L>
struct list_count {}; // Not used

template <typename T, typename... Ts>
struct list_count<T, List<Ts...>> : static_sum<std::is_same<T, Ts>::value...> {};

// ****************************
// Implementation of is_empty_list

template <typename L>
struct is_empty_list : std::is_same<List<>, L> {};


// ****************************
// Implementation of add_to_set

template <bool is_in_set, typename T, typename L>
struct add_to_set_impl {}; // Not used.

// Not in the set, add.
template <typename T, typename L>
struct add_to_set_impl<false, T, L> {
  using type = add_to_list<T, L>;
};

// Already in the set, no-op.
template <typename T, typename L>
struct add_to_set_impl<true, T, L> {
  using type = L;
};

template <typename T, typename L>
using add_to_set = typename add_to_set_impl<is_in_list<T, L>::value, T, L>::type;

// ***********************************
// Implementation of are_sets_disjoint

template <typename L1, typename L2>
struct are_sets_disjoint {}; // Not used

template <typename... Ts, typename... Us>
struct are_sets_disjoint<List<Ts...>, List<Us...>> : static_and<is_in_list<Ts, List<Us...>>::value...> {};

// ***********************************
// Implementation of set_intersection

template <typename L1, typename L2>
struct set_intersection_helper {}; // Not used

template <typename... Ts, typename... Us>
struct set_intersection_helper<List<Ts...>, List<Us...>> {
  using type = concat_lists<
      typename std::conditional<is_in_list<Ts, List<Us...>>::value, List<Ts>, List<>>::type
    ...>;
};

template <typename T1, typename... T, typename... U>
struct set_intersection_helper<List<T1, T...>, List<U...>> {
  using recursion_result = typename set_intersection_helper<List<T...>, List<U...>>::type;
  using type = typename std::conditional<is_in_list<T1, List<U...>>::value,
                                         add_to_list<T1, recursion_result>,
                                         recursion_result>::type;
};

template <typename L1, typename L2>
using set_intersection = typename set_intersection_helper<L1, L2>::type;

// ***********************************
// Implementation of merge_sets

template <typename L1, typename L2>
struct merge_sets_impl {}; // Not used.

template <typename L>
struct merge_sets_impl<List<>, L> {
  using type = L;
};

template <typename T1, typename... T, typename L>
struct merge_sets_impl<List<T1, T...>, L> {
  using recursive_result = typename merge_sets_impl<List<T...>, L>::type;
  using type = add_to_set<T1, recursive_result>;
};

template <typename L1, typename L2>
using merge_sets = typename merge_sets_impl<L1, L2>::type;

// ****************************
// Implementation of replace_with_set

template <bool T_in_L, typename T, typename L1, typename L>
struct replace_with_set_helper {};

template <typename T, typename L1, typename L>
struct replace_with_set_helper<true, T, L1, L> {
  using type = merge_sets<L1, remove_from_list<T, L>>;
};

template <typename T, typename L1, typename L>
struct replace_with_set_helper<false, T, L1, L> {
  using type = L;
};

template <typename T, typename L1, typename L>
using replace_with_set = typename replace_with_set_helper<is_in_list<T, L>::value, T, L1, L>::type;

// ****************************
// Implementation of list_to_set

// Empty list.
template <typename L>
struct list_to_set_helper {
  using type = List<>;
};

template <typename T1, typename... T>
struct list_to_set_helper<List<T1, T...>> {
  using recursion_result = typename list_to_set_helper<List<T...>>::type;
  using type = add_to_set<T1, recursion_result>;
};

template <typename L>
using list_to_set = typename list_to_set_helper<L>::type;

// ***********************************
// Implementation of set_difference

template <typename L1, typename L2>
struct set_difference_impl {}; // Not used.

template <typename... Ts, typename... Us>
struct set_difference_impl<List<Ts...>, List<Us...>> {
  using type = concat_lists<
      typename std::conditional<is_in_list<Ts, List<Us...>>::value, List<>, List<Ts>>::type
      ...>;
};

template <typename L1, typename L2>
using set_difference = typename set_difference_impl<L1, L2>::type;

//**********************************

template <typename T>
struct DebugTypeHelper {
  static_assert(sizeof(T)*0 != 0, "");
  using type = T;
};

template <typename T>
using DebugType = typename DebugTypeHelper<T>::type;

template <typename C, typename L>
struct is_constructible_with_list {}; // Not used.

template <typename C, typename... Types>
struct is_constructible_with_list<C, List<Types...>> : public std::is_constructible<C, Types...> {};

template <typename Signature>
struct SignatureTraits {
  static_assert(sizeof(Signature), "Not a signature");
};

template <typename T, typename... Types>
struct SignatureTraits<T(Types...)> {
  using type = T;
  using Args = List<Types...>;
};

template <typename Signature>
using SignatureType = typename SignatureTraits<Signature>::type;

template <typename Signature>
using SignatureArgs = typename SignatureTraits<Signature>::Args;

template <typename T, typename L>
struct ConstructSignatureImpl {};

template <typename T, typename... Types>
struct ConstructSignatureImpl<T, List<Types...>> {
  using type = T(Types...);
};

template <typename T, typename L>
using ConstructSignature = typename ConstructSignatureImpl<T, L>::type;

template <typename Signature>
struct ConstructorProvider {};

template <typename C, typename... Args>
struct ConstructorProvider<C(Args...)> {
  static C* f(Args... args) {
    static_assert(!std::is_pointer<C>::value, "Error, C should not be a pointer");
    static_assert(std::is_constructible<C, Args...>::value, "Error, C should be constructible with Args...");
    return new C(std::forward<Args>(args)...);
  }
};

template <typename C>
struct SimpleDeleter {
  static void f(void* p) {
    C* c = reinterpret_cast<C*>(p);
    delete c;
  }
};

template <typename C>
struct ConcreteClassDeleter {
  static void f(void* p) {
    C* c = reinterpret_cast<C*>(p);
    // Use the concrete destructor. This gives a (likely negligible) performance gain since it skips the virtual method call
    // and also avoids compiler warnings when C has virtual methods but no virtual destructor.
    c->C::~C();
    operator delete(c);
  }
};

static void nopDeleter(void*) {
}

template <typename Signature>
struct ConstructorFactoryProvider {};

template <typename C, typename... Args>
struct ConstructorFactoryProvider<C(Args...)> {
  static C f(Args... args) {
    static_assert(!std::is_pointer<C>::value, "Error, C should not be a pointer");
    static_assert(std::is_constructible<C, Args...>::value, "Error, C should be constructible with Args...");
    return C(std::forward<Args>(args)...);
  }
};

template <typename Signature>
struct IsValidSignatureForNonSingleton : public std::integral_constant<bool, 
    !std::is_pointer<SignatureType<Signature>>::value
> {};

template <typename Signature>
struct IsValidSignatureForSingleton : public std::integral_constant<bool, 
    std::is_pointer<SignatureType<Signature>>::value
    && !std::is_pointer<typename std::remove_pointer<SignatureType<Signature>>::type>::value
> {};

template <typename L>
struct list_size {};

template <typename... Types>
struct list_size<List<Types...>> {
  static constexpr int value = sizeof...(Types);
};

template <template<typename T> class Op, typename L>
struct LiftHelper {};

template <template<typename T> class Op, typename... Ts>
struct LiftHelper<Op, List<Ts...>> {
  using type = List<Op<Ts>...>;
};

template <template<typename T> class Op, typename L>
using Lift = typename LiftHelper<Op, L>::type;

template <typename L>
struct AddPointerToListHelper {};

template <typename... Ts>
struct AddPointerToListHelper<List<Ts...>> {
  using type = List<Ts*...>;
};

template <typename L>
using AddPointerToList = typename AddPointerToListHelper<L>::type;

template <typename T>
struct IsFunctionType : public std::false_type {};

template <typename T, typename... Args>
struct IsFunctionType<std::function<T(Args...)>> : public std::true_type {};

template<int n, int... ns>
struct GenerateIntSequenceHelper : public GenerateIntSequenceHelper<n-1, n-1, ns...> {};

template<int... ns>
struct GenerateIntSequenceHelper<0, ns...> {
  typedef IntList<ns...> type;
};

template <int n>
using GenerateIntSequence = typename GenerateIntSequenceHelper<n>::type;

template <int n, typename L>
struct GetNthTypeHelper {};

template <typename T, typename... Ts>
struct GetNthTypeHelper<0, List<T, Ts...>> {
  using type = T;
};

template <int n, typename T, typename... Ts>
struct GetNthTypeHelper<n, List<T, Ts...>> : public GetNthTypeHelper<n-1, List<Ts...>> {};

template <int n, typename L>
using GetNthType = typename GetNthTypeHelper<n, L>::type;

template <typename F, typename... Args>
using FunctorResult = typename std::remove_reference<decltype(F()(std::declval<Args>()...))>::type;

template <typename LambdaMethod>
struct FunctionSignatureHelper2 {};

template <typename Result, typename LambdaObject, typename... Args>
struct FunctionSignatureHelper2<Result(LambdaObject::*)(Args...) const> {
  using type = Result(Args...);
};

template <typename Function>
struct FunctionSignatureHelper {
  using type = typename FunctionSignatureHelper2<decltype(&Function::operator())>::type;
};

template <typename Result, typename... Args>
struct FunctionSignatureHelper<Result(*)(Args...)> {
  using type = Result(Args...);
};

// Function is either a plain function type of the form T(*)(Args...) or a lambda.
template <typename Function>
using FunctionSignature = typename FunctionSignatureHelper<Function>::type;

} // namespace impl
} // namespace fruit


#endif // FRUIT_METAPROGRAMMING_H
