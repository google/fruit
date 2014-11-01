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

#include "basics.h"
#include "logical_operations.h"
#include <functional>

/*

Types and operations provided by this header:

List<Ts...>                : constructs a List with the specified elements.
None                       : a List element that should be ignored (all transformations should map it to itself).
IsList<L>                  : true if L is a List.
IsInList<L, T>             : true if T appears at least once in L.
IsEmptyList<L>             : true if L is empty (i.e. if all the elements are None)
ListSize<L>                : the number of (non-None) elements of the list (as an int).
ListApparentSize<L>        : the number of elements of the list *including* any None elements.
AddToList<T, L>            : adds T in front of L.
ConcatLists<L1, L2>        : returns the concatenation of the given lists.
ConcatMultipleLists<Ls...> : extension of concat_lists that works with an arbitrary number of lists. Use sparingly, it's quite
                             slow.
TransformList<F, L>        : returns the list obtained by applying F to each element of L. F<None>::type must be None.
RemoveFromList<L, T>       : returns a list equivalent to L but with all the occurrences of T replaced with None.

*/

namespace fruit {
namespace impl {

// Used to pass around a List<Types...>, no meaning per se.
template <typename... Types>
struct List {};

// None elements in a list are just placeholders (put in place of real elements when removing an element) and should be ignored.
struct None {};

struct IsList {
  template <typename T>
  struct apply : std::false_type {};

  template <typename... Ts>
  struct apply<List<Ts...>> : std::true_type {};
};

struct IsInList {
  template <typename T, typename L>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<T, List<Ts...>> {
    static constexpr bool value = StaticOr<std::is_same<T, Ts>::value...>::value;
  };
};

struct IsEmptyList {
  template <typename L>
  struct apply;

  template <typename... Ts>
  struct apply<List<Ts...>> {
    static constexpr bool value = StaticAnd<std::is_same<Ts, None>::value...>::value;
  };
};

struct ListSize {
  template <typename L>
  struct apply;

  template <typename... Ts>
  struct apply<List<Ts...>> {
    static constexpr int value = StaticSum<!std::is_same<Ts, None>::value...>::value;
  };
};

struct ListApparentSize {
  template <typename L>
  struct apply;

  template <typename... Ts>
  struct apply<List<Ts...>> {
    static constexpr int value = sizeof...(Ts);
  };
};

struct AddToList {
  template <typename T, typename L>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<T, List<Ts...>> {
    using type = List<T, Ts...>;
  };
};

struct ConcatLists {
  template <typename L1, typename L2>
  struct apply;

  template <typename... Ts, typename... Us>
  struct apply<List<Ts...>, List<Us...>> {
    using type = List<Ts..., Us...>;
  };
};

struct ConcatMultipleLists {
  // Empty list.
  template <typename... L>
  struct apply {
    using type = List<>;
  };

  template <typename L>
  struct apply<L> {
    using type = L;
  };

  template <typename... Ts, typename... Us, typename... Ls>
  struct apply<List<Ts...>, List<Us...>, Ls...> {
    using type = Apply<ConcatMultipleLists, List<Ts..., Us...>, Ls...>;
  };
};

struct ApplyWithListHelper {
  template <typename F, typename L, typename... Args>
  struct apply;
  
  template <typename F, typename... Elems, typename... Args>
  struct apply<F, List<Elems...>, Args...> {
    using type = Apply<F, Args..., Elems...>;
  };
};

template <typename F, typename L, typename... Args>
using ApplyWithList = Apply<ApplyWithListHelper, F, L, Args...>;

// TODO: This is slow when T is not in the list, consider doing a is_in_list check first.
struct RemoveFromList {
  template <typename T, typename L>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<T, List<Ts...>> {
    using type = List<
      Eval<std::conditional<std::is_same<T, Ts>::value, None, Ts>>
      ...>;
  };
};

} // namespace impl
} // namespace fruit


#endif // FRUIT_METAPROGRAMMING_LIST_H
