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

#include <fruit/impl/metaprogramming/list.h>
#include <fruit/impl/metaprogramming/metaprogramming.h>

#include <vector>

using namespace std;
using namespace fruit::impl;

struct A {};
struct B {};
struct C {};

#define Assert(...) static_assert(true || sizeof(Conditional<__VA_ARGS__::value, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>), "static assertion failed.")
#define AssertNot(...) static_assert(true || sizeof(Conditional<!__VA_ARGS__::value, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>), "static assertion failed.")
#define AssertSameList(...) Assert(std::is_same<__VA_ARGS__>)
#define AssertNotSameList(...) AssertNot(std::is_same<__VA_ARGS__>)

void test_IsList() {
  AssertNot(ApplyC<IsList, int>);
  Assert(ApplyC<IsList, List<>>);
  Assert(ApplyC<IsList, List<A>>);
}

void test_IsInList() {
  AssertNot(ApplyC<IsInList, A, List<>>);
  AssertNot(ApplyC<IsInList, A, List<B>>);
  Assert(ApplyC<IsInList, A, List<A>>);
}

void test_IsSameList() {
  AssertNotSameList(List<A, B>, List<B, A>);
  AssertNotSameList(List<A>, List<>);
  AssertNotSameList(List<>, List<A>);
}

void test_ListSize() {
  Assert(LazyC<ApplyC<ListSize, List<>>::value == 0>);
  Assert(LazyC<ApplyC<ListSize, List<A>>::value == 1>);
  Assert(LazyC<ApplyC<ListSize, List<A, B>>::value == 2>);
  Assert(LazyC<ApplyC<ListSize, List<A, None>>::value == 1>);
  Assert(LazyC<ApplyC<ListSize, List<None, None>>::value == 0>);
}

void test_ListApparentSize() {
  Assert(LazyC<ApplyC<ListApparentSize, List<>>::value == 0>);
  Assert(LazyC<ApplyC<ListApparentSize, List<A>>::value == 1>);
  Assert(LazyC<ApplyC<ListApparentSize, List<A, B>>::value == 2>);
  Assert(LazyC<ApplyC<ListApparentSize, List<A, None>>::value == 2>);
  Assert(LazyC<ApplyC<ListApparentSize, List<None, None>>::value == 2>);
}

void test_ConcatLists() {
  AssertSameList(Apply<ConcatLists, List<>, List<>>, List<>);
  AssertSameList(Apply<ConcatLists, List<>, List<A, B>>, List<A, B>);
  AssertSameList(Apply<ConcatLists, List<A, B>, List<>>, List<A, B>);
  AssertSameList(Apply<ConcatLists, List<A>, List<A, B>>, List<A, A, B>);
  AssertSameList(Apply<ConcatLists, List<A, B>, List<A, C>>, List<A, B, A, C>);
}

void test_ConcatMultipleLists() {
  AssertSameList(Apply<ConcatMultipleLists, List<>, List<>, List<>>, List<>);
  AssertSameList(Apply<ConcatMultipleLists, List<>, List<>, List<A, B>>, List<A, B>);
  AssertSameList(Apply<ConcatMultipleLists, List<>, List<A, B>, List<>>, List<A, B>);
  AssertSameList(Apply<ConcatMultipleLists, List<>, List<A>, List<A, B>>, List<A, A, B>);
  AssertSameList(Apply<ConcatMultipleLists, List<>, List<A, B>, List<A, C>>, List<A, B, A, C>);
}

void test_RemoveFromList() {
  AssertSameList(Apply<RemoveFromList, A, List<>>, List<>);
  AssertSameList(Apply<RemoveFromList, A, List<A>>, List<None>);
  AssertSameList(Apply<RemoveFromList, A, List<B>>, List<B>);
}

int main() {
  
  test_IsList();
  test_IsInList();
  test_IsSameList();
  test_ListSize();
  test_ListApparentSize();
  test_ConcatLists();
  test_ConcatMultipleLists();
  test_RemoveFromList();
  
  return 0;
}
