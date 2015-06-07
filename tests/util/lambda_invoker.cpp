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

#include <fruit/impl/util/lambda_invoker.h>
#include "../test_macros.h"

#include <cassert>

using namespace std;
using namespace fruit::impl;


void test_invoke_no_args() {
  // This is static because the lambda must have no captures.
  static int num_invocations = 0;
  
  auto l = []() {
    ++num_invocations;
  };
  using L = decltype(l);
  LambdaInvoker::invoke<L>();
  Assert(num_invocations == 1);
}

void test_invoke_some_args() {
  // This is static because the lambda must have no captures.
  static int num_invocations = 0;
  
  auto l = [](int n, double x) {
    Assert(n == 5);
    Assert(x == 3.14);
    ++num_invocations;
  };
  using L = decltype(l);
  LambdaInvoker::invoke<L>(5, 3.14);
  Assert(num_invocations == 1);
}


int main() {
  
  test_invoke_no_args();
  test_invoke_some_args();
  
  return 0;
}
