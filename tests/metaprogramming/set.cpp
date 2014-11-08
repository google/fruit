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

#include <fruit/impl/metaprogramming/set.h>
#include <fruit/impl/metaprogramming/metaprogramming.h>

#include <vector>

using namespace std;
using namespace fruit::impl;

struct A {};
struct B {};
struct C {};

#define Assert(...) static_assert(true || sizeof(Conditional<__VA_ARGS__::value, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>), "static assertion failed.")
#define AssertNot(...) static_assert(true || sizeof(Conditional<!__VA_ARGS__::value, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>), "static assertion failed.")
#define AssertSameSet(...) Assert(ApplyC<IsSameSet, __VA_ARGS__>)
#define AssertNotSameSet(...) AssertNot(ApplyC<IsSameSet, __VA_ARGS__>)

void test_IsSameSet() {
  AssertSameSet(List<>, List<>);
  AssertSameSet(List<None>, List<>);
  AssertSameSet(List<>, List<None>);
  AssertSameSet(List<A, B>, List<B, A>);
  AssertNotSameSet(List<A, B>, List<B, B>);
  AssertNotSameSet(List<B, B>, List<B, A>);
  AssertSameSet(List<None, A, None>, List<A>);
  AssertNotSameSet(List<None, B, None>, List<A>);
}

void test_AddToSet() {
  AssertSameSet(Apply<AddToSet, A, List<>>, List<A>);
  AssertSameSet(Apply<AddToSet, A, List<A, B>>, List<A, B>);
  AssertSameSet(Apply<AddToSet, A, List<C, B>>, List<A, C, B>);
}

void test_SetListUnion() {
  AssertSameSet(Apply<SetListUnion, List<>, List<>>, List<>);
  AssertSameSet(Apply<SetListUnion, List<>, List<A, B>>, List<A, B>);
  AssertSameSet(Apply<SetListUnion, List<A, B>, List<>>, List<A, B>);
  AssertSameSet(Apply<SetListUnion, List<A, B>, List<C>>, List<A, B, C>);
  AssertSameSet(Apply<SetListUnion, List<A, B>, List<B, C>>, List<A, B, C>);
}

void test_ListToSet() {
  AssertSameSet(Apply<ListToSet, List<>>, List<>);
  AssertSameSet(Apply<ListToSet, List<A, A, B>>, List<A, B>);
}

void test_SetIntersection() {
  AssertSameSet(Apply<SetIntersection, List<A, B>, List<A, B>>, List<A, B>);
  AssertSameSet(Apply<SetIntersection, List<A>, List<A, B>>, List<A>);
  AssertSameSet(Apply<SetIntersection, List<A, B>, List<A>>, List<A>);
  AssertSameSet(Apply<SetIntersection, List<A>, List<B>>, List<>);
}

void test_SetUnion() {
  AssertSameSet(Apply<SetUnion, List<A, B>, List<A, B>>, List<A, B>);
  AssertSameSet(Apply<SetUnion, List<A>, List<A, B>>, List<A, B>);
  AssertSameSet(Apply<SetUnion, List<A, B>, List<A>>, List<A, B>);
  AssertSameSet(Apply<SetUnion, List<A>, List<B>>, List<A, B>);
}

void test_SetDifference() {
  AssertSameSet(Apply<SetDifference, List<A, B>, List<A, B>>, List<>);
  AssertSameSet(Apply<SetDifference, List<A>, List<A, B>>, List<>);
  AssertSameSet(Apply<SetDifference, List<A, B>, List<A>>, List<B>);
  AssertSameSet(Apply<SetDifference, List<A>, List<B>>, List<A>);
}

int main() {
  
  test_IsSameSet();
  test_AddToSet();
  test_SetListUnion();
  test_ListToSet();
  test_SetIntersection();
  test_SetUnion();
  test_SetDifference();
  
  return 0;
}
