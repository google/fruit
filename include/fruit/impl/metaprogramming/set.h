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

AddToSet<T, S>           : adds T to S. If T is in S already, this is a no-op and S is returned.
SetListUnion<S, L>       : adds all the elements of the list L to the set S.
ListToSet<L>             : returns a set containing all the elements in L.
SetIntersection<S1, S2>  : returns the intersection of the given sets.
SetUnion<S1, S2>         : returns the union of the given sets.
SetDifference<S1, S2>    : returns the set of elements that are in S1 but not in S2.
IsSameSet<S1, S2>        : true if S1 and S2 represent the same set.
ReplaceWithSet<S, T, S1> : if T is in S, returns (S - T + S1). Otherwise, returns S.
ListOfSetsUnion<L>       : returns the union of all sets in the list L.

Other operations provided by list.h that can be used for sets:

// TODO: Provide aliases in this file, List is an implementation detail of Set.
List<Ts...>              : constructs a set with the specified elements. The elements must be distinct (except None, that can
                           appear any number of times).
IsInList<S, T>           : true if T appears at least once in S.
IsEmptyList<S>           : true if S is empty (i.e. if all the elements are None)
ListSize<S>              : the number of (non-None) elements of the set (as an int).
ListApparentSize<S>      : the number of elements of the set *including* any None elements.
RemoveFromList<S, T>     : returns a set equivalent to S but with T removed (if it was there at all).

*/

namespace fruit {
namespace impl {
  
struct AddToSet {
  template <typename T, typename L>
  struct apply {
    using type = Conditional<ApplyC<IsInList, T, L>::value,
                             Lazy<L>,
                             LazyApply<AddToList, T, L>>;
  };
};

struct AddToSetMultiple {
  template <typename S, typename... Ts>
  struct apply {
    using type = S;
  };

  template <typename S, typename T1, typename... Ts>
  struct apply<S, T1, Ts...> {
    using type = Apply<AddToSet,
                       T1,
                       Apply<AddToSetMultiple, S, Ts...>>;
  };
};

struct SetListUnion {
  template <typename S, typename L>
  struct apply {
    using type = S;
  };

  template <typename S, typename... Ts>
  struct apply<S, List<Ts...>> {
    using type = Apply<AddToSetMultiple, S, Ts...>;
  };
};

struct ListToSet {
  template <typename L>
  struct apply {
    using type = Apply<SetListUnion, List<>, L>;
  };
};

struct SetDifference {
  template <typename S1, typename S2>
  struct apply;

  template <typename... Ts, typename S>
  struct apply<List<Ts...>, S> {
    using type = List<Eval<std::conditional<ApplyC<IsInList, Ts, S>::value, None, Ts>>...>;
  };
};

struct SetIntersection {
  template <typename S1, typename S2>
  struct apply;

  template <typename... Ts, typename S>
  struct apply<List<Ts...>, S> {
    using type = List<Eval<std::conditional<ApplyC<IsInList, Ts, S>::value, Ts, None>>...>;
  };
};

struct SetUnion {
  template <typename S1, typename S2>
  struct apply {
    using type = Apply<ConcatLists, Apply<SetDifference, S1, S2>, S2>;
  };
};

struct IsSameSet {
  template <typename S1, typename S2>
  struct apply {
    static constexpr bool value = ApplyC<IsEmptyList, Apply<SetDifference, S1, S2>>::value
                               && ApplyC<IsEmptyList, Apply<SetDifference, S2, S1>>::value;
  };
};

struct ReplaceWithSetHelper {
  template <typename S, typename T, typename S1>
  struct apply {
    using type = Apply<SetUnion, S1, Apply<RemoveFromList, T, S>>;
  };
};

struct ReplaceWithSet {
  template <typename S, typename T, typename S1>
  struct apply {
    using type = Conditional<ApplyC<IsInList, T, S>::value,
                             LazyApply<ReplaceWithSetHelper, S, T, S1>,
                             Lazy<S>>;
  };
};

struct MultipleSetsUnion {
  template <typename... Sets>
  struct apply {
    using type = List<>;
  };

  template <typename Set>
  struct apply<Set> {
    using type = Set;
  };

  template <typename Set, typename Set2, typename... Sets>
  struct apply<Set, Set2, Sets...> {
    using type = Apply<SetUnion,
                       Apply<SetUnion, Set, Set2>,
                       Apply<MultipleSetsUnion, Sets...>>;
  };
};

struct ListOfSetsUnion {
  template <typename L>
  struct apply {}; // Not used.

  template <typename... Sets>
  struct apply<List<Sets...>> {
    using type = Apply<MultipleSetsUnion, Sets...>;
  };
};

} // namespace impl
} // namespace fruit


#endif // FRUIT_METAPROGRAMMING_SET_H
