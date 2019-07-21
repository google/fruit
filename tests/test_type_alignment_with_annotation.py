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
    
    struct Annotation {};
    
    struct alignas(1) X {
      using Inject = X();
      X() {
        Assert(reinterpret_cast<std::uintptr_t>(this) % 1 == 0);
      }
    };
    
    struct alignas(4) Y {
      using Inject = Y();
      Y() {
        Assert(reinterpret_cast<std::uintptr_t>(this) % 4 == 0);
      }
    };
    
    struct alignas(128) Z {
      using Inject = Z();
      Z() {
        Assert(reinterpret_cast<std::uintptr_t>(this) % 128 == 0);
      }
    };
    
    using XAnnot = fruit::Annotated<Annotation, X>;
    using YAnnot = fruit::Annotated<Annotation, Y>;
    using ZAnnot = fruit::Annotated<Annotation, Z>;
    '''

class TestTypeAlignmentWithAnnotation(parameterized.TestCase):
    def test_type_alignment_with_annotation(self):
        source = '''
            fruit::Component<XAnnot, YAnnot, ZAnnot> getComponent() {
              return fruit::createComponent();
            }
            
            int main() {
              fruit::Injector<XAnnot, YAnnot, ZAnnot> injector(getComponent);
              
              injector.get<fruit::Annotated<Annotation, X*>>();
              injector.get<fruit::Annotated<Annotation, Y*>>();
              injector.get<fruit::Annotated<Annotation, Z*>>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
