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
    #define IN_FRUIT_CPP_FILE 1
    
    #include "meta/common.h"
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
    '''

class TestBasics(parameterized.TestCase):
    def test_ImplicitCall(self):
        source = '''
            int main() {
              AssertSameType(Type<int>,   Id<Select1st(Type<int>, Type<float>)>);
              AssertSameType(Type<float>, Id<Select2nd(Type<int>, Type<float>)>);
              AssertSameType(Type<int>,   Id<Select1st(Type<int>, Type<float>)>);
              AssertSameType(Type<float>, Id<Select2nd(Type<int>, Type<float>)>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_Call(self):
        source = '''
            int main() {
              AssertSameType(Type<int>,   Id<Call(Select1st, Type<int>, Type<float>)>);
              AssertSameType(Type<float>, Id<Call(Select2nd, Type<int>, Type<float>)>);
              AssertSameType(Type<int>,   Id<Call(Select1st, Type<int>, Type<float>)>);
              AssertSameType(Type<float>, Id<Call(Select2nd, Type<int>, Type<float>)>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_DeferArgs(self):
        source = '''
            int main() {
              AssertSameType(Type<int>,   Id<Call(Id<Call(Id<DeferArgs(Select1st)>, Type<int>)>, Type<float>)>);
              AssertSameType(Type<float>, Id<Call(Id<Call(Id<DeferArgs(Select2nd)>, Type<int>)>, Type<float>)>);
              AssertSameType(Type<int>,   Id<Call(Id<Call(Id<DeferArgs(Select1st)>, Type<int>)>, Type<float>)>);
              AssertSameType(Type<float>, Id<Call(Id<Call(Id<DeferArgs(Select2nd)>, Type<int>)>, Type<float>)>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
