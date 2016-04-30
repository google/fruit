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
#include <fruit/impl/meta/component.h>

struct A1 {};
struct B1 {};

using A = Type<A1>;
using B = Type<B1>;

using AssistedA = Type<Assisted<A1>>;
using AssistedB = Type<Assisted<B1>>;

void test_NumAssisted() {
  AssertSame(Int<0>, NumAssisted(Vector<>));
  AssertSame(Int<0>, NumAssisted(Vector<A>));
  AssertSame(Int<1>, NumAssisted(Vector<AssistedA>));
  AssertSame(Int<0>, NumAssisted(Vector<A, B>));
  AssertSame(Int<1>, NumAssisted(Vector<AssistedA, B>));
  AssertSame(Int<1>, NumAssisted(Vector<A, AssistedB>));
  AssertSame(Int<2>, NumAssisted(Vector<AssistedA, AssistedB>));
}

void test_NumAssistedBefore() {
  AssertSame(Int<0>, NumAssistedBefore(Int<0>, Vector<>));
  
  AssertSame(Int<0>, NumAssistedBefore(Int<0>, Vector<A>));
  AssertSame(Int<0>, NumAssistedBefore(Int<1>, Vector<A>));
  
  AssertSame(Int<0>, NumAssistedBefore(Int<0>, Vector<AssistedA>));
  AssertSame(Int<1>, NumAssistedBefore(Int<1>, Vector<AssistedA>));
  
  AssertSame(Int<0>, NumAssistedBefore(Int<0>, Vector<A, B>));
  AssertSame(Int<0>, NumAssistedBefore(Int<1>, Vector<A, B>));
  AssertSame(Int<0>, NumAssistedBefore(Int<2>, Vector<A, B>));
  
  AssertSame(Int<0>, NumAssistedBefore(Int<0>, Vector<AssistedA, B>));
  AssertSame(Int<1>, NumAssistedBefore(Int<1>, Vector<AssistedA, B>));
  AssertSame(Int<1>, NumAssistedBefore(Int<2>, Vector<AssistedA, B>));
  
  AssertSame(Int<0>, NumAssistedBefore(Int<0>, Vector<A, AssistedB>));
  AssertSame(Int<0>, NumAssistedBefore(Int<1>, Vector<A, AssistedB>));
  AssertSame(Int<1>, NumAssistedBefore(Int<2>, Vector<A, AssistedB>));
  
  AssertSame(Int<0>, NumAssistedBefore(Int<0>, Vector<AssistedA, AssistedB>));
  AssertSame(Int<1>, NumAssistedBefore(Int<1>, Vector<AssistedA, AssistedB>));
  AssertSame(Int<2>, NumAssistedBefore(Int<2>, Vector<AssistedA, AssistedB>));
}

int main() {
  test_NumAssisted();
  test_NumAssistedBefore();
  
  return 0;
}
