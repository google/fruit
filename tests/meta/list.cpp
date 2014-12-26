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

struct A {};
struct B {};
struct C {};

#define Assert(...) static_assert(true || sizeof(Conditional<__VA_ARGS__::value, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>), "static assertion failed.")
#define AssertNot(...) static_assert(true || sizeof(Conditional<!__VA_ARGS__::value, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>), "static assertion failed.")
#define AssertSameVector(...) Assert(std::is_same<__VA_ARGS__>)
#define AssertNotSameVector(...) AssertNot(std::is_same<__VA_ARGS__>)

void test_IsVector() {
  AssertNot(ApplyC<IsVector, int>);
  Assert(ApplyC<IsVector, Vector<>>);
  Assert(ApplyC<IsVector, Vector<A>>);
}

void test_IsInVector() {
  AssertNot(ApplyC<IsInVector, A, Vector<>>);
  AssertNot(ApplyC<IsInVector, A, Vector<B>>);
  Assert(ApplyC<IsInVector, A, Vector<A>>);
}

void test_IsSameVector() {
  AssertNotSameVector(Vector<A, B>, Vector<B, A>);
  AssertNotSameVector(Vector<A>, Vector<>);
  AssertNotSameVector(Vector<>, Vector<A>);
}

void test_VectorSize() {
  Assert(LazyC<ApplyC<VectorSize, Vector<>>::value == 0>);
  Assert(LazyC<ApplyC<VectorSize, Vector<A>>::value == 1>);
  Assert(LazyC<ApplyC<VectorSize, Vector<A, B>>::value == 2>);
  Assert(LazyC<ApplyC<VectorSize, Vector<A, None>>::value == 1>);
  Assert(LazyC<ApplyC<VectorSize, Vector<None, None>>::value == 0>);
}

void test_VectorApparentSize() {
  Assert(LazyC<ApplyC<VectorApparentSize, Vector<>>::value == 0>);
  Assert(LazyC<ApplyC<VectorApparentSize, Vector<A>>::value == 1>);
  Assert(LazyC<ApplyC<VectorApparentSize, Vector<A, B>>::value == 2>);
  Assert(LazyC<ApplyC<VectorApparentSize, Vector<A, None>>::value == 2>);
  Assert(LazyC<ApplyC<VectorApparentSize, Vector<None, None>>::value == 2>);
}

void test_ConcatVectors() {
  AssertSameVector(Apply<ConcatVectors, Vector<>, Vector<>>, Vector<>);
  AssertSameVector(Apply<ConcatVectors, Vector<>, Vector<A, B>>, Vector<A, B>);
  AssertSameVector(Apply<ConcatVectors, Vector<A, B>, Vector<>>, Vector<A, B>);
  AssertSameVector(Apply<ConcatVectors, Vector<A>, Vector<A, B>>, Vector<A, A, B>);
  AssertSameVector(Apply<ConcatVectors, Vector<A, B>, Vector<A, C>>, Vector<A, B, A, C>);
}

void test_ConcatMultipleVectors() {
  AssertSameVector(Apply<ConcatMultipleVectors, Vector<>, Vector<>, Vector<>>, Vector<>);
  AssertSameVector(Apply<ConcatMultipleVectors, Vector<>, Vector<>, Vector<A, B>>, Vector<A, B>);
  AssertSameVector(Apply<ConcatMultipleVectors, Vector<>, Vector<A, B>, Vector<>>, Vector<A, B>);
  AssertSameVector(Apply<ConcatMultipleVectors, Vector<>, Vector<A>, Vector<A, B>>, Vector<A, A, B>);
  AssertSameVector(Apply<ConcatMultipleVectors, Vector<>, Vector<A, B>, Vector<A, C>>, Vector<A, B, A, C>);
}

void test_RemoveFromVector() {
  AssertSameVector(Apply<RemoveFromVector, A, Vector<>>, Vector<>);
  AssertSameVector(Apply<RemoveFromVector, A, Vector<A>>, Vector<None>);
  AssertSameVector(Apply<RemoveFromVector, A, Vector<B>>, Vector<B>);
}

int main() {
  
  test_IsVector();
  test_IsInVector();
  test_IsSameVector();
  test_VectorSize();
  test_VectorApparentSize();
  test_ConcatVectors();
  test_ConcatMultipleVectors();
  test_RemoveFromVector();
  
  return 0;
}
