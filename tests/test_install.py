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

from nose2.tools.params import params

from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #include "test_common.h"

    struct X;

    struct Annotation1 {};
    using XAnnot1 = fruit::Annotated<Annotation1, X>;
    '''

def test_success():
    source = '''
        struct X {
          int n;
          X(int n) : n(n) {}
        };

        fruit::Component<X> getParentComponent() {
          return fruit::createComponent()
            .registerProvider([]() { return X(5); });
        }

        fruit::Component<X> getComponent() {
          return fruit::createComponent()
            .install(getParentComponent());
        }

        int main() {
          fruit::Injector<X> injector(getComponent());
          X x = injector.get<X>();
          Assert(x.n == 5);
        }
        '''
    expect_success(COMMON_DEFINITIONS, source)

def test_with_requirements_success():
    source = '''
        struct X {
          int n;
          X(int n) : n(n) {}
        };

        struct Y {
          X x;
          Y(X x): x(x) {}
        };

        fruit::Component<fruit::Required<X>, Y> getParentYComponent() {
          return fruit::createComponent()
            .registerProvider([](X x) { return Y(x); });
        }

        fruit::Component<fruit::Required<X>, Y> getYComponent() {
          return fruit::createComponent()
            .install(getParentYComponent());
        }

        fruit::Component<Y> getComponent() {
          return fruit::createComponent()
            .registerProvider([]() { return X(5); })
            .install(getYComponent());
        }

        int main() {
          fruit::Injector<Y> injector(getComponent());
          Y y = injector.get<Y>();
          Assert(y.x.n == 5);
        }
        '''
    expect_success(COMMON_DEFINITIONS, source)

def test_with_requirements_not_specified_in_child_component_error():
    source = '''
        struct X {
          int n;
          X(int n) : n(n) {}
        };

        struct Y {
          X x;
          Y(X x): x(x) {}
        };

        fruit::Component<fruit::Required<X>, Y> getParentYComponent() {
          return fruit::createComponent()
            .registerProvider([](X x) { return Y(x); });
        }

        // We intentionally don't have fruit::Required<X> here, we want to test that this results in an error.
        fruit::Component<Y> getYComponent() {
          return fruit::createComponent()
            .install(getParentYComponent());
        }
        '''
    expect_compile_error(
        'NoBindingFoundError<X>',
        'No explicit binding nor C::Inject definition was found for T',
        COMMON_DEFINITIONS,
        source)

if __name__ == '__main__':
    import nose2
    nose2.main()
