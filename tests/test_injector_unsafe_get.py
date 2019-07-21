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

    struct Annotation1 {};
    struct Annotation2 {};
    struct Annotation3 {};
    '''

class TestInjectorUnsafeGet(parameterized.TestCase):
    @parameterized.parameters([
        ('X', 'Y', 'Z'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, Y>', 'fruit::Annotated<Annotation3, Z>'),
    ])
    def test_success(self, XAnnot, YAnnot, ZAnnot):
        source = '''
            struct Y {
              using Inject = Y();
              Y() = default;
            };
    
            struct X {
              using Inject = X(YAnnot);
              X(Y) {
              }
            };
    
            struct Z {};
    
            fruit::Component<XAnnot> getComponent() {
              return fruit::createComponent();
            }
    
            fruit::Component<> getRootComponent() {
              return fruit::createComponent()
                  .install(getComponent);
            }
    
            int main() {
              fruit::Injector<> injector(getRootComponent);
              const X* x = fruit::impl::InjectorAccessorForTests::unsafeGet<XAnnot>(injector);
              const Y* y = fruit::impl::InjectorAccessorForTests::unsafeGet<YAnnot>(injector);
              const Z* z = fruit::impl::InjectorAccessorForTests::unsafeGet<ZAnnot>(injector);
    
              (void) x;
              (void) y;
              (void) z;
              Assert(x != nullptr);
              Assert(y != nullptr);
              Assert(z == nullptr);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
