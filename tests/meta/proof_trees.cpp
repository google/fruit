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

#define Assert(...) static_assert(true || sizeof(Eval<Conditional<Lazy<Bool<__VA_ARGS__::value>>, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>>), "static assertion failed.")
#define AssertNot(...) static_assert(true || sizeof(Eval<Conditional<Lazy<Bool<!__VA_ARGS__::value>>, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>>), "static assertion failed.")
#define AssertSameType(...) Assert(std::is_same<__VA_ARGS__>)
#define AssertSameProof(...) Assert(Apply<IsProofTreeEqualTo, __VA_ARGS__>)
#define AssertSameForest(...) Assert(Apply<IsForestEqualTo, __VA_ARGS__>)
#define AssertNotSameProof(...) AssertNot(Apply<IsProofTreeEqualTo, __VA_ARGS__>)
#define AssertNotSameForest(...) AssertNot(Apply<IsForestEqualTo, __VA_ARGS__>)

using Proof1 = ConsProofTree<Vector<A, B>, X>;
using Proof1b = ConsProofTree<Vector<B, A>, X>;
using Proof2 = ConsProofTree<Vector<B, C>, Y>;

void test_IsProofTreeEqualTo() {
  AssertNotSameProof(ConsProofTree<Vector<A>, X>, ConsProofTree<Vector<B>, X>);
  AssertNotSameProof(Proof1, Proof2);
  AssertSameProof(Proof1, Proof1b);
}

void test_IsForestEqualTo() {
  AssertSameForest(Vector<>, Vector<>);
  AssertNotSameForest(Vector<Proof1>, Vector<Proof2>);
  AssertSameForest(Vector<Proof1, Proof2>, Vector<Proof2, Proof1b>);
}

void test_RemoveHpFromProofTree() {
  AssertSameProof(Apply<RemoveHpFromProofTree, A, ConsProofTree<Vector<>, X>>, ConsProofTree<Vector<>, X>);
  AssertSameProof(Apply<RemoveHpFromProofTree, A, Proof1>, ConsProofTree<Vector<B>, X>);
  AssertSameProof(Apply<RemoveHpFromProofTree, C, Proof1>, ConsProofTree<Vector<A, B>, X>);
  AssertSameProof(Apply<RemoveHpFromProofTree, X, Proof1>, ConsProofTree<Vector<A, B>, X>);
}

void test_RemoveHpFromProofForest() {
  AssertSameForest(Apply<RemoveHpFromProofForest, A, Vector<>>, Vector<>);
  AssertSameForest(Apply<RemoveHpFromProofForest, A, Vector<Proof1>>, Vector<ConsProofTree<Vector<B>, X>>);
  AssertSameForest(Apply<RemoveHpFromProofForest, C, Vector<Proof1>>, Vector<ConsProofTree<Vector<A, B>, X>>);
  AssertSameForest(Apply<RemoveHpFromProofForest, X, Vector<Proof1>>, Vector<ConsProofTree<Vector<A, B>, X>>);
}

void test_ConstructProofTree() {
  AssertSameProof(Apply<ConstructProofTree, Vector<>, X>, ConsProofTree<Vector<>, X>);
  AssertSameProof(Apply<ConstructProofTree, Vector<A>, X>, ConsProofTree<Vector<A>, X>);
  AssertSameProof(Apply<ConstructProofTree, Vector<A, A>, X>, ConsProofTree<Vector<A>, X>);
  AssertSameProof(Apply<ConstructProofTree, Vector<A, B>, X>, ConsProofTree<Vector<A, B>, X>);
}

void test_ConstructProofForest() {
  AssertSameForest(Apply<ConstructProofForest, Vector<>>, Vector<>);
  AssertSameForest(Apply<ConstructProofForest, Vector<>, X>, Vector<ConsProofTree<Vector<>, X>>);
  AssertSameForest(Apply<ConstructProofForest, Vector<A>, X>, Vector<ConsProofTree<Vector<A>, X>>);
  AssertSameForest(Apply<ConstructProofForest, Vector<A, A>, X>, Vector<ConsProofTree<Vector<A>, X>>);
  AssertSameForest(Apply<ConstructProofForest, Vector<A, B>, X>, Vector<ConsProofTree<Vector<A, B>, X>>);
  AssertSameForest(Apply<ConstructProofForest, Vector<A, B>, X, Y>, Vector<ConsProofTree<Vector<A, B>, X>, ConsProofTree<Vector<A, B>, Y>>);
}

void test_HasSelfLoop() {
  AssertNot(Apply<HasSelfLoop, ConsProofTree<Vector<>, X>>);
  AssertNot(Apply<HasSelfLoop, ConsProofTree<Vector<A>, X>>);
  Assert(Apply<HasSelfLoop, ConsProofTree<Vector<X>, X>>);
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
                         ConsProofTree<Vector<B>, C>,
                         Vector<
                           ConsProofTree<Vector<A>, B>,
                           ConsProofTree<Vector<C>, D>
                         >,
                         Vector<B, D>>,
                   Vector<ConsProofTree<Vector<A>, D>, ConsProofTree<Vector<A>, C>, ConsProofTree<Vector<A>, B>>);
}

void test_FindProofInForest() {
  AssertSameType(Apply<FindProofInForest, B, Vector<ConsProofTree<Vector<A, B>, X>>>, None);
  AssertSameType(Apply<FindProofInForest, Y, Vector<ConsProofTree<Vector<A, B>, X>>>, None);
  AssertSameType(Apply<FindProofInForest, X, Vector<ConsProofTree<Vector<A, B>, X>>>, ConsProofTree<Vector<A, B>, X>);
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
