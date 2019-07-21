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
    '''

class TestMultibindingsBindInstance(parameterized.TestCase):
    @parameterized.parameters([
        'X',
        'fruit::Annotated<Annotation1, X>',
    ])
    def test_multibindings_bind_instance_ok(self, XAnnot):
        source = '''
            struct X {};
    
            X x;
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addInstanceMultibinding<XAnnot, X>(x);
            }
    
            int main() {
              fruit::Injector<> injector(getComponent);
    
              std::vector<X*> multibindings = injector.getMultibindings<XAnnot>();
              Assert(multibindings.size() == 1);
              Assert(multibindings[0] == &x);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        'X',
        'fruit::Annotated<Annotation1, X>',
    ])
    def test_multibindings_bind_const_instance_error(self, XAnnot):
        source = '''
            struct X {};
    
            const X x{};
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addInstanceMultibinding<XAnnot, X>(x);
            }
            '''
        expect_generic_compile_error(
            r'candidate function not viable: 1st argument \(.const X.\) would lose const qualifier'
            r'|no matching function for call to .fruit::PartialComponent<.*>::addInstanceMultibinding(<XAnnot,X>)?\(const X&\).'
            r'|error: no matching member function for call to .addInstanceMultibinding.'
            r'|cannot convert argument 1 from .const X. to .X &.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        'X',
        'fruit::Annotated<Annotation1, X>',
    ])
    def test_multibindings_bind_instance_vector(self, XAnnot):
        source = '''
            struct X {};
    
            std::vector<X> values = {X(), X()};
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addInstanceMultibindings<XAnnot, X>(values);
            }
    
            int main() {
              fruit::Injector<> injector(getComponent);
    
              std::vector<X*> multibindings = injector.getMultibindings<XAnnot>();
              Assert(multibindings.size() == 2);
              Assert(multibindings[0] == &(values[0]));
              Assert(multibindings[1] == &(values[1]));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        'X',
        'fruit::Annotated<Annotation1, X>',
    ])
    def test_multibindings_bind_const_instance_vector_error(self, XAnnot):
        source = '''
            struct X {};
    
            const std::vector<X> values{};
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addInstanceMultibindings<XAnnot, X>(values);
            }
            '''
        expect_generic_compile_error(
            r'candidate function not viable: 1st argument \(.const std::vector<X>.\) would lose const qualifier'
            r'|cannot convert .values. \(type .const std::(__debug::)?vector<X>.\) to type .std::(__debug::)?vector<X>&.'
            r'|no matching .*function for call to .*addInstanceMultibindings'
            r'|cannot convert argument 1 from .const std::vector<X,std::allocator<.*>>. to .std::vector<X,std::allocator<.*>> &.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        'X',
        'fruit::Annotated<Annotation1, X>',
    ])
    def test_multibindings_bind_instance_vector_of_consts_error(self, XAnnot):
        source = '''
            struct X {};
    
            std::vector<const X> values;
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addInstanceMultibindings<XAnnot, X>(values);
            }
            '''
        expect_generic_compile_error(
            '.*',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X**', r'X\*\*'),
        ('std::shared_ptr<X>*', r'std::shared_ptr<X>\*'),
        ('const std::shared_ptr<X>', r'const std::shared_ptr<X>'),
        ('X* const', r'X\* const'),
        ('const X* const', r'const X\* const'),
        ('X*&', r'X\*&'),
        ('fruit::Annotated<Annotation1, X**>', r'X\*\*'),
    ])
    def test_multibindings_bind_instance_non_class_type_error(self, XVariant, XVariantRegex):
        source = '''
            struct X {};
    
            using XVariantT = XVariant;
            fruit::Component<> getComponent(XVariantT x) {
              return fruit::createComponent()
                .addInstanceMultibinding<XVariant, XVariant>(x);
            }
            '''
        expect_compile_error(
            'NonClassTypeError<XVariantRegex,X>',
            'A non-class type T was specified.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('std::nullptr_t', r'(std::)?nullptr(_t)?'),
        ('X(*)()', r'X(\((__cdecl)?\*\))?\((void)?\)'),
    ])
    def test_multibindings_bind_instance_non_injectable_type_error(self, XVariant, XVariantRegex):
        source = '''
            struct X {};
    
            using XVariantT = XVariant;
            fruit::Component<> getComponent(XVariantT x) {
              return fruit::createComponent()
                .addInstanceMultibinding<XVariant, XVariant>(x);
            }
            '''
        expect_compile_error(
            'NonInjectableTypeError<XVariantRegex>',
            'The type T is not injectable.',
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
