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
    using type = If(IsInVector(T, V),
                    V,
                    PushFront(V, T));
  };
};

struct AddToSetMultiple {
  template <typename S, typename... Ts>
  struct apply {
    using type = S;
  };

  template <typename S, typename T1, typename... Ts>
  struct apply<S, T1, Ts...> {
    using type = AddToSet(T1, AddToSetMultiple(S, Ts...));
  };
};

struct SetVectorUnion {
  template <typename S, typename V>
  struct apply {
    using type = S;
  };

  template <typename S, typename... Ts>
  struct apply<S, Vector<Ts...>> {
    using type = AddToSetMultiple(S, Ts...);
  };
};

struct VectorToSet {
  template <typename V>
  struct apply {
    using type = SetVectorUnion(Vector<>, V);
  };
};

struct SetDifference {
  template <typename S1, typename S2>
  struct apply;

  template <typename... Ts, typename S>
  struct apply<Vector<Ts...>, S> {
    using type = ConsVector(Id<If(IsInVector(Ts, S), None, Ts)>
                            ...);
  };
};

struct SetIntersection {
  template <typename S1, typename S2>
  struct apply;

  template <typename... Ts, typename S>
  struct apply<Vector<Ts...>, S> {
    using type = ConsVector(Id<If(IsInVector(Ts, S), Ts, None)>
                            ...);
  };
};

struct SetUnion {
  template <typename S1, typename S2>
  struct apply {
    using type = ConcatVectors(SetDifference(S1, S2), S2);
  };
};

struct IsSameSet {
  template <typename S1, typename S2>
  struct apply {
    using type = And(IsEmptyVector(SetDifference(S1, S2)),
                     IsEmptyVector(SetDifference(S2, S1)));
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
    using type = SetUnion(SetUnion(Set, Set2),
                          MultipleSetsUnion(Sets...));
  };
};

// TODO: We could remove this and just use CallWithVector+MultipleSetsUnion in the caller.
struct VectorOfSetsUnion {
  template <typename V>
  struct apply;

  template <typename... Sets>
  struct apply<Vector<Sets...>> {
    using type = MultipleSetsUnion(Sets...);
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_SET_H
