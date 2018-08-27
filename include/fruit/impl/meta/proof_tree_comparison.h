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

#ifndef FRUIT_PROOF_TREE_COMPARISON_H
#define FRUIT_PROOF_TREE_COMPARISON_H

#if !FRUIT_EXTRA_DEBUG && !FRUIT_IN_META_TEST
#error "This file should only be included in debug mode or in tests."
#endif

namespace fruit {
namespace impl {
namespace meta {

// Checks whether Proof is entailed by Forest, i.e. whether there is a corresponding Proof1 in Forest with the same
// thesis
// and with the same hypotheses as Proof (or a subset).
struct IsProofEntailedByForest {
  template <typename ProofTh, typename ProofHps, typename Forest>
  struct apply {
    using ForestHps = FindInMap(Forest, ProofTh);
    using type = If(IsNone(ForestHps), Bool<false>, IsContained(ForestHps, ProofHps));
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
        using type = And(CurrentResult, IsProofEntailedByForest(EntailedProofTh, EntailedProofHps, Forest));
      };
    };

    using type = FoldVector(EntailedForest, Helper, Bool<true>);
  };
};

// Given two proof trees, check if they are equal.
// Only for debugging.
struct IsProofTreeEqualTo {
  template <typename Proof1, typename Proof2>
  struct apply {
    using type = And(IsSame(typename Proof1::First, typename Proof2::First),
                     IsSameSet(typename Proof1::Second, typename Proof2::Second));
  };
};

// Given two proofs forests, check if they are equal.
// This is not very efficient, consider re-implementing if it will be used often.
// Only for debugging.
struct IsForestEqualTo {
  template <typename Forest1, typename Forest2>
  struct apply {
    using type = And(IsForestEntailedByForest(Forest1, Forest2), IsForestEntailedByForest(Forest2, Forest1));
  };
};

// Only for debugging, similar to checking IsProofEntailedByForest but gives a detailed error.
// Only for debugging.
struct CheckProofEntailedByForest {
  template <typename ProofTh, typename ProofHps, typename Forest>
  struct apply {
    using ForestHps = FindInMap(Forest, ProofTh);
    using type = If(IsNone(ForestHps),
                    ConstructError(ProofNotEntailedByForestBecauseThNotFoundErrorTag, ProofTh, GetMapKeys(Forest)),
                    If(IsContained(ForestHps, ProofHps), Bool<true>,
                       ConstructError(ProofNotEntailedByForestBecauseHpsNotASubsetErrorTag, ForestHps, ProofHps,
                                      SetDifference(ForestHps, ProofHps))));
  };
};

// Only for debugging, similar to checking IsProofEntailedByForest but gives a detailed error.
// Only for debugging.
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

// Given two proofs forests, check if they are equal.
// This is not very efficient, consider re-implementing if it will be used often.
// Only for debugging.
struct CheckForestEqualTo {
  template <typename Forest1, typename Forest2>
  struct apply {
    using type = PropagateError(CheckForestEntailedByForest(Forest1, Forest2),
                                CheckForestEntailedByForest(Forest2, Forest1));
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_PROOF_TREE_COMPARISON_H
