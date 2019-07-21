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
    #include <fruit/impl/meta/algos.h>
    '''

class TestAlgos(parameterized.TestCase):
    def test_HasDuplicates(self):
        source = '''
            int main() {
              AssertNot(HasDuplicates(Vector<>));
              AssertNot(HasDuplicates(Vector<Int<0>>));
              AssertNot(HasDuplicates(Vector<Int<0>, Int<1>>));
              Assert(HasDuplicates(Vector<Int<0>, Int<0>>));
              Assert(HasDuplicates(Vector<Int<2>, Int<0>, Int<1>, Int<0>, Int<3>>));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
