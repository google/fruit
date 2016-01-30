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
#include <fruit/impl/meta/set.h>
#include <fruit/impl/meta/graph.h>
#include <fruit/impl/meta/errors.h>

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

#ifndef FRUIT_NO_LOOP_CHECK

// Constructs a proof forest with the given theses and where all theses have the same (given)
// hypotheses.
struct ConstructProofForest {
  template <typename Hps, typename... Ths>
  struct apply {
    using type = Vector<Pair<Ths, Hps>...>;
  };
};

using ProofForestFindHps = GraphFindNeighbors;

// ProofForestFindLoop(F) returns a loop in the given forest as a Vector<Th1, ..., Thk> such that:
// IsInSet(Th1, ProofForestFindHps(Th2)), IsInSet(Th2, ProofForestFindHps(Th3)), ...,
//     IsInSet(Th{k-1}, ProofForestFindHps(Thk)), IsInSet(Thk, ProofForestFindHps(Th1))
// if there is no such loop, returns None.
using ProofForestFindLoop = GraphFindLoop;

#if defined(FRUIT_EXTRA_DEBUG) || defined(FRUIT_IN_META_TEST)

// Checks whether Proof is entailed by Forest, i.e. whether there is a corresponding Proof1 in Forest with the same thesis
// and with the same hypotheses as Proof (or a subset).
struct IsProofEntailedByForest {
  template <typename ProofTh, typename ProofHps, typename Forest>
  struct apply {
    using ForestHps = FindInMap(Forest, ProofTh);
    using type = If(IsNone(ForestHps),
                    Bool<false>,
                 IsContained(ForestHps, ProofHps));
  };
};


// Only for debugging, similar to checking IsProofEntailedByForest but gives a detailed error.
struct CheckProofEntailedByForest {
  template <typename ProofTh, typename ProofHps, typename Forest>
  struct apply {
    using ForestHps = FindInMap(Forest, ProofTh);
    using type = If(IsNone(ForestHps),
                    ConstructError(ProofNotEntailedByForestBecauseThNotFoundErrorTag, ProofTh, GetMapKeys(Forest)),
                 If(IsContained(ForestHps, ProofHps),
                    Bool<true>,
                    ConstructError(ProofNotEntailedByForestBecauseHpsNotASubsetErrorTag, ForestHps, ProofHps, SetDifference(ForestHps, ProofHps))));
  };
};

struct IsForestEntailedByForest {
  template <typename EntailedForest, typename Forest>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename EntailedProof>
      struct apply;
      
      template <typename CurrentResult, typename EntailedProofTh, typename EntailedProofHps>
      struct apply<CurrentResult, Pair<EntailedProofTh, EntailedProofHps>> {
        using type = And(CurrentResult,
                         IsProofEntailedByForest(EntailedProofTh, EntailedProofHps, Forest));
      };
    };
    
    using type = FoldVector(EntailedForest, Helper, Bool<true>);
  };
};

// Only for debugging, similar to checking IsProofEntailedByForest but gives a detailed error.
struct CheckForestEntailedByForest {
  template <typename EntailedForest, typename Forest>
  struct apply {
    struct Helper {
      template <typename CurrentResult, typename EntailedProof>
      struct apply;

      template <typename CurrentResult, typename EntailedProofTh, typename EntailedProofHps>
      struct apply<CurrentResult, Pair<EntailedProofTh, EntailedProofHps>> {
        using type = PropagateError(CurrentResult,
                     CheckProofEntailedByForest(EntailedProofTh, EntailedProofHps, Forest));
      };
    };
    
    using type = FoldVector(EntailedForest, Helper, Bool<true>);
  };
};

// Given two proof trees, check if they are equal.
struct IsProofTreeEqualTo {
  template <typename Proof1, typename Proof2>
  struct apply {
    using type = And(IsSame(typename Proof1::First, typename Proof2::First),
                     IsSameSet(typename Proof1::Second, typename Proof2::Second));
  };
};

// Given two proofs forests, check if they are equal.
// This is not very efficient, consider re-implementing if it will be used often.
struct IsForestEqualTo {
  template <typename Forest1, typename Forest2>
  struct apply {
    using type = And(IsForestEntailedByForest(Forest1, Forest2),
                     IsForestEntailedByForest(Forest2, Forest1));
  };
};

// Given two proofs forests, check if they are equal.
// This is not very efficient, consider re-implementing if it will be used often.
struct CheckForestEqualTo {
  template <typename Forest1, typename Forest2>
  struct apply {
    using type = PropagateError(CheckForestEntailedByForest(Forest1, Forest2),
                                CheckForestEntailedByForest(Forest2, Forest1));
  };
};

#endif // defined(FRUIT_EXTRA_DEBUG) || defined(FRUIT_IN_META_TEST)
#else // FRUIT_NO_LOOP_CHECK

struct ProofForestFindLoop {
  template <typename F>
  struct apply {
    using type = None;
  };
};

struct ConstructProofForest {
  template <typename Rs, typename... P>
  struct apply {
    using type = Vector<>;
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
