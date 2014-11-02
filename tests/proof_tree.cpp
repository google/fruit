// expect-success
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

#include <fruit/fruit.h>

using namespace fruit;
using namespace fruit::impl;

#ifndef FRUIT_NO_LOOP_CHECKz

template <typename Proof1, typename Proof2>
struct CheckSameProofTree {
  static_assert(ApplyC<IsProofTreeEqualTo, Proof1, Proof2>::value, "Proof trees differ");
};

template <typename Forest1, typename Forest2>
struct CheckSameProofForest {
  static_assert(ApplyC<IsForestEqualTo, Forest1, Forest2>::value, "Proof forests differ");
};

#define CHECK_SAME_PROOF(...) static_assert(true || sizeof(CheckSameProofTree<__VA_ARGS__>), "")
#define CHECK_SAME_FOREST(...) static_assert(true || sizeof(CheckSameProofForest<__VA_ARGS__>), "")

struct A{};
struct B{};
struct C{};
  
int main() {
  // TODO: Move this to the compile-time set test.
  static_assert(ApplyC<IsSameSet,
                       List<C, None>,
                       List<C>
                      >::value,
                "");
  CHECK_SAME_PROOF(ConsProofTree<List<C, None>, A>,
                   ConsProofTree<List<C>, A>);
  CHECK_SAME_FOREST(List<ConsProofTree<List<C, None>, A>>,
                    List<ConsProofTree<List<C>, A>>);

  using Proof_B_A = Apply<ConstructProofTree, List<B>, A>;
  using Proof_C_B = Apply<ConstructProofTree, List<C>, B>;  
  using Proof_C_A = Apply<ConstructProofTree, List<C>, A>;
  
  CHECK_SAME_FOREST(List<Proof_C_A, Proof_C_B>,
                    List<Proof_C_B, Proof_C_A>);
  
  CHECK_SAME_FOREST(Apply<CombineForestHypothesesWithProof, List<Proof_B_A>, Proof_C_B>,
                     List<Proof_C_A>);
  CHECK_SAME_FOREST(Apply<CombineForestHypothesesWithProof, List<Proof_C_B>, Proof_B_A>,
                     List<Proof_C_B>);
  
  CHECK_SAME_PROOF(Apply<CombineProofHypothesesWithForest, Proof_B_A, List<Proof_C_B>, List<B>>,
                   Proof_C_A);
  CHECK_SAME_PROOF(Apply<CombineProofHypothesesWithForest, Proof_C_B, List<Proof_B_A>, List<B>>,
                   Proof_C_B);
  
  CHECK_SAME_FOREST(Apply<AddProofTreeToForest, Proof_B_A, List<Proof_C_B>, List<B>>,
                    List<Proof_C_A, Proof_C_B>);
  CHECK_SAME_FOREST(Apply<AddProofTreeToForest, Proof_C_B, List<Proof_B_A>, List<A>>,
                    List<Proof_C_A, Proof_C_B>);
  
  return 0;
}

#else // FRUIT_NO_LOOP_CHECK

int main() {
  return 0;
}

#endif // FRUIT_NO_LOOP_CHECK

