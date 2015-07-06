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

#define Assert(...) static_assert(true || sizeof(Eval<Conditional<Lazy<Bool<__VA_ARGS__::value>>, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>>), "static assertion failed.")
#define AssertNot(...) static_assert(true || sizeof(Eval<Conditional<Lazy<Bool<!__VA_ARGS__::value>>, Lazy<int>, DebugTypeHelper<__VA_ARGS__>>>), "static assertion failed.")
#define AssertSameVector(...) Assert(std::is_same<__VA_ARGS__>)
#define AssertNotSameVector(...) AssertNot(std::is_same<__VA_ARGS__>)

#define AssertEqual(...) static_assert(fruit::impl::meta::Eval<IsSame(__VA_ARGS__)>::type::value, "")

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
  AssertEqual(Type<int>,   Select1st(Type<int>, Type<float>));
  AssertEqual(Type<float>, Select2nd(Type<int>, Type<float>));
  AssertEqual(Type<int>,   Select1st(Type<int>, Type<float>));
  AssertEqual(Type<float>, Select2nd(Type<int>, Type<float>));
}

void test_Call() {
  AssertEqual(Type<int>,   Call(Select1st, Type<int>, Type<float>));
  AssertEqual(Type<float>, Call(Select2nd, Type<int>, Type<float>));
  AssertEqual(Type<int>,   Call(Select1st, Type<int>, Type<float>));
  AssertEqual(Type<float>, Call(Select2nd, Type<int>, Type<float>));
}

void test_DeferArgs() {
  AssertEqual(Type<int>,   Call(Call(DeferArgs(Select1st), Type<int>), Type<float>));
  AssertEqual(Type<float>, Call(Call(DeferArgs(Select2nd), Type<int>), Type<float>));
  AssertEqual(Type<int>,   Call(Call(DeferArgs(Select1st), Type<int>), Type<float>));
  AssertEqual(Type<float>, Call(Call(DeferArgs(Select2nd), Type<int>), Type<float>));
}

int main() {
  test_ImplicitCall();
  test_Call();
  test_DeferArgs();
  
  return 0;
}
