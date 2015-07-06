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

#include "../fruit_assert.h"
#include "set.h"

namespace fruit {
namespace impl {
namespace meta {

// Given a set of formulas Hps=Vector<Hp1, ... Hp(n)> and a formula Th, ConsProofTree<Hps, Th> represents the following proof
// tree:
// 
// Hp1 ... Hp(n)
// -------------
//      Th
// 
// Hp1, ... Hp(n) must be distinct.
// Formulas are atomic, any type can be used as formula (except None).
template <typename Hps1, typename Th1>
struct ProofTree {
  using Hps = Hps1;
  using Th = Th1;
};

// Using ConsProofTree(MetaExpr1, MetaExpr2) instead of ProofTree<MetaExpr1, MetaExpr2> in a
// meta-expression allows the types to be evaluated. Avoid using ProofTree<...> directly in a
// meta-expression, unless you're sure that the arguments have already been evaluated (e.g. if T, U
// are arguments of a metafunction, ProofTree<T, U> is ok but ProofTree<MyFunction(T), U> is
// wrong.
struct ConsProofTree {
  template <typename Hps1, typename Th1>
  struct apply {
    using type = ProofTree<Hps1, Th1>;
  };
};

// A proof forest is a Vector of ConsProofTree<> elements, with unique theses, and where the theses never appear as hypotheses.
using EmptyProofForest = Vector<>;

// Removes the specified Hp from the proof tree.
struct RemoveHpFromProofTree {
  template <typename Hp, typename Proof>
  struct apply {
    using type = ConsProofTree(RemoveFromVector(Hp, typename Proof::Hps),
                               typename Proof::Th);
  };
};

// Removes the specified Hp from all proofs in the forest.
struct RemoveHpFromProofForest {
  template <typename Hp, typename Forest>
  struct apply;
  
  template <typename Hp, typename... Proofs>
  struct apply<Hp, Vector<Proofs...>> {
    using type = ConsVector(Id<RemoveHpFromProofTree(Hp, Proofs)>
                            ...);
  };
};

#ifndef FRUIT_NO_LOOP_CHECK

// Constructs a proof tree with the given hypotheses and thesis.
// The hypotheses don't have to be distinct. If you know that they are, use ConsProofTree instead.
struct ConstructProofTree {
  template <typename Hps, typename Th>
  struct apply {
    using type = ConsProofTree(VectorToSet(Hps), Th);
  };
};

// Constructs a proof forest tree with the given theses and where all theses have the same (given) hypotheses.
// The hypotheses don't have to be distinct. If you know that they are, use ConsProofTree instead.
struct ConstructProofForest {
  template <typename Hps, typename... Ths>
  struct apply {
    using type = ConsVector(Id<ConsProofTree(VectorToSet(Hps), Ths)>
                            ...);
  };
};

// Checks if the given proof tree has the thesis among its hypotheses.
struct HasSelfLoop {
  template <typename Proof>
  struct apply {
    using type = IsInVector(typename Proof::Th, typename Proof::Hps);
  };
};

struct CombineForestHypothesesWithProofHelper {
  template <typename Proof, typename NewProof>
  struct apply {
    using type = ConsProofTree(SetUnion(typename NewProof::Hps,
                                        RemoveFromVector(typename NewProof::Th, typename Proof::Hps)),
                               typename Proof::Th);
  };
};

// Combines `Proof' with the hypotheses in the given proof forest. If the thesis of Proof is among the hypotheses of a proof in
// the tree, it is removed and Proof's hypotheses are added in its place.
// Returns the modified forest.
struct CombineForestHypothesesWithProof {
  template <typename Forest, typename NewProof>
  struct apply;

  template <typename... Proofs, typename NewProof>
  struct apply<Vector<Proofs...>, NewProof> {
    using type = ConsVector(Id<If(IsInVector(typename NewProof::Th, typename Proofs::Hps),
                               CombineForestHypothesesWithProofHelper(Proofs, NewProof),
                               Proofs)>
                            ...);
  };
};

struct CombineProofHypothesesWithForestHelper {
  template <typename Hps, typename Forest>
  struct apply;

  template <typename Hps, typename... Proofs>
  struct apply<Hps, Vector<Proofs...>> {
    using type = ConsVector(Id<If(IsInVector(typename Proofs::Th, Hps),
                               typename Proofs::Hps,
                               Vector<>)>
                            ...);
  };
};

// Combines Forest into Proof by replacing each theses of Forest that is an hypothesis in Proof with its hypotheses in Forest.
// Returns the modified proof.
// ForestThs is also passed as parameter, but just as an optimization.
// TODO: VectorOfSetsUnion is slow, consider optimizing here to avoid it. E.g. we could try finding each Hp of Proof in Forest, and
//       then do the union of the results.
// TODO: See if ForestThs improves performance, if not remove it.
struct CombineProofHypothesesWithForest {
  template <typename Proof, typename Forest, typename ForestThs>
  struct apply {
    using Hps = SetUnion(
        VectorOfSetsUnion(
          CombineProofHypothesesWithForestHelper(typename Proof::Hps, Forest)),
        SetDifference(typename Proof::Hps, ForestThs));
    using type = ConsProofTree(Hps, typename Proof::Th);
  };
};

struct ForestTheses {
  template <typename Forest>
  struct apply;
  
  template <typename... Proof>
  struct apply<Vector<Proof...>> {
    using type = Vector<typename Proof::Th...>;
  };
};

// Adds Proof to Forest. If doing so would create a loop (a thesis depending on itself), returns None instead.
struct AddProofTreeToForest {
  template <typename Proof, typename Forest, typename ForestThs>
  struct apply {
    FruitStaticAssert(IsSameSet(ForestTheses(Forest), ForestThs));
    using NewProof = CombineProofHypothesesWithForest(Proof, Forest, ForestThs);
    // Note that NewProof might have its own thesis as hypothesis.
    // At this point, no hypotheses of NewProof appear as theses of Forest. A single replacement step is sufficient.
    using type = If(HasSelfLoop(NewProof),
                    None,
                    PushFront(CombineForestHypothesesWithProof(Forest, NewProof),
                              NewProof));
  };
};

struct AddProofTreesToForest {
  // Case with empty Proofs....
  template <typename Forest, typename ForestThs, typename... Proofs>
  struct apply {
    using type = Forest;
  };

  template <typename Forest, typename ForestThs, typename Proof, typename... Proofs>
  struct apply<Forest, ForestThs, Proof, Proofs...> {
    using type = AddProofTreesToForest(
                       AddProofTreeToForest(Proof, Forest, ForestThs),
                       PushFront(ForestThs, typename Proof::Th),
                       Proofs...);
  };
};

struct AddProofTreeVectorToForest {
  template <typename Proofs, typename Forest, typename ForestThs>
  struct apply {
    using type = CallWithVector(Call(DeferArgs(AddProofTreesToForest), Forest, ForestThs), Proofs);
  };
};

// Returns the proof with the given thesis in Proofs..., or None if there isn't one.
struct FindProofInProofs {
  // Case with empty Proofs...
  template <typename Th, typename... Proofs>
  struct apply {
    using type = None;
  };
  
  template <typename Th, typename Hps, typename... Proofs>
  struct apply<Th, ProofTree<Hps, Th>, Proofs...> {
    using type = ProofTree<Hps, Th>;
  };
  
  template <typename Th, typename Th1, typename Hps1, typename... Proofs>
  struct apply<Th, ProofTree<Hps1, Th1>, Proofs...> {
    using type = FindProofInProofs(Th, Proofs...);
  };
};

// Returns the proof with the given thesis in Forest, or None if there isn't one.
struct FindProofInForest {
  template <typename Th, typename Forest>
  struct apply {
    using type = CallWithVector(Call(DeferArgs(FindProofInProofs), Th), Forest);
  };
};

struct IsProofEntailedByForestHelper {
  template <typename Proof, typename Proof1>
  struct apply {
    using type = IsEmptyVector(SetDifference(typename Proof1::Hps, typename Proof::Hps));
  };
};

// Checks whether Proof is entailed by Forest, i.e. whether there is a corresponding Proof1 in Forest with the same thesis
// and with the same hypotheses as Proof (or a subset).
struct IsProofEntailedByForest {
  template <typename Proof, typename Forest>
  struct apply {
    using Proof1 = FindProofInForest(typename Proof::Th, Forest);
    using type = If(IsSame(Proof1, None),
                    Bool<false>,
                    IsProofEntailedByForestHelper(Proof, Proof1));
  };
};

struct IsForestEntailedByForest {
  template <typename EntailedForest, typename Forest>
  struct apply;

  template <typename... EntailedProofs, typename Forest>
  struct apply<Vector<EntailedProofs...>, Forest> {
    using type = And(Id<IsProofEntailedByForest(EntailedProofs, Forest)>
                     ...);
  };
};

// Given two proof trees, check if they are equal.
struct IsProofTreeEqualTo {
  template <typename Proof1, typename Proof2>
  struct apply {
    using type = And(IsSame(typename Proof1::Th, typename Proof2::Th),
                     IsSameSet(typename Proof1::Hps, typename Proof2::Hps));
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

#else // FRUIT_NO_LOOP_CHECK

struct ConstructProofTree {
  template <typename P, typename Rs>
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
  template <typename Proof, typename Forest, typename ForestThs>
  struct apply {
    using type = Vector<>;
  };
};

struct AddProofTreeVectorToForest {
  template <typename Proofs, typename Forest, typename ForestThs>
  struct apply {
    using type = Vector<>;
  };
};

#endif // FRUIT_NO_LOOP_CHECK

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_PROOF_TREES_H
