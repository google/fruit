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

    struct X;

    struct Annotation1 {};
    using intAnnot1 = fruit::Annotated<Annotation1, int>;
    using XAnnot1 = fruit::Annotated<Annotation1, X>;

    struct Annotation2 {};
    using intAnnot2 = fruit::Annotated<Annotation2, int>;
    using XAnnot2 = fruit::Annotated<Annotation2, X>;
    '''

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_component(XAnnot):
    source = '''
        struct X {};

        fruit::Component<XAnnot, XAnnot> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'RepeatedTypesError<XAnnot,XAnnot>',
        'A type was specified more than once.',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_component_with_different_annotation_ok():
    source = '''
        struct X {};

        fruit::Component<XAnnot1, XAnnot2> getComponent() {
          return fruit::createComponent()
            .registerConstructor<XAnnot1()>()
            .registerConstructor<XAnnot2()>();
        }

        int main() {
          fruit::Injector<XAnnot1, XAnnot2> injector(getComponent());
          injector.get<XAnnot1>();
          injector.get<XAnnot2>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_component_in_required_with_annotations(XAnnot):
    source = '''
        struct X {};

        fruit::Component<fruit::Required<XAnnot, XAnnot>> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'RepeatedTypesError<XAnnot,XAnnot>',
        'A type was specified more than once.',
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_component_between_required_and_provided(XAnnot):
    source = '''
        struct X {};

        fruit::Component<fruit::Required<XAnnot>, XAnnot> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'RepeatedTypesError<XAnnot,XAnnot>',
        'A type was specified more than once.',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_component_between_required_and_provided_with_different_annotation_ok():
    source = '''
        struct X {
          using Inject = X();
        };

        fruit::Component<fruit::Required<XAnnot1>, XAnnot2> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params('int', 'fruit::Annotated<Annotation1, int>')
def test_normalized_component_with_annotations(intAnnot):
    source = '''
        InstantiateType(fruit::NormalizedComponent<intAnnot, intAnnot>)
        '''
    expect_compile_error(
        'RepeatedTypesError<intAnnot, intAnnot>',
        'A type was specified more than once.',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_normalized_component_with_different_annotations_ok():
    source = '''
        InstantiateType(fruit::NormalizedComponent<intAnnot1, intAnnot2>)
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

if __name__ == '__main__':
    import nose2
    nose2.main()
