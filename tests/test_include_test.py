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
    '''

FRUIT_PUBLIC_HEADERS = [
    "component.h",
    "fruit.h",
    "fruit_forward_decls.h",
    "injector.h",
    "macro.h",
    "normalized_component.h",
    "provider.h",
]

class TestHeaders(parameterized.TestCase):
    @parameterized.parameters(FRUIT_PUBLIC_HEADERS)
    def test_header_self_contained(self, HeaderFile):
        source = '''
            #include <fruit/HeaderFile>
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
