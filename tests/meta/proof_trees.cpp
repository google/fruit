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

#define IN_FRUIT_CPP_FILE

#include <fruit/impl/meta/proof_trees.h>
#include <fruit/impl/meta/metaprogramming.h>

#include <vector>

using namespace std;
using namespace fruit::impl::meta;

struct A {};
struct B {};
struct C {};
struct D {};
struct X {};
struct Y {};

#define Assert(...) static_assert(true || sizeof(Conditional<__VA_ARGS__::value, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>), "static assertion failed.")
#define AssertNot(...) static_assert(true || sizeof(Conditional<!__VA_ARGS__::value, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>), "static assertion failed.")
#define AssertSameType(...) Assert(std::is_same<__VA_ARGS__>)
#define AssertSameProof(...) Assert(ApplyC<IsProofTreeEqualTo, __VA_ARGS__>)
#define AssertSameForest(...) Assert(ApplyC<IsForestEqualTo, __VA_ARGS__>)
#define AssertNotSameProof(...) AssertNot(ApplyC<IsProofTreeEqualTo, __VA_ARGS__>)
#define AssertNotSameForest(...) AssertNot(ApplyC<IsForestEqualTo, __VA_ARGS__>)

using Proof1 = ConsProofTree<List<A, B>, X>;
using Proof1b = ConsProofTree<List<B, A>, X>;
using Proof2 = ConsProofTree<List<B, C>, Y>;

void test_IsProofTreeEqualTo() {
  AssertNotSameProof(ConsProofTree<List<A>, X>, ConsProofTree<List<B>, X>);
  AssertNotSameProof(Proof1, Proof2);
  AssertSameProof(Proof1, Proof1b);
}

void test_IsForestEqualTo() {
  AssertSameForest(List<>, List<>);
  AssertNotSameForest(List<Proof1>, List<Proof2>);
  AssertSameForest(List<Proof1, Proof2>, List<Proof2, Proof1b>);
}

void test_RemoveHpFromProofTree() {
  AssertSameProof(Apply<RemoveHpFromProofTree, A, ConsProofTree<List<>, X>>, ConsProofTree<List<>, X>);
  AssertSameProof(Apply<RemoveHpFromProofTree, A, Proof1>, ConsProofTree<List<B>, X>);
  AssertSameProof(Apply<RemoveHpFromProofTree, C, Proof1>, ConsProofTree<List<A, B>, X>);
  AssertSameProof(Apply<RemoveHpFromProofTree, X, Proof1>, ConsProofTree<List<A, B>, X>);
}

void test_RemoveHpFromProofForest() {
  AssertSameForest(Apply<RemoveHpFromProofForest, A, List<>>, List<>);
  AssertSameForest(Apply<RemoveHpFromProofForest, A, List<Proof1>>, List<ConsProofTree<List<B>, X>>);
  AssertSameForest(Apply<RemoveHpFromProofForest, C, List<Proof1>>, List<ConsProofTree<List<A, B>, X>>);
  AssertSameForest(Apply<RemoveHpFromProofForest, X, List<Proof1>>, List<ConsProofTree<List<A, B>, X>>);
}

void test_ConstructProofTree() {
  AssertSameProof(Apply<ConstructProofTree, List<>, X>, ConsProofTree<List<>, X>);
  AssertSameProof(Apply<ConstructProofTree, List<A>, X>, ConsProofTree<List<A>, X>);
  AssertSameProof(Apply<ConstructProofTree, List<A, A>, X>, ConsProofTree<List<A>, X>);
  AssertSameProof(Apply<ConstructProofTree, List<A, B>, X>, ConsProofTree<List<A, B>, X>);
}

void test_ConstructProofForest() {
  AssertSameForest(Apply<ConstructProofForest, List<>>, List<>);
  AssertSameForest(Apply<ConstructProofForest, List<>, X>, List<ConsProofTree<List<>, X>>);
  AssertSameForest(Apply<ConstructProofForest, List<A>, X>, List<ConsProofTree<List<A>, X>>);
  AssertSameForest(Apply<ConstructProofForest, List<A, A>, X>, List<ConsProofTree<List<A>, X>>);
  AssertSameForest(Apply<ConstructProofForest, List<A, B>, X>, List<ConsProofTree<List<A, B>, X>>);
  AssertSameForest(Apply<ConstructProofForest, List<A, B>, X, Y>, List<ConsProofTree<List<A, B>, X>, ConsProofTree<List<A, B>, Y>>);
}

void test_HasSelfLoop() {
  AssertNot(ApplyC<HasSelfLoop, ConsProofTree<List<>, X>>);
  AssertNot(ApplyC<HasSelfLoop, ConsProofTree<List<A>, X>>);
  Assert(ApplyC<HasSelfLoop, ConsProofTree<List<X>, X>>);
}

void test_AddProofTreeToForest() {
  // Before:
  // A->B
  // C->D
  // Adding: B->C
  // After:
  // A->D
  // A->C
  // A->B
  AssertSameForest(Apply<AddProofTreeToForest,
                         ConsProofTree<List<B>, C>,
                         List<
                           ConsProofTree<List<A>, B>,
                           ConsProofTree<List<C>, D>
                         >,
                         List<B, D>>,
                   List<ConsProofTree<List<A>, D>, ConsProofTree<List<A>, C>, ConsProofTree<List<A>, B>>);
}

void test_FindProofInForest() {
  AssertSameType(Apply<FindProofInForest, B, List<ConsProofTree<List<A, B>, X>>>, None);
  AssertSameType(Apply<FindProofInForest, Y, List<ConsProofTree<List<A, B>, X>>>, None);
  AssertSameType(Apply<FindProofInForest, X, List<ConsProofTree<List<A, B>, X>>>, ConsProofTree<List<A, B>, X>);
}

int main() {
  
  test_IsProofTreeEqualTo();
  test_IsForestEqualTo();
  test_RemoveHpFromProofTree();
  test_RemoveHpFromProofForest();
  test_ConstructProofTree();
  test_ConstructProofForest();
  test_HasSelfLoop();
  test_AddProofTreeToForest();
  test_FindProofInForest();
  
  return 0;
}
