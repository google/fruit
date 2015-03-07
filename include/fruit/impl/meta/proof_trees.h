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
struct ConsProofTree {
  using Hps = Hps1;
  using Th = Th1;
};

// A proof forest is a Vector of ConsProofTree<> elements, with unique theses, and where the theses never appear as hypotheses.
using EmptyProofForest = Vector<>;

// Removes the specified Hp from the proof tree.
struct RemoveHpFromProofTree {
  template <typename Hp, typename Proof>
  struct apply {
    using type = ConsProofTree<Apply<RemoveFromVector, Hp, typename Proof::Hps>,
                               typename Proof::Th>;
  };
};

// Removes the specified Hp from all proofs in the forest.
struct RemoveHpFromProofForest {
  template <typename Hp, typename Forest>
  struct apply;
  
  template <typename Hp, typename... Proofs>
  struct apply<Hp, Vector<Proofs...>> {
    using type = Vector<Apply<RemoveHpFromProofTree, Hp, Proofs>...>;
  };
};

#ifndef FRUIT_NO_LOOP_CHECK

// Constructs a proof tree with the given hypotheses and thesis.
// The hypotheses don't have to be distinct. If you know that they are, use ConsProofTree instead.
struct ConstructProofTree {
  template <typename Hps, typename Th>
  struct apply {
    using type = ConsProofTree<Apply<VectorToSet, Hps>, Th>;
  };
};

// Constructs a proof forest tree with the given theses and where all theses have the same (given) hypotheses.
// The hypotheses don't have to be distinct. If you know that they are, use ConsProofTree instead.
struct ConstructProofForest {
  template <typename Hps, typename... Ths>
  struct apply {
    using HpsSet = Apply<VectorToSet, Hps>;
    using type = Vector<ConsProofTree<HpsSet, Ths>...>;
  };
};

// Checks if the given proof tree has the thesis among its hypotheses.
struct HasSelfLoop {
  template <typename Proof>
  struct apply {
    using type = Apply<IsInVector, typename Proof::Th, typename Proof::Hps>;
  };
};

struct CombineForestHypothesesWithProofHelper {
  template <typename Proof, typename NewProof>
  struct apply {
    using type = ConsProofTree<Apply<SetUnion,
                                     typename NewProof::Hps,
                                     Apply<RemoveFromVector, typename NewProof::Th, typename Proof::Hps>>,
                               typename Proof::Th>;
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
    using type = Vector<Eval<Conditional<Lazy<Apply<IsInVector, typename NewProof::Th, typename Proofs::Hps>>,
                                         Apply<LazyFunctor<CombineForestHypothesesWithProofHelper>, Lazy<Proofs>, Lazy<NewProof>>,
                                         Lazy<Proofs>
                                         >>
                        ...>;
  };
};

struct CombineProofHypothesesWithForestHelper {
  template <typename Hps, typename Forest>
  struct apply;

  template <typename Hps, typename... Proofs>
  struct apply<Hps, Vector<Proofs...>> {
    using type = Vector<Eval<std::conditional<Apply<IsInVector, typename Proofs::Th, Hps>::value,
                                              typename Proofs::Hps,
                                              Vector<>>>...>;
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
    using type = ConsProofTree<
      Apply<SetUnion,
        Apply<VectorOfSetsUnion,
          Apply<CombineProofHypothesesWithForestHelper, typename Proof::Hps, Forest>
        >,
        Apply<SetDifference,
          typename Proof::Hps,
          ForestThs
        >
      >,
      typename Proof::Th>;
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
    FruitStaticAssert(Apply<IsSameSet, Apply<ForestTheses, Forest>, ForestThs>::value, "");
    using NewProof = Apply<CombineProofHypothesesWithForest, Proof, Forest, ForestThs>;
    // Note that NewProof might have its own thesis as hypothesis.
    // At this point, no hypotheses of NewProof appear as theses of Forest. A single replacement step is sufficient.
    using type = Eval<std::conditional<Apply<HasSelfLoop, NewProof>::value,
                                       None,
                                       Apply<PushFront,
                                             Apply<CombineForestHypothesesWithProof, Forest, NewProof>,
                                             NewProof>
                                       >>;
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
    using type = Apply<AddProofTreesToForest,
                       Apply<AddProofTreeToForest, Proof, Forest, ForestThs>,
                       Apply<PushFront, ForestThs, typename Proof::Th>,
                       Proofs...>;
  };
};

struct AddProofTreeVectorToForest {
  template <typename Proofs, typename Forest, typename ForestThs>
  struct apply {
    using type = ApplyWithVector<AddProofTreesToForest, Proofs, Forest, ForestThs>;
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
  struct apply<Th, ConsProofTree<Hps, Th>, Proofs...> {
    using type = ConsProofTree<Hps, Th>;
  };
  
  template <typename Th, typename Th1, typename Hps1, typename... Proofs>
  struct apply<Th, ConsProofTree<Hps1, Th1>, Proofs...> {
    using type = Apply<FindProofInProofs, Th, Proofs...>;
  };
};

// Returns the proof with the given thesis in Forest, or None if there isn't one.
struct FindProofInForest {
  template <typename Th, typename Forest>
  struct apply {
    using type = ApplyWithVector<FindProofInProofs, Forest, Th>;
  };
};

struct IsProofEntailedByForestHelper {
  template <typename Proof, typename Proof1>
  struct apply {
    using type = Apply<IsEmptyVector, Apply<SetDifference, typename Proof1::Hps, typename Proof::Hps>>;
  };
};

// Checks whether Proof is entailed by Forest, i.e. whether there is a corresponding Proof1 in Forest with the same thesis
// and with the same hypotheses as Proof (or a subset).
struct IsProofEntailedByForest {
  template <typename Proof, typename Forest>
  struct apply {
    using Proof1 = Apply<FindProofInForest, typename Proof::Th, Forest>;
    using type = Eval<Conditional<Lazy<Bool<std::is_same<Proof1, None>::value>>,
                                  Lazy<Bool<false>>,
                                  Apply<LazyFunctor<IsProofEntailedByForestHelper>, Lazy<Proof>, Lazy<Proof1>>>>;
  };
};

struct IsForestEntailedByForest {
  template <typename EntailedForest, typename Forest>
  struct apply;

  template <typename... EntailedProofs, typename Forest>
  struct apply<Vector<EntailedProofs...>, Forest> {
    using type = Bool<StaticAnd<Apply<IsProofEntailedByForest, EntailedProofs, Forest>::value...>::value>;
  };
};

// Given two proof trees, check if they are equal.
struct IsProofTreeEqualTo {
  template <typename Proof1, typename Proof2>
  struct apply {
    using type = Bool<std::is_same<typename Proof1::Th, typename Proof2::Th>::value
                      && Apply<IsSameSet, typename Proof1::Hps, typename Proof2::Hps>::value>;
  };
};

// Given two proofs forests, check if they are equal.
// This is not very efficient, consider re-implementing if it will be used often.
struct IsForestEqualTo {
  template <typename Forest1, typename Forest2>
  struct apply {
    using type = Bool<   Apply<IsForestEntailedByForest, Forest1, Forest2>::value
                      && Apply<IsForestEntailedByForest, Forest2, Forest1>::value>;
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
