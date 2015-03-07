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
using namespace fruit::impl::meta;

#ifndef FRUIT_NO_LOOP_CHECKz

template <typename Proof1, typename Proof2>
struct CheckSameProofTree {
  static_assert(Apply<IsProofTreeEqualTo, Proof1, Proof2>::value, "Proof trees differ");
};

template <typename Forest1, typename Forest2>
struct CheckSameProofForest {
  static_assert(Apply<IsForestEqualTo, Forest1, Forest2>::value, "Proof forests differ");
};

#define CHECK_SAME_PROOF(...) static_assert(true || sizeof(CheckSameProofTree<__VA_ARGS__>), "")
#define CHECK_SAME_FOREST(...) static_assert(true || sizeof(CheckSameProofForest<__VA_ARGS__>), "")

struct A{};
struct B{};
struct C{};
  
int main() {
  // TODO: Move this to the compile-time set test.
  static_assert(Apply<IsSameSet,
                       Vector<C, None>,
                       Vector<C>
                      >::value,
                "");
  CHECK_SAME_PROOF(ConsProofTree<Vector<C, None>, A>,
                   ConsProofTree<Vector<C>, A>);
  CHECK_SAME_FOREST(Vector<ConsProofTree<Vector<C, None>, A>>,
                    Vector<ConsProofTree<Vector<C>, A>>);

  using Proof_B_A = Apply<ConstructProofTree, Vector<B>, A>;
  using Proof_C_B = Apply<ConstructProofTree, Vector<C>, B>;  
  using Proof_C_A = Apply<ConstructProofTree, Vector<C>, A>;
  
  CHECK_SAME_FOREST(Vector<Proof_C_A, Proof_C_B>,
                    Vector<Proof_C_B, Proof_C_A>);
  
  CHECK_SAME_FOREST(Apply<CombineForestHypothesesWithProof, Vector<Proof_B_A>, Proof_C_B>,
                     Vector<Proof_C_A>);
  CHECK_SAME_FOREST(Apply<CombineForestHypothesesWithProof, Vector<Proof_C_B>, Proof_B_A>,
                     Vector<Proof_C_B>);
  
  CHECK_SAME_PROOF(Apply<CombineProofHypothesesWithForest, Proof_B_A, Vector<Proof_C_B>, Vector<B>>,
                   Proof_C_A);
  CHECK_SAME_PROOF(Apply<CombineProofHypothesesWithForest, Proof_C_B, Vector<Proof_B_A>, Vector<B>>,
                   Proof_C_B);
  
  CHECK_SAME_FOREST(Apply<AddProofTreeToForest, Proof_B_A, Vector<Proof_C_B>, Vector<B>>,
                    Vector<Proof_C_A, Proof_C_B>);
  CHECK_SAME_FOREST(Apply<AddProofTreeToForest, Proof_C_B, Vector<Proof_B_A>, Vector<A>>,
                    Vector<Proof_C_A, Proof_C_B>);
  
  return 0;
}

#else // FRUIT_NO_LOOP_CHECK

int main() {
  return 0;
}

#endif // FRUIT_NO_LOOP_CHECK

