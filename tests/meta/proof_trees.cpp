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

struct A1 {};
struct B1 {};
struct C1 {};
struct D1 {};
struct X1 {};
struct Y1 {};

using A = Type<A1>;
using B = Type<B1>;
using C = Type<C1>;
using D = Type<D1>;
using X = Type<X1>;
using Y = Type<Y1>;

#define Assert(...) static_assert(fruit::impl::meta::Eval<__VA_ARGS__>::type::value, "")
#define AssertSameType(...) Assert(IsSame(__VA_ARGS__))
#define AssertSameProof(...) Assert(IsProofTreeEqualTo(__VA_ARGS__))
#define AssertSameForest(...) Assert(IsForestEqualTo(__VA_ARGS__))
#define AssertNotSameProof(...) Assert(Not(IsProofTreeEqualTo(__VA_ARGS__)))
#define AssertNotSameForest(...) Assert(Not(IsForestEqualTo(__VA_ARGS__)))

using Proof1 = ConsProofTree(Vector<A, B>, X);
using Proof1b = ConsProofTree(Vector<B, A>, X);
using Proof2 = ConsProofTree(Vector<B, C>, Y);

void test_IsProofTreeEqualTo() {
  AssertNotSameProof(ConsProofTree(Vector<A>, X), ConsProofTree(Vector<B>, X));
  AssertNotSameProof(Proof1, Proof2);
  AssertSameProof(Proof1, Proof1b);
}

void test_IsForestEqualTo() {
  AssertSameForest(Vector<>, Vector<>);
  AssertNotSameForest(Vector<Proof1>, Vector<Proof2>);
  AssertSameForest(Vector<Proof1, Proof2>, Vector<Proof2, Proof1b>);
}

void test_RemoveHpFromProofTree() {
  AssertSameProof(RemoveHpFromProofTree(A, ConsProofTree(Vector<>, X)), ConsProofTree(Vector<>, X));
  AssertSameProof(RemoveHpFromProofTree(A, Proof1), ConsProofTree(Vector<B>, X));
  AssertSameProof(RemoveHpFromProofTree(C, Proof1), ConsProofTree(Vector<A, B>, X));
  AssertSameProof(RemoveHpFromProofTree(X, Proof1), ConsProofTree(Vector<A, B>, X));
}

void test_RemoveHpFromProofForest() {
  AssertSameForest(RemoveHpFromProofForest(A, Vector<>), Vector<>);
  AssertSameForest(RemoveHpFromProofForest(A, Vector<Proof1>), ConsVector(ConsProofTree(Vector<B>, X)));
  AssertSameForest(RemoveHpFromProofForest(C, Vector<Proof1>), ConsVector(ConsProofTree(Vector<A, B>, X)));
  AssertSameForest(RemoveHpFromProofForest(X, Vector<Proof1>), ConsVector(ConsProofTree(Vector<A, B>, X)));
}

void test_ConstructProofTree() {
  AssertSameProof(ConstructProofTree(Vector<>, X), ConsProofTree(Vector<>, X));
  AssertSameProof(ConstructProofTree(Vector<A>, X), ConsProofTree(Vector<A>, X));
  AssertSameProof(ConstructProofTree(Vector<A, A>, X), ConsProofTree(Vector<A>, X));
  AssertSameProof(ConstructProofTree(Vector<A, B>, X), ConsProofTree(Vector<A, B>, X));
}

void test_ConstructProofForest() {
  AssertSameForest(ConstructProofForest(Vector<>), ConsVector());
  AssertSameForest(ConstructProofForest(Vector<>, X), ConsVector(ConsProofTree(Vector<>, X)));
  AssertSameForest(ConstructProofForest(Vector<A>, X), ConsVector(ConsProofTree(Vector<A>, X)));
  AssertSameForest(ConstructProofForest(Vector<A, A>, X), ConsVector(ConsProofTree(Vector<A>, X)));
  AssertSameForest(ConstructProofForest(Vector<A, B>, X), ConsVector(ConsProofTree(Vector<A, B>, X)));
  AssertSameForest(ConstructProofForest(Vector<A, B>, X, Y), ConsVector(ConsProofTree(Vector<A, B>, X), ConsProofTree(Vector<A, B>, Y)));
}

void test_HasSelfLoop() {
  Assert(Not(HasSelfLoop(ConsProofTree(Vector<>, X))));
  Assert(Not(HasSelfLoop(ConsProofTree(Vector<A>, X))));
  Assert(HasSelfLoop(ConsProofTree(Vector<X>, X)));
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
  AssertSameForest(AddProofTreeToForest(
                         ConsProofTree(Vector<B>, C),
                         ConsVector(
                           ConsProofTree(Vector<A>, B),
                           ConsProofTree(Vector<C>, D)
                         ),
                         Vector<B, D>),
                   ConsVector(ConsProofTree(Vector<A>, D), ConsProofTree(Vector<A>, C), ConsProofTree(Vector<A>, B)));
}

void test_FindProofInForest() {
  AssertSameType(FindProofInForest(B, ConsVector(ConsProofTree(Vector<A, B>, X))), None);
  AssertSameType(FindProofInForest(Y, ConsVector(ConsProofTree(Vector<A, B>, X))), None);
  AssertSameType(FindProofInForest(X, ConsVector(ConsProofTree(Vector<A, B>, X))), ConsProofTree(Vector<A, B>, X));
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
