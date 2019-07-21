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
    #include <fruit/impl/meta/list.h>
    #include <fruit/impl/meta/metaprogramming.h>
    '''

class TestList(parameterized.TestCase):
    def test_FoldList(self):
        source = '''
            struct Helper {
                template <typename CurrentResult, typename N>
                struct apply {
                    using type = Int<(CurrentResult::value + 1) * N::value>;
                };
            };
            
            int main() {
                AssertSameType(Id<FoldList(EmptyList, Helper, Int<4>)>, Int<4>);
                AssertSameType(Id<FoldList(Cons<Int<2>, EmptyList>, Helper, Int<4>)>, Int<10>);
                AssertSameType(Id<FoldList(Cons<Int<3>, Cons<Int<2>, EmptyList>>, Helper, Int<4>)>, Int<32>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
