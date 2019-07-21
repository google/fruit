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
    
    #define IN_FRUIT_CPP_FILE 1
    #include <fruit/impl/data_structures/fixed_size_vector.templates.h>
    
    using namespace std;
    using namespace fruit::impl;
    '''

class TestFixedSizeVector(parameterized.TestCase):
    def test_empty_capacity_0(self):
        source = '''
            int main() {
              FixedSizeVector<int> v;
              const FixedSizeVector<int>& const_v = v;
              Assert(v.size() == 0);
              Assert(v.data() == nullptr);
              Assert(v.begin() == v.end());
              Assert(const_v.data() == nullptr);
              Assert(const_v.begin() == const_v.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_empty_capacity_nonzero(self):
        source = '''
            int main() {
              FixedSizeVector<int> v(15);
              const FixedSizeVector<int>& const_v = v;
              Assert(v.size() == 0);
              Assert(v.data() != nullptr);
              Assert(v.begin() == v.end());
              Assert(const_v.data() != nullptr);
              Assert(const_v.begin() == const_v.end());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_push_back(self):
        source = '''
            int main() {
              FixedSizeVector<int> v(2);
              v.push_back(1000);
              v.push_back(2000);
              const FixedSizeVector<int>& const_v = v;
              Assert(v.size() == 2);
              Assert(v.data() == &(v[0]));
              Assert(v.end() - v.begin() == 2);
              Assert(&*(v.begin()) == &(v[0]));
              Assert(&*(v.begin() + 1) == &(v[1]));
              Assert(const_v.data() != nullptr);
              Assert(const_v.begin() == &(const_v[0]));
              Assert(const_v.end() - const_v.begin() == 2);
              Assert(&*(const_v.begin()) == &(const_v[0]));
              Assert(&*(const_v.begin() + 1) == &(const_v[1]));
              Assert(v[0] == 1000);
              Assert(v[1] == 2000);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_2arg_constructor(self):
        source = '''
            int main() {
              FixedSizeVector<int> v(2, 1000);
              const FixedSizeVector<int>& const_v = v;
              Assert(v.size() == 2);
              Assert(v.data() == &(v[0]));
              Assert(v.end() - v.begin() == 2);
              Assert(&*(v.begin()) == &(v[0]));
              Assert(&*(v.begin() + 1) == &(v[1]));
              Assert(const_v.data() != nullptr);
              Assert(const_v.begin() == &(const_v[0]));
              Assert(const_v.end() - const_v.begin() == 2);
              Assert(&*(const_v.begin()) == &(const_v[0]));
              Assert(&*(const_v.begin() + 1) == &(const_v[1]));
              Assert(v[0] == 1000);
              Assert(v[1] == 1000);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_move_constructor(self):
        source = '''
            int main() {
              FixedSizeVector<int> v1(2);
              v1.push_back(1000);
              v1.push_back(2000);
              FixedSizeVector<int> v = std::move(v1);
              Assert(v.size() == 2);
              Assert(v.data() == &(v[0]));
              Assert(v.end() - v.begin() == 2);
              Assert(&*(v.begin()) == &(v[0]));
              Assert(&*(v.begin() + 1) == &(v[1]));
              Assert(v[0] == 1000);
              Assert(v[1] == 2000);  
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_copy_constructor(self):
        source = '''
            int main() {
              FixedSizeVector<int> v1(2);
              v1.push_back(1000);
              v1.push_back(2000);
              FixedSizeVector<int> v(v1, 3);
              Assert(v.size() == 2);
              Assert(v.data() == &(v[0]));
              Assert(v.end() - v.begin() == 2);
              Assert(&*(v.begin()) == &(v[0]));
              Assert(&*(v.begin() + 1) == &(v[1]));
              Assert(v[0] == 1000);
              Assert(v[1] == 2000);  
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_move_assignment(self):
        source = '''
            int main() {
              FixedSizeVector<int> v1(2);
              v1.push_back(1000);
              v1.push_back(2000);
              FixedSizeVector<int> v;
              v = std::move(v1);
              Assert(v.size() == 2);
              Assert(v.data() == &(v[0]));
              Assert(v.end() - v.begin() == 2);
              Assert(&*(v.begin()) == &(v[0]));
              Assert(&*(v.begin() + 1) == &(v[1]));
              Assert(v[0] == 1000);
              Assert(v[1] == 2000);  
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_swap(self):
        source = '''
            int main() {
              FixedSizeVector<int> v1(2);
              v1.push_back(1000);
              v1.push_back(2000);
              FixedSizeVector<int> v2(1);
              v2.push_back(3000);
              std::swap(v1, v2);
              Assert(v1.size() == 1);
              Assert(v1[0] == 3000);
              Assert(v2.size() == 2);
              Assert(v2[0] == 1000);
              Assert(v2[1] == 2000);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_clear(self):
        source = '''
            int main() {
              FixedSizeVector<int> v(2);
              v.push_back(1000);
              v.push_back(2000);
              v.clear();
              Assert(v.size() == 0);
              Assert(v.data() != nullptr);
              Assert(v.begin() == v.end());
              // This must not blow up, clear() must preserve the capacity.
              v.push_back(1000);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
