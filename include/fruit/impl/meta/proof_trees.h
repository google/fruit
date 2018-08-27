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

#ifndef FRUIT_META_PROOF_TREES_H
#define FRUIT_META_PROOF_TREES_H

#include <fruit/impl/fruit_assert.h>
#include <fruit/impl/injection_debug_errors.h>
#include <fruit/impl/meta/errors.h>
#include <fruit/impl/meta/graph.h>
#include <fruit/impl/meta/set.h>

namespace fruit {
namespace impl {
namespace meta {

// Given a set of formulas Hps=Set<Hp1, ... Hp(n)> and a formula Th, Pair<Th, Hps> represents the
// following proof tree:
//
// Hp1 ... Hp(n)
// -------------
//      Th
//
// Hp1, ... Hp(n) must be distinct.
// Formulas are atomic, any type can be used as formula (except None).
//
// A proof forest is a map (i.e. a Set of Pair(s)) from each Th to the corresponding set of Hps.
// Note that a proof forest doesn't need to have any additional property (for example, a proof tree
// might contain the thesis as hypotheses, or there might be a longer loop e.g A=>B, B=>A.
using EmptyProofForest = EmptySet;

#if !FRUIT_NO_LOOP_CHECK

using ProofForestFindHps = GraphFindNeighbors;

// ProofForestFindLoop(F) returns a loop in the given forest as a Vector<Th1, ..., Thk> such that:
// IsInSet(Th1, ProofForestFindHps(Th2)), IsInSet(Th2, ProofForestFindHps(Th3)), ...,
//     IsInSet(Th{k-1}, ProofForestFindHps(Thk)), IsInSet(Thk, ProofForestFindHps(Th1))
// if there is no such loop, returns None.
using ProofForestFindLoop = GraphFindLoop;

#else // FRUIT_NO_LOOP_CHECK

struct ProofForestFindLoop {
  template <typename F>
  struct apply {
    using type = None;
  };
};

struct AddProofTreeToForest {
  template <typename Forest, typename Proof>
  struct apply {
    using type = Vector<>;
  };
};

struct AddProofTreeSetToForest {
  template <typename Proofs, typename Forest>
  struct apply {
    using type = Vector<>;
  };
};

#endif // FRUIT_NO_LOOP_CHECK

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_PROOF_TREES_H
