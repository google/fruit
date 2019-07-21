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

    struct X {};
    struct Y {};

    struct Annotation1 {};
    using IntAnnot1 = fruit::Annotated<Annotation1, int>;
    using XAnnot1 = fruit::Annotated<Annotation1, X>;

    struct Annotation2 {};
    using IntAnnot2 = fruit::Annotated<Annotation2, int>;
    using XAnnot2 = fruit::Annotated<Annotation2, X>;
    '''

class TestComponentAndInjectorParams(parameterized.TestCase):
    @multiple_parameters([
        ('X', 'X'),
        ('X', 'const X'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>'),
    ], [
        'Component',
        'NormalizedComponent',
        'Injector',
    ])
    def test_duplicate_type(self, XAnnot, MaybeConstXAnnot, Class):
        source = '''
            InstantiateType(fruit::Class<MaybeConstXAnnot, MaybeConstXAnnot>)
            '''
        expect_compile_error(
            'RepeatedTypesError<XAnnot,XAnnot>',
            'A type was specified more than once.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @multiple_parameters([
        ('X', 'const X'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>'),
    ], [
        'Component',
        'NormalizedComponent',
        'Injector',
    ])
    def test_duplicate_type_different_constness(self, XAnnot, ConstXAnnot, Class):
        source = '''
            InstantiateType(fruit::Class<XAnnot, ConstXAnnot>)
            '''
        expect_compile_error(
            'RepeatedTypesError<XAnnot,XAnnot>',
            'A type was specified more than once.',
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_duplicate_type_with_different_annotation_ok(self):
        source = '''
            fruit::Component<XAnnot1, XAnnot2> getComponent() {
              return fruit::createComponent()
                .registerConstructor<XAnnot1()>()
                .registerConstructor<XAnnot2()>();
            }
    
            int main() {
              fruit::Injector<XAnnot1, XAnnot2> injector1(getComponent);
              injector1.get<XAnnot1>();
              injector1.get<XAnnot2>();
              
              fruit::NormalizedComponent<XAnnot1, XAnnot2> normalizedComponent(getComponent);
              fruit::Injector<XAnnot1, XAnnot2> injector2(getComponent);
              injector2.get<XAnnot1>();
              injector2.get<XAnnot2>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source)

    @multiple_parameters([
        ('X', 'X'),
        ('X', 'const X'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>'),
    ], [
        'Component',
        'NormalizedComponent',
    ])
    def test_duplicate_type_in_required(self, XAnnot, MaybeConstXAnnot, Class):
        source = '''
            InstantiateType(fruit::Class<fruit::Required<MaybeConstXAnnot, MaybeConstXAnnot>>)
            '''
        expect_compile_error(
            'RepeatedTypesError<XAnnot,XAnnot>',
            'A type was specified more than once.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @multiple_parameters([
        'Component',
        'NormalizedComponent',
    ], [
        ('X', 'const X'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>'),
    ])
    def test_component_duplicate_type_in_required_different_constness(self, Class, XAnnot, ConstXAnnot):
        source = '''
            InstantiateType(fruit::Class<fruit::Required<XAnnot, ConstXAnnot>>)
            '''
        expect_compile_error(
            'RepeatedTypesError<XAnnot,XAnnot>',
            'A type was specified more than once.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @multiple_parameters([
        ('X', 'X'),
        ('X', 'const X'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>'),
    ], [
        'Component',
        'NormalizedComponent',
    ])
    def test_same_type_in_required_and_provided(self, XAnnot, MaybeConstXAnnot, Class):
        source = '''
            InstantiateType(fruit::Class<fruit::Required<MaybeConstXAnnot>, MaybeConstXAnnot>)
            '''
        expect_compile_error(
            'RepeatedTypesError<XAnnot,XAnnot>',
            'A type was specified more than once.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @multiple_parameters([
        ('X', 'X', 'const X'),
        ('X', 'const X', 'X'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, X>'),
    ], [
        'Component',
        'NormalizedComponent',
    ])
    def test_same_type_in_required_and_provided_different_constness(self, XAnnot, XAnnotInRequirements, XAnnotInProvides, Class):
        source = '''
            InstantiateType(fruit::Class<fruit::Required<XAnnotInRequirements>, XAnnotInProvides>)
            '''
        expect_compile_error(
            'RepeatedTypesError<XAnnot,XAnnot>',
            'A type was specified more than once.',
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_same_type_in_required_and_provided_different_annotation_ok(self):
        source = '''
            fruit::Component<fruit::Required<XAnnot1>, XAnnot2> getComponent() {
              return fruit::createComponent()
                .registerConstructor<XAnnot2()>();
            }
            
            fruit::Component<XAnnot1, XAnnot2> getRootComponent() {
              return fruit::createComponent()
                  .install(getComponent)
                  .registerConstructor<XAnnot1()>();
            }
            
            fruit::Component<> getEmptyComponent() {
              return fruit::createComponent();
            }
            
            int main() {
              fruit::Injector<XAnnot1, XAnnot2> injector1(getRootComponent);
              injector1.get<XAnnot1>();
              injector1.get<XAnnot2>();
              
              fruit::NormalizedComponent<XAnnot1, XAnnot2> normalizedComponent(getRootComponent);
              fruit::Injector<XAnnot1, XAnnot2> injector2(normalizedComponent, getEmptyComponent);
              injector2.get<XAnnot1>();
              injector2.get<XAnnot2>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source)

    @multiple_parameters([
        ('X*', r'X\*'),
        ('const X*', r'const X\*'),
        ('X&', r'X&'),
        ('const X&', r'const X&'),
        ('std::shared_ptr<X>', r'std::shared_ptr<X>'),
        ('fruit::Annotated<Annotation1, X*>', r'X\*'),
        ('fruit::Annotated<Annotation1, const X*>', r'const X\*'),
        ('fruit::Annotated<Annotation1, X&>', r'X&'),
        ('fruit::Annotated<Annotation1, const X&>', r'const X&'),
        ('fruit::Annotated<Annotation1, std::shared_ptr<X>>', r'std::shared_ptr<X>'),
    ], [
        'Component',
        'NormalizedComponent',
        'Injector',
    ])
    def test_error_non_class_type(self, XVariantAnnot, XVariantRegexp, Class):
        source = '''
            InstantiateType(fruit::Class<XVariantAnnot>)
            '''
        expect_compile_error(
            'NonClassTypeError<XVariantRegexp,X>',
            'A non-class type T was specified. Use C instead.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @multiple_parameters([
        ('const X', 'const X'),
        ('fruit::Annotated<Annotation1, const X>', 'const X'),
    ], [
        'Component',
        'NormalizedComponent',
        'Injector',
    ])
    def test_const_provided_type_ok(self, XVariantAnnot, XVariantRegexp, Class):
        source = '''
            InstantiateType(fruit::Class<XVariantAnnot>)
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @multiple_parameters([
        ('X*', r'X\*'),
        ('const X*', r'const X\*'),
        ('X&', r'X&'),
        ('const X&', r'const X&'),
        ('std::shared_ptr<X>', r'std::shared_ptr<X>'),
        ('fruit::Annotated<Annotation1, X*>', r'X\*'),
        ('fruit::Annotated<Annotation1, const X*>', r'const X\*'),
        ('fruit::Annotated<Annotation1, X&>', r'X&'),
        ('fruit::Annotated<Annotation1, const X&>', r'const X&'),
        ('fruit::Annotated<Annotation1, std::shared_ptr<X>>', r'std::shared_ptr<X>'),
    ], [
        'Component',
        'NormalizedComponent',
    ])
    def test_error_non_class_type_in_requirements(self, XVariantAnnot, XVariantRegexp, Class):
        source = '''
            InstantiateType(fruit::Class<fruit::Required<XVariantAnnot>>)
            '''
        expect_compile_error(
            'NonClassTypeError<XVariantRegexp,X>',
            'A non-class type T was specified. Use C instead.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('const Z', 'Z'),
        ('fruit::Annotated<Annotation1, const Z>', 'fruit::Annotated<Annotation1, Z>'),
    ])
    def test_const_class_type_ok(self, ConstZAnnot, ZAnnot):
        source = '''
            struct Z {};
            
            const Z z{};
    
            fruit::Component<ConstZAnnot> getComponent() {
              return fruit::createComponent()
                  .bindInstance<ZAnnot, Z>(z);
            }
            
            fruit::Component<> getEmptyComponent() {
              return fruit::createComponent();
            }
            
            int main() {
              fruit::NormalizedComponent<ConstZAnnot> normalizedComponent(getComponent);
              fruit::Injector<ConstZAnnot> injector(normalizedComponent, getEmptyComponent);
              injector.get<ZAnnot>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('const Z', 'Z'),
        ('fruit::Annotated<Annotation1, const Z>', 'fruit::Annotated<Annotation1, Z>'),
    ])
    def test_const_class_type_in_requirements_ok(self, ConstZAnnot, ZAnnot):
        source = '''
            struct Z {};
    
            fruit::Component<fruit::Required<ConstZAnnot>> getComponent() {
              return fruit::createComponent();
            }
            
            const Z z{};
            
            fruit::Component<ConstZAnnot> getEmptyComponent() {
              return fruit::createComponent()
                  .bindInstance<ZAnnot, Z>(z);
            }
            
            int main() {
              fruit::NormalizedComponent<fruit::Required<ConstZAnnot>> normalizedComponent(getComponent);
              fruit::Injector<ConstZAnnot> injector(normalizedComponent, getEmptyComponent);
              injector.get<ZAnnot>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        'Component',
        'NormalizedComponent',
    ])
    def test_two_required_lists_error(self, Class):
        source = '''
            InstantiateType(fruit::Class<fruit::Required<X>, fruit::Required<Y>>)
        '''
        expect_compile_error(
            'RequiredTypesInComponentArgumentsError<fruit::Required<Y>>',
            'A Required<...> type was passed as a non-first template parameter to fruit::Component or fruit::NormalizedComponent',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        'Component',
        'NormalizedComponent',
    ])
    def test_required_list_not_first_argument_error(self, Class):
        source = '''
            InstantiateType(fruit::Class<X, fruit::Required<Y>>)
        '''
        expect_compile_error(
            'RequiredTypesInComponentArgumentsError<fruit::Required<Y>>',
            'A Required<...> type was passed as a non-first template parameter to fruit::Component or fruit::NormalizedComponent',
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_multiple_required_types_ok(self):
        source = '''
            fruit::Component<fruit::Required<X, Y>> getEmptyComponent() {
              return fruit::createComponent();
            }
    
            fruit::Component<X, Y> getComponent() {
              return fruit::createComponent()
                  .install(getEmptyComponent)
                  .registerConstructor<X()>()
                  .registerConstructor<Y()>();
            }
    
            int main() {
              fruit::NormalizedComponent<fruit::Required<X, Y>> normalizedComponent(getEmptyComponent);
              fruit::Injector<X> injector(normalizedComponent, getComponent);
              injector.get<X>();
            }
        '''
        expect_success(
            COMMON_DEFINITIONS,
            source)

    @parameterized.parameters([
        ('X', 'Y'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, Y>'),
    ])
    def test_error_requirements_in_injector(self, XAnnot, YAnnot):
        source = '''
            InstantiateType(fruit::Injector<fruit::Required<YAnnot>, XAnnot>)
            '''
        expect_compile_error(
            'InjectorWithRequirementsError<YAnnot>',
            'Injectors can.t have requirements.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X', 'Y'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, Y>'),
    ])
    def test_error_requirements_in_injector_second_argument(self, XAnnot, YAnnot):
        source = '''
            InstantiateType(fruit::Injector<XAnnot, fruit::Required<YAnnot>>)
            '''
        expect_compile_error(
            'InjectorWithRequirementsError<YAnnot>',
            'Injectors can.t have requirements.',
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
