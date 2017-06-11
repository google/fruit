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

from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #include "test_common.h"

    using namespace std;
    using namespace fruit::impl;
    '''

def test_size():
    source = '''
        int main() {
          Assert(getTypeId<char[27]>().type_info->size() == 27);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_alignment():
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

def test_name():
    source = '''
        struct MyStruct {
        };
        
        int main() {
          std::string result = getTypeId<MyStruct>().type_info->name();
          if (result != "MyStruct" && result != "struct MyStruct") {
            std::cerr << "Demangling failed." << std::endl;
            std::cerr << "typeid(MyStruct).name() == " << typeid(MyStruct).name() << std::endl;
            std::cerr << "getTypeId<MyStruct>().type_info->name() == " << result << std::endl;
            abort();
          }
          Assert(std::string(getTypeId<MyStruct>()) == "MyStruct" || std::string(getTypeId<MyStruct>()) == "struct MyStruct");
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_isTriviallyDestructible_true():
    source = '''
        int main() {
          Assert(getTypeId<int>().type_info->isTriviallyDestructible());
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_isTriviallyDestructible_false():
    source = '''
        int main() {
          Assert(!getTypeId<std::vector<int>>().type_info->isTriviallyDestructible());
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

if __name__== '__main__':
    main(__file__)
