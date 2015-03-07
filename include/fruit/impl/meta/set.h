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

#ifndef FRUIT_META_SET_H
#define FRUIT_META_SET_H

#include "vector.h"

/*

Types and operations provided by this header:

AddToSet<T, S>           : adds T to S. If T is in S already, this is a no-op and S is returned.
SetVectorUnion<S, V>     : adds all the elements of the vector V to the set S.
VectorToSet<V>           : returns a set containing all the elements in V.
SetIntersection<S1, S2>  : returns the intersection of the given sets.
SetUnion<S1, S2>         : returns the union of the given sets.
SetDifference<S1, S2>    : returns the set of elements that are in S1 but not in S2.
IsSameSet<S1, S2>        : true if S1 and S2 represent the same set.
VectorOfSetsUnion<V>     : returns the union of all sets in the vector V.

Other operations provided by vector.h that can be used for sets:

Vector<Ts...>            : constructs a set with the specified elements. The elements must be distinct (except None, that can
                           appear any number of times).
IsInVector<S, T>         : true if T appears at least once in S.
IsEmptyVector<S>         : true if S is empty (i.e. if all the elements are None)
VectorSize<S>            : the number of (non-None) elements of the set (as an int).
VectorApparentSize<S>    : the number of elements of the set *including* any None elements.
RemoveFromVector<S, T>   : returns a set equivalent to S but with T removed (if it was there at all).

*/

namespace fruit {
namespace impl {
namespace meta {

struct AddToSet {
  template <typename T, typename V>
  struct apply {
    using type = Eval<Conditional<Lazy<Apply<IsInVector, T, V>>,
                                  Lazy<V>,
                                  Apply<LazyFunctor<PushFront>, Lazy<V>, Lazy<T>>
                                  >>;
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

struct SetVectorUnion {
  template <typename S, typename V>
  struct apply {
    using type = S;
  };

  template <typename S, typename... Ts>
  struct apply<S, Vector<Ts...>> {
    using type = Apply<AddToSetMultiple, S, Ts...>;
  };
};

struct VectorToSet {
  template <typename V>
  struct apply {
    using type = Apply<SetVectorUnion, Vector<>, V>;
  };
};

struct SetDifference {
  template <typename S1, typename S2>
  struct apply;

  template <typename... Ts, typename S>
  struct apply<Vector<Ts...>, S> {
    using type = Vector<Eval<std::conditional<Apply<IsInVector, Ts, S>::value, None, Ts>>...>;
  };
};

struct SetIntersection {
  template <typename S1, typename S2>
  struct apply;

  template <typename... Ts, typename S>
  struct apply<Vector<Ts...>, S> {
    using type = Vector<Eval<std::conditional<Apply<IsInVector, Ts, S>::value, Ts, None>>...>;
  };
};

struct SetUnion {
  template <typename S1, typename S2>
  struct apply {
    using type = Apply<ConcatVectors, Apply<SetDifference, S1, S2>, S2>;
  };
};

struct IsSameSet {
  template <typename S1, typename S2>
  struct apply {
    using type = Bool<   Apply<IsEmptyVector, Apply<SetDifference, S1, S2>>::value
                      && Apply<IsEmptyVector, Apply<SetDifference, S2, S1>>::value>;
  };
};

struct MultipleSetsUnion {
  template <typename... Sets>
  struct apply {
    using type = Vector<>;
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

struct VectorOfSetsUnion {
  template <typename V>
  struct apply {}; // Not used.

  template <typename... Sets>
  struct apply<Vector<Sets...>> {
    using type = Apply<MultipleSetsUnion, Sets...>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_SET_H
