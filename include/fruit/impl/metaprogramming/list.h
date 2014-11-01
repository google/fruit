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

#ifndef FRUIT_METAPROGRAMMING_LIST_H
#define FRUIT_METAPROGRAMMING_LIST_H

#include "logical_operations.h"
#include <functional>

/*

Types and operations provided by this header:

List<Ts...>                  : constructs a List with the specified elements.
None                         : a List element that should be ignored (all transformations should map it to itself).
is_list<L>                   : true if L is a List.
is_in_list<L, T>             : true if T appears at least once in L.
is_empty_list<L>             : true if L is empty (i.e. if all the elements are None)
list_size<L>                 : the number of (non-None) elements of the list (as an int).
list_apparent_size<L>        : the number of elements of the list *including* any None elements.
add_to_list<T, L>            : adds T in front of L.
concat_lists<L1, L2>         : returns the concatenation of the given lists.
concat_multiple_lists<Ls...> : extension of concat_lists that works with an arbitrary number of lists. Use sparingly, it's quite
                               slow.
transform_list<F, L>         : returns the list obtained by applying F to each element of L. F<None>::type must be None.
remove_from_list<L, T>       : returns a list equivalent to L but with all the occurrences of T replaced with None.

*/

namespace fruit {
namespace impl {

// Used to pass around a List<Types...>, no meaning per se.
template <typename... Types>
struct List {};

// None elements in a list are just placeholders (put in place of real elements when removing an element) and should be ignored.
struct None {};

template <typename T>
struct is_list : std::false_type {};

template <typename... Ts>
struct is_list<List<Ts...>> : std::true_type {};

template <typename T, typename L>
struct is_in_list {}; // Not used

template <typename T, typename... Ts>
struct is_in_list<T, List<Ts...>> {
  static constexpr bool value = static_or<std::is_same<T, Ts>::value...>::value;
};

template <typename L>
struct is_empty_list {}; // Not used.

template <typename... Ts>
struct is_empty_list<List<Ts...>> {
  static constexpr bool value = static_and<std::is_same<Ts, None>::value...>::value;
};

template <typename L>
struct list_size {}; // Not used.

template <typename... Ts>
struct list_size<List<Ts...>> {
  static constexpr int value = static_sum<!std::is_same<Ts, None>::value...>::value;
};

template <typename L>
struct list_apparent_size {}; // Not used.

template <typename... Ts>
struct list_apparent_size<List<Ts...>> {
  static constexpr int value = sizeof...(Ts);
};

template <typename T, typename L>
struct add_to_list_impl {}; // Not used.

template <typename T, typename... Ts>
struct add_to_list_impl<T, List<Ts...>> {
  using type = List<T, Ts...>;
};

template <typename T, typename L>
using add_to_list = typename add_to_list_impl<T, L>::type;

template <typename L1, typename L2>
struct concat_lists_impl {}; // Not used.

template <typename... Ts, typename... Us>
struct concat_lists_impl<List<Ts...>, List<Us...>> {
  using type = List<Ts..., Us...>;
};

template <typename L1, typename L2>
using concat_lists = typename concat_lists_impl<L1, L2>::type;

template <typename... L>
struct concat_multiple_lists_impl {}; // Not used

template <>
struct concat_multiple_lists_impl<> {
  using type = List<>;
};

template <typename L>
struct concat_multiple_lists_impl<L> {
  using type = L;
};

template <typename... Ts, typename... Us, typename... Ls>
struct concat_multiple_lists_impl<List<Ts...>, List<Us...>, Ls...> {
  using type = typename concat_multiple_lists_impl<List<Ts..., Us...>, Ls...>::type;
};

template <typename... Ls>
using concat_multiple_lists = typename concat_multiple_lists_impl<Ls...>::type;

// TODO: Consider removing (inlining) this op, it's not convenient to use.
template <template <typename X> class Functor, typename L>
struct transform_list_impl {}; // Not used

template <template <typename X> class Functor, typename... Ts>
struct transform_list_impl<Functor, List<Ts...>> {
  using type = List<typename Functor<Ts>::type...>;
};

template <template <typename X> class Functor, typename L>
using transform_list = typename transform_list_impl<Functor, L>::type;


// TODO: This is slow when T is not in the list, consider doing a is_in_list check first.

template <typename T, typename L>
struct remove_from_list_impl {}; // Not used.

template <typename T, typename... Ts>
struct remove_from_list_impl<T, List<Ts...>> {
  using type = List<
    typename std::conditional<std::is_same<T, Ts>::value, None, Ts>::type
    ...>;
};

template <typename T, typename L>
using remove_from_list = typename remove_from_list_impl<T, L>::type;




/*
// ****************************
// Implementation of transform_list_1_to_1


// ****************************
// Implementation of transform_list_1_to_many

template <template <typename X> class Functor, typename L>
struct transform_list_1_to_many_impl {}; // Not used

template <template <typename X> class Functor, typename... Ts>
struct transform_list_1_to_many_impl<Functor, List<Ts...>> {
  using type = concat_lists<typename Functor<Ts>::type...>;
};

template <template <typename X> class Functor, typename L>
using transform_list_1_to_many = typename transform_list_1_to_many_impl<Functor, L>::type;

*/

} // namespace impl
} // namespace fruit


#endif // FRUIT_METAPROGRAMMING_LIST_H
