#!/usr/bin/env python3
#  Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS-IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from absl.testing import parameterized
from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #include "test_common.h"

    using namespace std;
    using namespace fruit::impl;
    '''

class TestLambdaInvoker(parameterized.TestCase):
    def test_invoke_no_args(self):
        source = '''
            int main() {
              // This is static because the lambda must have no captures.
              static int num_invocations = 0;
              
              auto l = []() {
                ++num_invocations;
              };
              using L = decltype(l);
              LambdaInvoker::invoke<L>();
              Assert(num_invocations == 1);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_invoke_some_args(self):
        source = '''
            int main() {
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
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
