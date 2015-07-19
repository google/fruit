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
#include <fruit/impl/meta/vector.h>
#include <fruit/impl/meta/metaprogramming.h>

struct A1 {};
struct B1 {};
struct C1 {};

using A = A1;
using B = B1;
using C = C1;

void test_IsInVector() {
  AssertNot(IsInVector(A, Vector<>));
  AssertNot(IsInVector(A, Vector<B>));
  Assert(IsInVector(A, Vector<A>));
}

void test_IsSameVector() {
  AssertNotSameType(Vector<A, B>, Vector<B, A>);
  AssertNotSameType(Vector<A>, Vector<>);
  AssertNotSameType(Vector<>, Vector<A>);
}

void test_VectorSize() {
  AssertSameType(VectorSize(Vector<>), Int<0>);
  AssertSameType(VectorSize(Vector<A>), Int<1>);
  AssertSameType(VectorSize(Vector<A, B>), Int<2>);
}

void test_ConcatVectors() {
  AssertSameType(ConcatVectors(Vector<>, Vector<>), Vector<>);
  AssertSameType(ConcatVectors(Vector<>, Vector<A, B>), Vector<A, B>);
  AssertSameType(ConcatVectors(Vector<A, B>, Vector<>), Vector<A, B>);
  AssertSameType(ConcatVectors(Vector<A>, Vector<A, B>), Vector<A, A, B>);
  AssertSameType(ConcatVectors(Vector<A, B>, Vector<A, C>), Vector<A, B, A, C>);
}

void test_VectorEndsWith() {
  Assert(VectorEndsWith(Vector<A, B>, B));
  AssertNot(VectorEndsWith(Vector<A, B>, A));
  AssertNot(VectorEndsWith(Vector<>, A));
}

int main() {
  
  test_IsInVector();
  test_IsSameVector();
  test_VectorSize();
  test_ConcatVectors();
  test_VectorEndsWith();
  
  return 0;
}
