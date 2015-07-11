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

#include <fruit/impl/meta/set.h>
#include <fruit/impl/meta/metaprogramming.h>

#include <vector>

using namespace std;
using namespace fruit::impl::meta;

struct A {};
struct B {};
struct C {};

#define Assert(...) static_assert(fruit::impl::meta::Eval<__VA_ARGS__>::value, "")
#define AssertNot(...) Assert(Not(__VA_ARGS__))
#define AssertSameSet(...) Assert(IsSameSet(__VA_ARGS__))
#define AssertNotSameSet(...) AssertNot(IsSameSet(__VA_ARGS__))

void test_IsSameSet() {
  AssertSameSet(Vector<>, Vector<>);
  AssertSameSet(Vector<None>, Vector<>);
  AssertSameSet(Vector<>, Vector<None>);
  AssertSameSet(Vector<A, B>, Vector<B, A>);
  AssertNotSameSet(Vector<A, B>, Vector<B, B>);
  AssertNotSameSet(Vector<B, B>, Vector<B, A>);
  AssertSameSet(Vector<None, A, None>, Vector<A>);
  AssertNotSameSet(Vector<None, B, None>, Vector<A>);
}

void test_AddToSet() {
  AssertSameSet(AddToSet(A, Vector<>), Vector<A>);
  AssertSameSet(AddToSet(A, Vector<A, B>), Vector<A, B>);
  AssertSameSet(AddToSet(A, Vector<C, B>), Vector<A, C, B>);
}

void test_SetVectorUnion() {
  AssertSameSet(SetVectorUnion(Vector<>, Vector<>), Vector<>);
  AssertSameSet(SetVectorUnion(Vector<>, Vector<A, B>), Vector<A, B>);
  AssertSameSet(SetVectorUnion(Vector<A, B>, Vector<>), Vector<A, B>);
  AssertSameSet(SetVectorUnion(Vector<A, B>, Vector<C>), Vector<A, B, C>);
  AssertSameSet(SetVectorUnion(Vector<A, B>, Vector<B, C>), Vector<A, B, C>);
}

void test_VectorToSet() {
  AssertSameSet(VectorToSet(Vector<>), Vector<>);
  AssertSameSet(VectorToSet(Vector<A, A, B>), Vector<A, B>);
}

void test_SetIntersection() {
  AssertSameSet(SetIntersection(Vector<A, B>, Vector<A, B>), Vector<A, B>);
  AssertSameSet(SetIntersection(Vector<A>, Vector<A, B>), Vector<A>);
  AssertSameSet(SetIntersection(Vector<A, B>, Vector<A>), Vector<A>);
  AssertSameSet(SetIntersection(Vector<A>, Vector<B>), Vector<>);
}

void test_SetUnion() {
  AssertSameSet(SetUnion(Vector<A, B>, Vector<A, B>), Vector<A, B>);
  AssertSameSet(SetUnion(Vector<A>, Vector<A, B>), Vector<A, B>);
  AssertSameSet(SetUnion(Vector<A, B>, Vector<A>), Vector<A, B>);
  AssertSameSet(SetUnion(Vector<A>, Vector<B>), Vector<A, B>);
}

void test_SetDifference() {
  AssertSameSet(SetDifference(Vector<A, B>, Vector<A, B>), Vector<>);
  AssertSameSet(SetDifference(Vector<A>, Vector<A, B>), Vector<>);
  AssertSameSet(SetDifference(Vector<A, B>, Vector<A>), Vector<B>);
  AssertSameSet(SetDifference(Vector<A>, Vector<B>), Vector<A>);
}

int main() {
  
  test_IsSameSet();
  test_AddToSet();
  test_SetVectorUnion();
  test_VectorToSet();
  test_SetIntersection();
  test_SetUnion();
  test_SetDifference();
  
  return 0;
}
