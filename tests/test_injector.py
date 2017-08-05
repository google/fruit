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
          fruit::Injector<> injector(getComponent);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@pytest.mark.parametrize('XAnnot', [
    'X',
    'fruit::Annotated<Annotation1, X>',
])
def test_error_component_with_requirements(XAnnot):
    source = '''
        struct X {};
    
        fruit::Component<fruit::Required<XAnnot>> getComponent();

        void f(fruit::NormalizedComponent<XAnnot> normalizedComponent) {
          fruit::Injector<XAnnot> injector(normalizedComponent, getComponent);
        }
        '''
    expect_compile_error(
        'ComponentWithRequirementsInInjectorError<XAnnot>',
        'When using the two-argument constructor of Injector, the component used as second parameter must not have requirements',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('XAnnot', [
    'X',
    'fruit::Annotated<Annotation1, X>',
])
def test_error_declared_types_not_provided(XAnnot):
    source = '''
        struct X {
          using Inject = X();
        };
        
        fruit::Component<> getEmptyComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::NormalizedComponent<> normalizedComponent(getEmptyComponent);
          fruit::Injector<XAnnot> injector(normalizedComponent, getEmptyComponent);
        }
        '''
    expect_compile_error(
        'TypesInInjectorNotProvidedError<XAnnot>',
        'The types in TypesNotProvided are declared as provided by the injector, but none of the two components passed to the Injector constructor provides them.',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('XAnnot,ConstXAnnot', [
    ('X', 'const X'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>'),
])
def test_error_declared_nonconst_types_provided_as_const(XAnnot, ConstXAnnot):
    source = '''
        struct X {
          using Inject = X();
        };
        
        fruit::Component<ConstXAnnot> getComponent();

        int main() {
          fruit::Injector<XAnnot> injector(getComponent);
        }
        '''
    expect_generic_compile_error(
        'no matching constructor for initialization of .fruit::Injector<XAnnot>.'
        '|no matching function for call to .fruit::Injector<XAnnot>::Injector\(fruit::Component<ConstXAnnot> \(&\)\(\)\).'
        # MSVC
        '|.fruit::Injector<XAnnot>::Injector.: none of the 2 overloads could convert all the argument types',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('XAnnot,ConstXAnnot', [
    ('X', 'const X'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>'),
])
def test_error_declared_nonconst_types_provided_as_const_with_normalized_component(XAnnot, ConstXAnnot):
    source = '''
        struct X {};
        
        fruit::Component<> getEmptyComponent();
        
        void f(fruit::NormalizedComponent<ConstXAnnot> normalizedComponent) {
          fruit::Injector<XAnnot> injector(normalizedComponent, getEmptyComponent);
        }
        '''
    expect_compile_error(
        'TypesInInjectorProvidedAsConstOnlyError<XAnnot>',
        'The types in TypesProvidedAsConstOnly are declared as non-const provided types by the injector',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('XAnnot,YAnnot', [
    ('X', 'Y'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, Y>'),
])
def test_injector_get_error_type_not_provided(XAnnot, YAnnot):
    source = '''
        struct X {
          using Inject = X();
        };

        struct Y {};

        fruit::Component<XAnnot> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<XAnnot> injector(getComponent);
          injector.get<YAnnot>();
        }
        '''
    expect_compile_error(
        'TypeNotProvidedError<YAnnot>',
        'Trying to get an instance of T, but it is not provided by this Provider/Injector.',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstXAnnot,XInjectorGetParam,XInjectorGetParamRegex', [
    ('const X', 'X&', 'X&'),
    ('const X', 'X*', 'X\*'),
    ('const X', 'std::shared_ptr<X>', 'std::shared_ptr<X>'),
    ('fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, X&>', 'fruit::Annotated<Annotation1, X&>'),
    ('fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, X*>', 'fruit::Annotated<Annotation1, X\*>'),
    ('fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, std::shared_ptr<X>>', 'fruit::Annotated<Annotation1, std::shared_ptr<X>>'),
])
def test_injector_const_provided_type_does_not_allow_injecting_nonconst_variants(ConstXAnnot, XInjectorGetParam, XInjectorGetParamRegex):
    source = '''
        void f(fruit::Injector<ConstXAnnot> injector) {
          injector.get<XInjectorGetParam>();
        }
        '''
    expect_compile_error(
        'TypeProvidedAsConstOnlyError<XInjectorGetParamRegex>',
        'Trying to get an instance of T, but it is only provided as a constant by this Provider/Injector',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('XBindingInInjector,XInjectorGetParam', [
    ('X', 'X'),
    ('X', 'const X&'),
    ('X', 'const X*'),
    ('X', 'X&'),
    ('X', 'X*'),
    ('X', 'std::shared_ptr<X>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X&>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X*>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X&>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, std::shared_ptr<X>>'),
])
def test_injector_get_ok(XBindingInInjector, XInjectorGetParam):
    source = '''
        struct X {
          using Inject = X();
        };

        fruit::Component<XBindingInInjector> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<XBindingInInjector> injector(getComponent);

          auto x = injector.get<XInjectorGetParam>();
          (void)x;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('XBindingInInjector,XInjectorGetParam', [
    ('const X', 'X'),
    ('const X', 'const X&'),
    ('const X', 'const X*'),
    ('fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, X>'),
    ('fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, const X&>'),
    ('fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, const X*>'),
])
def test_injector_get_const_binding_ok(XBindingInInjector, XInjectorGetParam):
    XBindingInInjectorWithoutConst = XBindingInInjector.replace('const ', '')
    source = '''
        struct X {};
        
        const X x{};

        fruit::Component<XBindingInInjector> getComponent() {
          return fruit::createComponent()
              .bindInstance<XBindingInInjectorWithoutConst, X>(x);
        }

        int main() {
          fruit::Injector<XBindingInInjector> injector(getComponent);

          auto x = injector.get<XInjectorGetParam>();
          (void)x;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('XVariant,XVariantRegex', [
    ('X**', r'X\*\*'),
    ('std::shared_ptr<X>*', r'std::shared_ptr<X>\*'),
    ('const std::shared_ptr<X>', r'const std::shared_ptr<X>'),
    ('X* const', r'X\* const'),
    ('const X* const', r'const X\* const'),
    ('std::nullptr_t', r'(std::)?nullptr(_t)?'),
    ('X*&', r'X\*&'),
    ('X(*)()', r'X(\((__cdecl)?\*\))?\((void)?\)'),
    ('void', r'void'),
    ('fruit::Annotated<Annotation1, X**>', r'X\*\*'),
])
def test_injector_get_error_type_not_injectable(XVariant,XVariantRegex):
    source = '''
        struct X {};

        void f(fruit::Injector<X> injector) {
          injector.get<XVariant>();
        }
        '''
    expect_compile_error(
        'NonInjectableTypeError<XVariantRegex>',
        'The type T is not injectable.',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('XVariant,XVariantRegex', [
    ('X[]', r'X\[\]'),
])
def test_injector_get_error_array_type(XVariant,XVariantRegex):
    source = '''
        struct X {};

        void f(fruit::Injector<X> injector) {
          injector.get<XVariant>();
        }
        '''
    expect_generic_compile_error(
        'function cannot return array type'
        '|function returning an array'
        # MSVC
        '|.fruit::Injector<X>::get.: no matching overloaded function found',
        COMMON_DEFINITIONS,
        source,
        locals())

if __name__== '__main__':
    main(__file__)
