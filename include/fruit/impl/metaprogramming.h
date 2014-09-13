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

#include "fruit/fruit_forward_decls.h"
#include "fruit/impl/fruit_assert.h"
#include <functional>

namespace fruit {
namespace impl {

struct EmptyList {};

// Used to construct list-like types, no meaning per se.
template <typename H, typename T>
struct Cons {
  using Head = H;
  using Tail = T;
};

// Used to construct list-like types, no meaning per se.
template <typename... T>
struct FlatList {};

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

template <>
struct is_list<EmptyList> : std::true_type {};

template <typename H, typename T>
struct is_list<Cons<H, T>> {
  static constexpr bool value = is_list<T>::value;
};

// ****************************
// Implementation of concat_lists

template <typename L1, typename L2>
struct concat_lists_impl {}; // Not used

template <typename L2>
struct concat_lists_impl<EmptyList, L2> {
  using type = L2;
};

template <typename Head, typename Tail, typename L2>
struct concat_lists_impl<Cons<Head, Tail>, L2> {
  using type = Cons<Head, typename concat_lists_impl<Tail, L2>::type>;
};

template <typename L1, typename L2>
using concat_lists = typename concat_lists_impl<L1, L2>::type;

// ****************************
// Implementation of is_in_list

template <typename T, typename L>
struct is_in_list {}; // Not used

template <typename T>
struct is_in_list<T, EmptyList> : std::false_type {};

template <typename T, typename Tail>
struct is_in_list<T, Cons<T, Tail>> : std::true_type {};

template <typename T, typename Head, typename Tail>
struct is_in_list<T, Cons<Head, Tail>> {
  constexpr static bool value = is_in_list<T, Tail>::value;
};

// ****************************
// Implementation of is_empty_list

template <typename L>
struct is_empty_list : std::is_same<EmptyList, L> {};


// ****************************
// Implementation of add_to_set

template <bool is_in_set, typename T, typename L>
struct add_to_set_impl {}; // Not used.

// Not in the set, add.
template <typename T, typename L>
struct add_to_set_impl<false, T, L> {
  using type = Cons<T, L>;
};

// Already in the set, no-op.
template <typename T, typename L>
struct add_to_set_impl<true, T, L> {
  using type = L;
};

template <typename T, typename L>
using add_to_set = typename add_to_set_impl<is_in_list<T, L>::value, T, L>::type;

// *********************************
// Implementation of remove_from_set

template <typename T, typename L>
struct remove_from_set_impl {}; // Not used.

template <typename T>
struct remove_from_set_impl<T, EmptyList> {
  using type = EmptyList;
};

template <typename T, typename Tail>
struct remove_from_set_impl<T, Cons<T, Tail>> {
  using type = Tail;
};

template <typename T, typename Head, typename Tail>
struct remove_from_set_impl<T, Cons<Head, Tail>> {
  using type = Cons<Head, typename remove_from_set_impl<T, Tail>::type>;
};

template <typename T, typename L>
using remove_from_set = typename remove_from_set_impl<T, L>::type;

// ***********************************
// Implementation of set_intersection

template <bool head_is_in_l2, typename Head, typename Tail, typename L2>
struct set_intersection_impl_helper {}; // Not used

template <typename L1, typename L2>
struct set_intersection_impl {}; // Not used

template <typename Head, typename Tail, typename L2>
struct set_intersection_impl_helper<true, Head, Tail, L2> {
  using type = Cons<Head, typename set_intersection_impl<Tail, L2>::type>;
};

template <typename Head, typename Tail, typename L2>
struct set_intersection_impl_helper<false, Head, Tail, L2> {
  using type = typename set_intersection_impl<Tail, L2>::type;
};

template <typename L2>
struct set_intersection_impl<EmptyList, L2> {
  using type = EmptyList;
};

template <typename Head, typename Tail, typename L2>
struct set_intersection_impl<Cons<Head, Tail>, L2> {
  using type = typename set_intersection_impl_helper<is_in_list<Head, L2>::value, Head, Tail, L2>::type;
};

template <typename L1, typename L2>
using set_intersection = typename set_intersection_impl<L1, L2>::type;

// ***********************************
// Implementation of set_union

template <typename L1, typename L2>
struct set_union_impl {}; // Not used.

template <bool head_is_in_l2, typename Head, typename Tail, typename L2>
struct set_union_impl_helper {}; // Not used.

template <typename Head, typename Tail, typename L2>
struct set_union_impl_helper<false, Head, Tail, L2> {
  using type = Cons<Head, typename set_union_impl<Tail, L2>::type>;
};

template <typename Head, typename Tail, typename L2>
struct set_union_impl_helper<true, Head, Tail, L2> {
  using type = typename set_union_impl<Tail, L2>::type;
};

template <typename L2>
struct set_union_impl<EmptyList, L2> {
  using type = L2;
};

template <typename Head, typename Tail, typename L2>
struct set_union_impl<Cons<Head, Tail>, L2> {
  using type = typename set_union_impl_helper<is_in_list<Head, L2>::value, Head, Tail, L2>::type;
};

template <typename L1, typename L2>
using set_union = typename set_union_impl<L1, L2>::type;

// ****************************
// Implementation of replace_with_set

template <bool T_in_L, typename T, typename L1, typename L>
struct replace_with_set_helper {};

template <typename T, typename L1, typename L>
struct replace_with_set_helper<true, T, L1, L> {
  using type = set_union<L1, remove_from_set<T, L>>;
};

template <typename T, typename L1, typename L>
struct replace_with_set_helper<false, T, L1, L> {
  using type = L;
};

template <typename T, typename L1, typename L>
using replace_with_set = typename replace_with_set_helper<is_in_list<T, L>::value, T, L1, L>::type;

// ****************************
// Implementation of list_to_set

template <typename L>
struct list_to_set_impl {}; // Not used

template <bool head_is_in_tail_set, typename Head, typename TailSet>
struct list_to_set_helper {}; // Not used

template <typename Head, typename TailSet>
struct list_to_set_helper<false, Head, TailSet> {
  using type = Cons<Head, TailSet>;
};

template <typename Head, typename TailSet>
struct list_to_set_helper<true, Head, TailSet> {
  using type = TailSet;
};

template <>
struct list_to_set_impl<EmptyList> {
  using type = EmptyList;
};

template <typename Head, typename Tail>
struct list_to_set_impl<Cons<Head, Tail>> {
  using TailSet = typename list_to_set_impl<Tail>::type;
  using type = typename list_to_set_helper<is_in_list<Head, TailSet>::value, Head, TailSet>::type;
};

template <typename L>
using list_to_set = typename list_to_set_impl<L>::type;

// ***********************************
// Implementation of set_difference

template <typename L1, typename L2>
struct set_difference_impl {}; // Not used.

template <bool head_in_l2, typename Head, typename TailDifference>
struct set_difference_helper {}; // Not used.

template <typename Head, typename TailDifference>
struct set_difference_helper<false, Head, TailDifference> {
  using type = Cons<Head, TailDifference>;
};

template <typename Head, typename TailDifference>
struct set_difference_helper<true, Head, TailDifference> {
  using type = TailDifference;
};

template <typename L2>
struct set_difference_impl<EmptyList, L2> {
  using type = EmptyList;
};

template <typename Head, typename Tail, typename L2>
struct set_difference_impl<Cons<Head, Tail>, L2> {
  using TailDifference = typename set_difference_impl<Tail, L2>::type;
  using type = typename set_difference_helper<is_in_list<Head, L2>::value, Head, TailDifference>::type;
};

template <typename L1, typename L2>
using set_difference = typename set_difference_impl<L1, L2>::type;

// ****************************
// Implementation of add_to_flat_list

template <typename T, typename FlatL>
struct add_to_flat_list_impl {}; // Not used.

template <typename T, typename... Ts>
struct add_to_flat_list_impl<T, FlatList<Ts...>> {
  using type = FlatList<T, Ts...>;
};

template <typename T, typename FlatL>
using add_to_flat_list = typename add_to_flat_list_impl<T, FlatL>::type;

// ****************************
// Implementation of flatten_list

template <typename L>
struct flatten_list_impl {}; // Not used.

template <>
struct flatten_list_impl<EmptyList> {
  using type = FlatList<>;
};

template <typename Head, typename Tail>
struct flatten_list_impl<Cons<Head, Tail>> {
  using type = add_to_flat_list<Head, typename flatten_list_impl<Tail>::type>;
};

template <typename L>
using flatten_list = typename flatten_list_impl<L>::type;

// ****************************
// Implementation of unflatten_list

template <typename FlatL>
struct unflatten_list_impl {}; // Not used.

template <>
struct unflatten_list_impl<FlatList<>> {
  using type = EmptyList;
};

template <typename Head, typename... Tail>
struct unflatten_list_impl<FlatList<Head, Tail...>> {
  using type = Cons<Head, typename unflatten_list_impl<FlatList<Tail...>>::type>;
};

template <typename FlatL>
using unflatten_list = typename unflatten_list_impl<FlatL>::type;

//**********************************

template <typename T>
struct DebugTypeHelper {
  static_assert(sizeof(T*)*0 != 0, "");
  using type = T;
};

template <typename T>
using DebugType = typename DebugTypeHelper<T>::type;

template <typename C, typename FlatL>
struct is_constructible_with_flat_list {}; // Not used.

template <typename C, typename... Types>
struct is_constructible_with_flat_list<C, FlatList<Types...>> : public std::is_constructible<C, Types...> {};

template <typename Signature>
struct SignatureTraits {
  static_assert(false && sizeof(Signature), "Not a signature");
};

template <typename T, typename... Types>
struct SignatureTraits<T(Types...)> {
  using type = T;
  using Args = FlatList<Types...>;
};

template <typename Signature>
using SignatureType = typename SignatureTraits<Signature>::type;

template <typename Signature>
using SignatureFlatArgs = typename SignatureTraits<Signature>::Args;

template <typename T, typename FlatL>
struct ConstructSignatureImpl {};

template <typename T, typename... Types>
struct ConstructSignatureImpl<T, FlatList<Types...>> {
  using type = T(Types...);
};

template <typename T, typename FlatL>
using ConstructSignature = typename ConstructSignatureImpl<T, FlatL>::type;

template <typename FlatL>
struct flat_list_size {}; // Not used

template <typename... Ts>
struct flat_list_size<FlatList<Ts...>> {
  static constexpr int value = sizeof...(Ts);
};

template <typename L>
struct list_size {}; // Not used

template <>
struct list_size<EmptyList> {
  static constexpr int value = 0;
};

template <typename Head, typename Tail>
struct list_size<Cons<Head, Tail>> {
  static constexpr int value = 1 + list_size<Tail>::value;
};

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

template <typename T, typename Ts>
struct GetNthTypeHelper<0, Cons<T, Ts>> {
  using type = T;
};

template <int n, typename T, typename Ts>
struct GetNthTypeHelper<n, Cons<T, Ts>> : public GetNthTypeHelper<n-1, Ts> {};

template <int n, typename L>
using GetNthType = typename GetNthTypeHelper<n, L>::type;

template <typename MethodSignature>
struct FunctorResultHelper {};

template <typename Result, typename Functor, typename... Args>
struct FunctorResultHelper<Result(Functor::*)(Args...)> {
  using type = Result;
};

template <typename F>
using FunctorResult = typename FunctorResultHelper<decltype(&F::operator())>::type;

template <typename LambdaMethod>
struct FunctionSignatureHelper2 {};

template <typename Result, typename LambdaObject, typename... Args>
struct FunctionSignatureHelper2<Result(LambdaObject::*)(Args...) const> {
  using type = Result(Args...);
};

template <typename Function>
struct FunctionSignatureHelper {
  using type = typename FunctionSignatureHelper2<decltype(&Function::operator())>::type;
  FruitDelegateCheck(fruit::impl::FunctorUsedAsProvider<type, Function>);
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
