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

#include "common.h"
#include <fruit/impl/meta/metaprogramming.h>
#include <fruit/impl/meta/proof_trees.h>

#include <vector>

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

using Proof1 = Pair<X, ToSet<A, B>>;
using Proof1b = Pair<X, ToSet<B, A>>;
using Proof2 = Pair<Y, ToSet<B, C>>;

void test_IsProofTreeEqualTo() {
  AssertNotSameProof(Pair<X, ToSet<A>>, Pair<X, ToSet<B>>);
  AssertNotSameProof(Proof1, Proof2);
  AssertSameProof(Proof1, Proof1b);
}

void test_IsForestEqualTo() {
  AssertSameForest(Vector<>, Vector<>);
  AssertNotSameForest(Vector<Proof1>, Vector<Proof2>);
  AssertSameForest(Vector<Proof1, Proof2>, Vector<Proof2, Proof1b>);
}

void test_ConstructProofForest() {
  AssertSameForest(ConstructProofForest(EmptySet), Vector<>);
  AssertSameForest(ConstructProofForest(EmptySet, X), Vector<Pair<X, EmptySet>>);
  AssertSameForest(ConstructProofForest(ToSet<A>, X), Vector<Pair<X, ToSet<A>>>);
  AssertSameForest(ConstructProofForest(ToSet<A, B>, X), Vector<Pair<X, ToSet<A, B>>>);
  AssertSameForest(ConstructProofForest(ToSet<A, B>, X, Y), Vector<Pair<X, ToSet<A, B>>, Pair<Y, ToSet<A, B>>>);
}

int main() {
  
  test_IsProofTreeEqualTo();
  test_IsForestEqualTo();
  test_ConstructProofForest();
  
  return 0;
}
