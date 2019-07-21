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
    #include <fruit/impl/meta/component.h>
    
    struct A1 {};
    struct B1 {};
    
    using A = Type<A1>;
    using B = Type<B1>;
    
    using AssistedA = Type<Assisted<A1>>;
    using AssistedB = Type<Assisted<B1>>;
    '''

class TestMetaComponent(parameterized.TestCase):
    def test_NumAssisted(self):
        source = '''
            int main() {
              AssertSame(Int<0>, NumAssisted(Vector<>));
              AssertSame(Int<0>, NumAssisted(Vector<A>));
              AssertSame(Int<1>, NumAssisted(Vector<AssistedA>));
              AssertSame(Int<0>, NumAssisted(Vector<A, B>));
              AssertSame(Int<1>, NumAssisted(Vector<AssistedA, B>));
              AssertSame(Int<1>, NumAssisted(Vector<A, AssistedB>));
              AssertSame(Int<2>, NumAssisted(Vector<AssistedA, AssistedB>));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_NumAssistedBefore(self):
        source = '''
            int main() {
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
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
