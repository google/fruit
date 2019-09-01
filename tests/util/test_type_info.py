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

class TestTypeInfo(parameterized.TestCase):
    def test_size(self):
        source = '''
            int main() {
              Assert(getTypeId<char[27]>().type_info->size() == 27);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_alignment(self):
        source = '''
            struct alignas(128) TypeAligned128 {
            };
            
            int main() {
              Assert(getTypeId<TypeAligned128>().type_info->alignment() == 128);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('MyStruct', '{"MyStruct", "struct MyStruct"}'),
        ('std::pair<MyStruct, int>', '{"std::pair<MyStruct, int>", "std::__1::pair<MyStruct, int>"}'),
    ])
    def test_name(self, T, Expected):
        source = '''
            struct MyStruct {
            };
            
            int main() {
              std::string result = getTypeId<T>().type_info->name();
              Assert(std::string(getTypeId<T>()) == getTypeId<T>().type_info->name());
              for (std::string expected : Expected) {
                if (result == expected) {
                  return 0;
                }
              }
              std::cerr << "Unexpected demangled name." << std::endl;
              std::cerr << "typeid(T).name() == " << typeid(T).name() << std::endl;
              std::cerr << "getTypeId<T>().type_info->name() == " << result << std::endl;
              abort();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_isTriviallyDestructible_true(self):
        source = '''
            int main() {
              Assert(getTypeId<int>().type_info->isTriviallyDestructible());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_isTriviallyDestructible_false(self):
        source = '''
            int main() {
              Assert(!getTypeId<std::vector<int>>().type_info->isTriviallyDestructible());
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
