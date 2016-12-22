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
    using XAnnot1 = fruit::Annotated<Annotation1, X>;

    struct Annotation2 {};
    using XAnnot2 = fruit::Annotated<Annotation2, X>;
    '''

def test_empty_injector():
    source = '''
        fruit::Component<> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<> injector(getComponent());
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_component_with_requirements(XAnnot):
    source = '''
        struct X {
          using Inject = X();
        };

        fruit::Component<fruit::Required<XAnnot>> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::NormalizedComponent<XAnnot> normalizedComponent(fruit::createComponent());
          fruit::Injector<XAnnot> injector(normalizedComponent, getComponent());
        }
        '''
    expect_compile_error(
        'ComponentWithRequirementsInInjectorError<XAnnot>',
        'When using the two-argument constructor of Injector, the component used as second parameter must not have requirements',
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_types_not_provided(XAnnot):
    source = '''
        struct X {
          using Inject = XAnnot();
        };

        int main() {
          fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
          fruit::Injector<XAnnot> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));
        }
        '''
    expect_compile_error(
        'TypesInInjectorNotProvidedError<XAnnot>',
        'The types in TypesNotProvided are declared as provided by the injector, but none of the two components passed to the Injector constructor provides them.',
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_repeated_type(XAnnot):
    source = '''
        struct X {};

        InstantiateType(fruit::Injector<XAnnot, XAnnot>)
        '''
    expect_compile_error(
        'RepeatedTypesError<XAnnot, XAnnot>',
        'A type was specified more than once.',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_repeated_type_with_different_annotation_ok():
    source = '''
        struct X {};

        InstantiateType(fruit::Injector<XAnnot1, XAnnot2>)
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params('X*', 'fruit::Annotated<Annotation1, X*>')
def test_error_non_class_type(XPtrAnnot):
    source = '''
        struct X {};

        InstantiateType(fruit::Injector<XPtrAnnot>)
        '''
    expect_compile_error(
        'NonClassTypeError<X\*,X>',
        'A non-class type T was specified. Use C instead.',
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'Y'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, Y>'))
def test_error_requirements_in_injector_with_annotation(XAnnot, YAnnot):
    source = '''
        struct Y {};

        struct X {
          using Inject = XAnnot(YAnnot);
          X(Y) {
          }
        };

        fruit::Component<fruit::Required<YAnnot>, XAnnot> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<fruit::Required<YAnnot>, XAnnot> injector(getComponent());
        }
        '''
    expect_compile_error(
        'InjectorWithRequirementsError<YAnnot>',
        'Injectors can.t have requirements.',
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'Y'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, Y>'))
def test_error_type_not_provided_with_annotation(XAnnot, YAnnot):
    source = '''
        struct X {
          using Inject = X();
        };

        struct Y {};

        fruit::Component<XAnnot> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<XAnnot> injector(getComponent());
          injector.get<YAnnot>();
        }
        '''
    expect_compile_error(
        'TypeNotProvidedError<YAnnot>',
        'Trying to get an instance of T, but it is not provided by this Provider/Injector.',
        COMMON_DEFINITIONS,
        source,
        locals())

if __name__ == '__main__':
    import nose2
    nose2.main()
