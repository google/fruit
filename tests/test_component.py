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

@params(
    ('X', 'X*'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>'))
def test_component_conversion(XAnnot, XPtrAnnot):
    source = '''
        struct X {
          using Inject = X();
        };

        fruit::Component<> getComponent() {
          return fruit::createComponent();
        }

        fruit::Component<XAnnot> getXComponent() {
          return getComponent();
        }

        int main() {
          fruit::Component<XAnnot> component = getXComponent();
          fruit::Injector<XAnnot> injector(component);
          injector.get<XPtrAnnot>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_copy(XAnnot):
    source = '''
        struct X {
          using Inject = X();
        };

        fruit::Component<XAnnot> getComponent() {
          fruit::Component<XAnnot> c = fruit::createComponent();
          fruit::Component<XAnnot> copy = c;
          return copy;
        }

        int main() {
          fruit::Component<XAnnot> component = getComponent();
          fruit::Injector<XAnnot> injector(component);
          injector.get<XAnnot>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_move(XAnnot):
    source = '''
        struct X {
          using Inject = X();
        };

        fruit::Component<XAnnot> getComponent() {
          fruit::Component<XAnnot> c = fruit::createComponent();
          fruit::Component<XAnnot> c1 = std::move(c);
          return c1;
        }

        int main() {
          fruit::Component<XAnnot> component = getComponent();
          fruit::Injector<XAnnot> injector(component);
          injector.get<XAnnot>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_move_partial_component(XAnnot):
    source = '''
        struct X {
          using Inject = X();
        };

        fruit::Component<XAnnot> getComponent() {
          auto c = fruit::createComponent();
          auto c1 = std::move(c);
          return std::move(c1);
        }

        int main() {
          fruit::Component<XAnnot> component = getComponent();
          fruit::Injector<XAnnot> injector(component);
          injector.get<XAnnot>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X*', 'fruit::Annotated<Annotation1, X*>')
def test_error_non_class_type(XPtrAnnot):
    source = '''
        struct X {};

        InstantiateType(fruit::Component<XPtrAnnot>)
        '''
    expect_compile_error(
        'NonClassTypeError<X\*,X>',
        'A non-class type T was specified. Use C instead.',
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_repeated_type(XAnnot):
    source = '''
        struct X {};

        InstantiateType(fruit::Component<XAnnot, XAnnot>)
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

        InstantiateType(fruit::Component<XAnnot1, XAnnot2>)
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_type_required_and_provided(XAnnot):
    source = '''
        struct X {};

        InstantiateType(fruit::Component<fruit::Required<XAnnot>, XAnnot>)
        '''
    expect_compile_error(
        'RepeatedTypesError<XAnnot, XAnnot>',
        'A type was specified more than once.',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_type_required_and_provided_with_different_annotations_ok():
    source = '''
        struct X {};

        InstantiateType(fruit::Component<fruit::Required<XAnnot1>, XAnnot2>)
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_no_binding_found(XAnnot):
    source = '''
        struct X {};

        fruit::Component<XAnnot> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'NoBindingFoundError<XAnnot>',
        'No explicit binding nor C::Inject definition was found for T.',
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_no_binding_found_abstract_class(XAnnot):
    source = '''
        struct X {
          virtual void f() = 0;
        };

        fruit::Component<XAnnot> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'NoBindingFoundForAbstractClassError<XAnnot,X>',
        'No explicit binding was found for T, and note that C is an abstract class',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_error_no_factory_binding_found():
    source = '''
        struct X {};

        fruit::Component<std::function<std::unique_ptr<X>()>> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'NoBindingFoundError<std::function<std::unique_ptr<X(,std::default_delete<X>)?>\((void)?\)>',
        'No explicit binding nor C::Inject definition was found for T.',
        COMMON_DEFINITIONS,
        source)

def test_error_no_factory_binding_found_with_annotation():
    source = '''
        struct X {};

        fruit::Component<fruit::Annotated<Annotation1, std::function<std::unique_ptr<X>()>>> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'NoBindingFoundError<fruit::Annotated<Annotation1,std::function<std::unique_ptr<X(,std::default_delete<X>)?>\((void)?\)>>',
        'No explicit binding nor C::Inject definition was found for T.',
        COMMON_DEFINITIONS,
        source)

if __name__ == '__main__':
    import nose2
    nose2.main()
