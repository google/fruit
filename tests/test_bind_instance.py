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
import pytest
import xdist.plugin as xdist_plugin

from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #include "test_common.h"

    struct X;

    struct Annotation1 {};
    using XAnnot1 = fruit::Annotated<Annotation1, X>;
    '''

def escape_regex(regex):
    # We un-escape the space because we strip the spaces in fruit_test_common, and this would otherwise leave a
    # stray backslash.
    return re.escape(regex).replace('\\ ', ' ')

def test_success():
    source = '''
        struct X {
          int n;

          X(int n)
            : n(n) {
          }
        };

        fruit::Component<X> getComponent(X* x) {
          return fruit::createComponent()
            .bindInstance(*x);
        }

        int main() {
          X x(34);
          fruit::Injector<X> injector(getComponent, &x);
          X& x1 = injector.get<X&>();
          Assert(&x == &x1);
        }
        '''
    expect_success(COMMON_DEFINITIONS, source)

def test_success_annotated():
    source = '''
        struct X {
          int n;

          X(int n)
            : n(n) {
          }
        };

        fruit::Component<XAnnot1> getComponent(X* x) {
          return fruit::createComponent()
            .bindInstance<XAnnot1>(*x);
        }

        int main() {
          X x(34);
          fruit::Injector<XAnnot1> injector(getComponent, &x);
          X& x1 = injector.get<fruit::Annotated<Annotation1, X&>>();
          Assert(&x == &x1);
        }
        '''
    expect_success(COMMON_DEFINITIONS, source)

@pytest.mark.parametrize('XAnnot,XRefAnnot', [
    ('X', 'X&'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X&>'),
])
def test_success_two_explicit_type_arguments(XAnnot, XRefAnnot):
    source = '''
        struct X {
          int n;

          X(int n)
            : n(n) {
          }
        };

        fruit::Component<XAnnot> getComponent(X* x) {
          return fruit::createComponent()
            .bindInstance<XAnnot, X>(*x);
        }

        int main() {
          X x(34);
          fruit::Injector<XAnnot> injector(getComponent, &x);
          X& x1 = injector.get<XRefAnnot>();
          Assert(&x == &x1);
        }
        '''
    expect_success(COMMON_DEFINITIONS, source, locals())

@pytest.mark.parametrize('BaseAnnot,BasePtrAnnot', [
    ('Base', 'Base*'),
    ('fruit::Annotated<Annotation1, Base>', 'fruit::Annotated<Annotation1, Base*>'),
])
def test_bind_instance_to_subclass(BaseAnnot, BasePtrAnnot):
    source = '''
        struct Base {
          virtual void f() = 0;
          virtual ~Base() {
          }
        };

        struct Derived : public Base {
          void f() override {
          }
        };

        fruit::Component<BaseAnnot> getComponent(Derived* derived) {
          return fruit::createComponent()
            .bindInstance<BaseAnnot>(*derived);
        }

        int main() {
          Derived derived;
          fruit::Injector<BaseAnnot> injector(getComponent, &derived);
          Base* base = injector.get<BasePtrAnnot>();
          base->f();
        }
        '''
    expect_success(COMMON_DEFINITIONS, source, locals())

if __name__== '__main__':
    main(__file__)
