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

#include <vector>

struct A {};
struct B {};
struct C {};

struct Select1st {
  template <typename T, typename U>
  struct apply {
    using type = T;
  };
};

struct Select2nd {
  template <typename T, typename U>
  struct apply {
    using type = U;
  };
};

void test_ImplicitCall() {
  AssertSameType(Type<int>,   Select1st(Type<int>, Type<float>));
  AssertSameType(Type<float>, Select2nd(Type<int>, Type<float>));
  AssertSameType(Type<int>,   Select1st(Type<int>, Type<float>));
  AssertSameType(Type<float>, Select2nd(Type<int>, Type<float>));
}

void test_Call() {
  AssertSameType(Type<int>,   Call(Select1st, Type<int>, Type<float>));
  AssertSameType(Type<float>, Call(Select2nd, Type<int>, Type<float>));
  AssertSameType(Type<int>,   Call(Select1st, Type<int>, Type<float>));
  AssertSameType(Type<float>, Call(Select2nd, Type<int>, Type<float>));
}

void test_DeferArgs() {
  AssertSameType(Type<int>,   Call(Call(DeferArgs(Select1st), Type<int>), Type<float>));
  AssertSameType(Type<float>, Call(Call(DeferArgs(Select2nd), Type<int>), Type<float>));
  AssertSameType(Type<int>,   Call(Call(DeferArgs(Select1st), Type<int>), Type<float>));
  AssertSameType(Type<float>, Call(Call(DeferArgs(Select2nd), Type<int>), Type<float>));
}

int main() {
  test_ImplicitCall();
  test_Call();
  test_DeferArgs();
  
  return 0;
}
