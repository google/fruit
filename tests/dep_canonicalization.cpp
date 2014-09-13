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

#include <fruit/fruit.h>

using namespace fruit;
using namespace fruit::impl;

struct A{};
struct B{};
struct C{};
  
int main() {
  using Dep1 = ConstructDep<A, unflatten_list<FlatList<B>>>;
  using Dep2 = ConstructDep<B, unflatten_list<FlatList<C>>>;
  
  FruitDelegateCheck(CheckSame<CanonicalizeDepWithDep<Dep1, Dep2>, ConstructDep<A, unflatten_list<FlatList<C>>>>);
  FruitDelegateCheck(CheckSame<CanonicalizeDepWithDep<Dep2, Dep1>, Dep2>);
  
  FruitDelegateCheck(CheckSame<flatten_list<typename CanonicalizeDepsWithDep<unflatten_list<FlatList<Dep1>>, Dep2>::type>, FlatList<ConstructDep<A, unflatten_list<FlatList<C>>>>>);
  FruitDelegateCheck(CheckSame<flatten_list<typename CanonicalizeDepsWithDep<unflatten_list<FlatList<Dep2>>, Dep1>::type>, FlatList<Dep2>>);
  
  FruitDelegateCheck(CheckSame<typename CanonicalizeDepWithDeps<Dep1, unflatten_list<FlatList<Dep2>>>::type, ConstructDep<A, unflatten_list<FlatList<C>>>>);
  FruitDelegateCheck(CheckSame<typename CanonicalizeDepWithDeps<Dep2, unflatten_list<FlatList<Dep1>>>::type, Dep2>);
  
  using Deps1 = flatten_list<AddDep<Dep1, unflatten_list<FlatList<Dep2>>>>;
  using Deps2 = flatten_list<AddDep<Dep2, unflatten_list<FlatList<Dep1>>>>;
  
  static_assert(true || sizeof(CheckSame<Deps1, FlatList<ConsDep<A, unflatten_list<FlatList<C>>>, ConsDep<B, unflatten_list<FlatList<C>>>>>), "");
  static_assert(true || sizeof(CheckSame<Deps2, FlatList<ConsDep<B, unflatten_list<FlatList<C>>>, ConsDep<A, unflatten_list<FlatList<C>>>>>), "");
  
  return 0;
}
