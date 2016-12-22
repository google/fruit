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
from nose2.tools import params

from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #include "test_common.h"

    struct Annotation1 {};
    '''

@params(
    ('int', 'int*'),
    ('fruit::Annotated<Annotation1, int>', 'fruit::Annotated<Annotation1, int*>'))
def test_success(intAnnot, intPtrAnnot):
    source = '''
        fruit::Component<intAnnot> getComponentForInstance(int& n) {
          fruit::Component<> comp = fruit::createComponent()
            .bindInstance<intAnnot, int>(n);
          return fruit::createComponent()
            .install(comp)
            .bindInstance<intAnnot, int>(n);
        }

        int main() {
          int n = 5;
          fruit::Injector<intAnnot> injector(getComponentForInstance(n));
          if (injector.get<intPtrAnnot>() != &n)
            abort();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_abstract_class_ok(XAnnot):
    source = '''
        struct X {
          virtual void foo() = 0;
        };

        fruit::Component<XAnnot> getComponentForInstance(X& x) {
          fruit::Component<> comp = fruit::createComponent()
            .bindInstance<XAnnot, X>(x);
          return fruit::createComponent()
            .install(comp)
            .bindInstance<XAnnot, X>(x);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

if __name__ == '__main__':
    import nose2
    nose2.main()
