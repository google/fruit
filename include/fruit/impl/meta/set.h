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

#include <fruit/impl/fruit_assert.h>
#include <fruit/impl/meta/immutable_set.h>
#include <fruit/impl/meta/pair.h>
#include <fruit/impl/meta/vector.h>

namespace fruit {
namespace impl {
namespace meta {

// Set ::= Vector<Ts...>, with no duplicates.

using EmptySet = Vector<>;

template <typename T>
using ToSet1 = Vector<T>;

template <typename T, typename U>
using ToSet2 = Vector<T, U>;

using IsInSet = IsInVector;

// If S is a set with elements (T1, ..., Tn) this calculates
// F(InitialValue, F(T1, F(..., F(Tn) ...))).
// If S is EmptySet this returns InitialValue.
using FoldSet = FoldVector;

// If S is a set with elements (T1, ..., Tn) this calculates
// Combine(F(T1), Combine(F(T2),..., F(Tn) ...)).
//
// `Combine' must be associative, and CombineIdentity must be an identity value wrt Combine.
// Use this instead of FoldSet when possible, it shares more sub-instances when invoked multiple
// times with similar sets.
struct FoldSetWithCombine {
  template <typename S, typename F, typename Combine, typename CombineIdentity>
  struct apply {
    using type = FoldVector(TransformVector(S, F), Combine, CombineIdentity);
  };
};

// Converts a set (T1,...,Tn) to a set (F(T1), ..., F(Tn)).
// F(T1), ..., F(Tn) must be distinct.
using TransformSet = TransformVector;

using SetSize = VectorSize;

using AddToSetUnchecked = PushFront;

struct AddToSet {
  template <typename S, typename T>
  struct apply {
    using type = If(IsInSet(T, S), S, AddToSetUnchecked(S, T));
  };
};

// Checks if S1 is contained in S2.
struct IsContained {
  template <typename S1, typename S2>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename T>
      struct apply {
        using type = And(CurrentResult, IsInSet(T, S2));
      };
    };

    using type = FoldVector(S1, Helper, Bool<true>);
  };
};

// Checks if S1 is disjoint from S2.
struct IsDisjoint {
  template <typename S1, typename S2>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename T>
      struct apply {
        using type = Or(CurrentResult, IsInSet(T, S2));
      };
    };

    using type = Not(FoldVector(S1, Helper, Bool<false>));
  };
};

struct IsEmptySet {
  template <typename S>
  struct apply {
    using type = Bool<false>;
  };
};

template <>
struct IsEmptySet::apply<Vector<>> {
  using type = Bool<true>;
};

using SetToVector = Identity;

// The vector must have no duplicates.
using VectorToSetUnchecked = Identity;

struct SetDifference {
  template <typename S1, typename S2>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename T>
      struct apply {
        using type = If(IsInSet(T, S2), CurrentResult, AddToSetUnchecked(CurrentResult, T));
      };
    };

    using type = FoldSet(S1, Helper, EmptySet);
  };
};

struct SetIntersection {
  template <typename S1, typename S2>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename T>
      struct apply {
        using type = If(IsInSet(T, S2), AddToSetUnchecked(CurrentResult, T), CurrentResult);
      };
    };

    using type = If(GreaterThan(SetSize(S1), SetSize(S2)), SetIntersection(S2, S1), FoldSet(S1, Helper, EmptySet));
  };
};

struct SetUnion {
  template <typename S1, typename S2>
  struct apply {
    using type = If(GreaterThan(SetSize(S1), SetSize(S2)), SetUnion(S2, S1),
                    FoldSet(SetDifference(S1, S2), AddToSetUnchecked, S2));
  };
};

using SetUncheckedUnion = ConcatVectors;

struct IsSameSet {
  template <typename S1, typename S2>
  struct apply {
    using type = And(IsContained(S1, S2), IsContained(S2, S1));
  };
};

// Returns an arbitrary element from the given set (that must be non-empty).
struct GetArbitrarySetElement {
  template <typename S>
  struct apply;

  template <typename T, typename... Ts>
  struct apply<Vector<T, Ts...>> {
    using type = T;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_SET_H
