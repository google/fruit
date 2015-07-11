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

#include <fruit/impl/meta/vector.h>
#include <fruit/impl/meta/metaprogramming.h>

#include <vector>

using namespace std;
using namespace fruit::impl::meta;

struct A1 {};
struct B1 {};
struct C1 {};

using A = A1;
using B = B1;
using C = C1;

#define Assert(...) static_assert(fruit::impl::meta::Eval<__VA_ARGS__>::value, "")
#define AssertNot(...) Assert(Not(__VA_ARGS__))
#define AssertSameVector(...) Assert(IsSame(__VA_ARGS__))
#define AssertNotSameVector(...) AssertNot(IsSame(__VA_ARGS__))

void test_IsInVector() {
  AssertNot(IsInVector(A, Vector<>));
  AssertNot(IsInVector(A, Vector<B>));
  Assert(IsInVector(A, Vector<A>));
}

void test_IsSameVector() {
  AssertNotSameVector(Vector<A, B>, Vector<B, A>);
  AssertNotSameVector(Vector<A>, Vector<>);
  AssertNotSameVector(Vector<>, Vector<A>);
}

void test_VectorSize() {
  Assert(IsSame(VectorSize(Vector<>), Int<0>));
  Assert(IsSame(VectorSize(Vector<A>), Int<1>));
  Assert(IsSame(VectorSize(Vector<A, B>), Int<2>));
  Assert(IsSame(VectorSize(Vector<A, None>), Int<1>));
  Assert(IsSame(VectorSize(Vector<None, None>), Int<0>));
}

void test_VectorApparentSize() {
  Assert(IsSame(VectorApparentSize(Vector<>), Int<0>));
  Assert(IsSame(VectorApparentSize(Vector<A>), Int<1>));
  Assert(IsSame(VectorApparentSize(Vector<A, B>), Int<2>));
  Assert(IsSame(VectorApparentSize(Vector<A, None>), Int<2>));
  Assert(IsSame(VectorApparentSize(Vector<None, None>), Int<2>));
}

void test_ConcatVectors() {
  AssertSameVector(ConcatVectors(Vector<>, Vector<>), Vector<>);
  AssertSameVector(ConcatVectors(Vector<>, Vector<A, B>), Vector<A, B>);
  AssertSameVector(ConcatVectors(Vector<A, B>, Vector<>), Vector<A, B>);
  AssertSameVector(ConcatVectors(Vector<A>, Vector<A, B>), Vector<A, A, B>);
  AssertSameVector(ConcatVectors(Vector<A, B>, Vector<A, C>), Vector<A, B, A, C>);
}

void test_ConcatMultipleVectors() {
  AssertSameVector(ConcatMultipleVectors(Vector<>, Vector<>, Vector<>), Vector<>);
  AssertSameVector(ConcatMultipleVectors(Vector<>, Vector<>, Vector<A, B>), Vector<A, B>);
  AssertSameVector(ConcatMultipleVectors(Vector<>, Vector<A, B>, Vector<>), Vector<A, B>);
  AssertSameVector(ConcatMultipleVectors(Vector<>, Vector<A>, Vector<A, B>), Vector<A, A, B>);
  AssertSameVector(ConcatMultipleVectors(Vector<>, Vector<A, B>, Vector<A, C>), Vector<A, B, A, C>);
}

void test_RemoveFromVector() {
  AssertSameVector(RemoveFromVector(A, Vector<>), Vector<>);
  AssertSameVector(RemoveFromVector(A, Vector<A>), Vector<None>);
  AssertSameVector(RemoveFromVector(A, Vector<B>), Vector<B>);
}

int main() {
  
  test_IsInVector();
  test_IsSameVector();
  test_VectorSize();
  test_VectorApparentSize();
  test_ConcatVectors();
  test_ConcatMultipleVectors();
  test_RemoveFromVector();
  
  return 0;
}
