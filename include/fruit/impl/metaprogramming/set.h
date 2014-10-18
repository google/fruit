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

#ifndef FRUIT_METAPROGRAMMING_SET_H
#define FRUIT_METAPROGRAMMING_SET_H

#include "list.h"

/*

Types and operations provided by this header:

add_to_set<T, S>             : adds T to S. If T is in S already, this is a no-op and S is returned.
set_list_union<S, L>         : adds all the elements of the list L to the set S.
list_to_set<L>               : returns a set containing all the elements in L.
set_intersection<S1, S2>     : returns the intersection of the given sets.
set_union<S1, S2>            : returns the union of the given sets.
set_difference<S1, S2>       : returns the set of elements that are in S1 but not in S2.
is_same_set<S1, S2>          : true if S1 and S2 represent the same set.
replace_with_set<S, T, S1>   : if T is in S, returns (S - T + S1). Otherwise, returns S.
list_of_sets_union<L>        : returns the union of all sets in the list L.

Other operations provided by list.h that can be used for sets:

List<Ts...>                  : constructs a set with the specified elements. The elements must be distinct (except None, that can
                               appear any number of times).
is_in_list<S, T>             : true if T appears at least once in S.
is_empty_list<S>             : true if S is empty (i.e. if all the elements are None)
list_size<S>                 : the number of (non-None) elements of the set (as an int).
list_apparent_size<S>        : the number of elements of the set *including* any None elements.
remove_from_list<S, T>       : returns a set equivalent to S but with T removed (if it was there at all).

*/

namespace fruit {
namespace impl {
  
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

template <typename S, typename... Ts>
struct add_to_set_multiple_impl {
  using type = S;
};

template <typename S, typename T1, typename... Ts>
struct add_to_set_multiple_impl<S, T1, Ts...> {
  using recursion_result = typename add_to_set_multiple_impl<S, Ts...>::type;
  using type = add_to_set<T1, recursion_result>;
};

template <typename S, typename... Ts>
using add_to_set_multiple = typename add_to_set_multiple_impl<S, Ts...>::type;

template <typename S, typename L>
struct set_list_union_impl {
  using type = S;
};

template <typename S, typename... Ts>
struct set_list_union_impl<S, List<Ts...>> {
  using type = add_to_set_multiple<S, Ts...>;
};

template <typename S, typename L>
using set_list_union = typename set_list_union_impl<S, L>::type;

template <typename L>
using list_to_set = set_list_union<List<>, L>;

template <typename S1, typename S2>
struct set_difference_impl {}; // Not used.

template <typename... Ts, typename S>
struct set_difference_impl<List<Ts...>, S> {
  using type = List<typename std::conditional<is_in_list<Ts, S>::value, None, Ts>::type...>;
};

template <typename S1, typename S2>
using set_difference = typename set_difference_impl<S1, S2>::type;

template <typename S1, typename S2>
struct set_intersection_impl {}; // Not used.

template <typename... Ts, typename S>
struct set_intersection_impl<List<Ts...>, S> {
  using type = List<typename std::conditional<is_in_list<Ts, S>::value, Ts, None>::type...>;
};

template <typename S1, typename S2>
using set_intersection = typename set_intersection_impl<S1, S2>::type;

template <typename S1, typename S2>
using set_union = concat_lists<set_difference<S1, S2>, S2>;

template <typename S1, typename S2>
struct is_same_set {
  static constexpr bool value = is_empty_list<set_difference<S1, S2>>::value
                             && is_empty_list<set_difference<S2, S1>>::value;
};

template <bool T_in_S, typename S, typename T, typename S1>
struct replace_with_set_helper {};

template <typename S, typename T, typename S1>
struct replace_with_set_helper<true, S, T, S1> {
  using type = set_union<S1, remove_from_list<T, S>>;
};

template <typename S, typename T, typename S1>
struct replace_with_set_helper<false, S, T, S1> {
  using type = S;
};

template <typename S, typename T, typename S1>
using replace_with_set = typename replace_with_set_helper<is_in_list<T, S>::value, S, T, S1>::type;

template <typename... Sets>
struct list_of_sets_union_helper {
  using type = List<>;
};

template <typename Set>
struct list_of_sets_union_helper<Set> {
  using type = Set;
};

template <typename Set, typename Set2, typename... Sets>
struct list_of_sets_union_helper<Set, Set2, Sets...> {
  using type = set_union<set_union<Set, Set2>, typename list_of_sets_union_helper<Sets...>::type>;
};

template <typename L>
struct list_of_sets_union_impl {}; // Not used.

template <typename... Sets>
struct list_of_sets_union_impl<List<Sets...>> {
  using type = typename list_of_sets_union_helper<Sets...>::type;
};

template <typename L>
using list_of_sets_union = typename list_of_sets_union_impl<L>::type;

} // namespace impl
} // namespace fruit


#endif // FRUIT_METAPROGRAMMING_SET_H
