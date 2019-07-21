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
from absl.testing import parameterized
from fruit_test_common import *
from fruit_test_config import CXX_COMPILER_NAME
import re

COMMON_DEFINITIONS = '''
    #include "test_common.h"

    struct Annotation {};
    struct Annotation1 {};
    struct Annotation2 {};
    '''

class TestMultibindingsBindInterface(parameterized.TestCase):
    @parameterized.parameters([
        ('X', 'XImpl'),
        ('X', 'fruit::Annotated<Annotation2, XImpl>'),
        ('fruit::Annotated<Annotation1, X>', 'XImpl'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, XImpl>'),
    ])
    def test_add_interface_multibinding_success(self, XAnnot, XImplAnnot):
        source = '''
            struct X {
              virtual int foo() = 0;
            };
    
            struct XImpl : public X {
              INJECT(XImpl()) = default;
    
              int foo() override {
                return 5;
              }
            };
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addMultibinding<XAnnot, XImplAnnot>();
            }
            
            int main() {
              fruit::Injector<> injector(getComponent);
    
              std::vector<X*> multibindings = injector.getMultibindings<XAnnot>();
              Assert(multibindings.size() == 1);
              Assert(multibindings[0]->foo() == 5);
            }        
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X', 'XImpl', 'const XImpl'),
        ('X', 'fruit::Annotated<Annotation2, XImpl>', 'fruit::Annotated<Annotation2, const XImpl>'),
    ])
    def test_add_interface_multibinding_const_target_error_install_first(self, XAnnot, XImplAnnot, ConstXImplAnnot):
        source = '''
            struct X {
              virtual int foo() = 0;
            };
    
            struct XImpl : public X {
              int foo() override {
                return 5;
              }
            };
            
            fruit::Component<ConstXImplAnnot> getXImplComponent();
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .install(getXImplComponent)
                .addMultibinding<XAnnot, XImplAnnot>();
            }
            '''
        expect_compile_error(
            'NonConstBindingRequiredButConstBindingProvidedError<XImplAnnot>',
            'The type T was provided as constant, however one of the constructors/providers/factories in this component',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X', 'XImpl', 'const XImpl'),
        ('X', 'fruit::Annotated<Annotation2, XImpl>', 'fruit::Annotated<Annotation2, const XImpl>'),
    ])
    def test_add_interface_multibinding_const_target_error_binding_first(self, XAnnot, XImplAnnot, ConstXImplAnnot):
        source = '''
            struct X {
              virtual int foo() = 0;
            };
    
            struct XImpl : public X {
              int foo() override {
                return 5;
              }
            };
            
            fruit::Component<ConstXImplAnnot> getXImplComponent();
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addMultibinding<XAnnot, XImplAnnot>()
                .install(getXImplComponent);
            }
            '''
        expect_compile_error(
            'NonConstBindingRequiredButConstBindingProvidedError<XImplAnnot>',
            'The type T was provided as constant, however one of the constructors/providers/factories in this component',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X', 'int'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, int>'),
    ])
    def test_error_not_base(self, XAnnot, intAnnot):
        source = '''
            struct X {};
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addMultibinding<XAnnot, intAnnot>();
            }
            '''
        expect_compile_error(
            'NotABaseClassOfError<X,int>',
            'I is not a base class of C.',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('Scaler', 'ScalerImpl'),
        ('fruit::Annotated<Annotation1, Scaler>', 'fruit::Annotated<Annotation2, ScalerImpl>'),
    ])
    def test_error_abstract_class(self, ScalerAnnot, ScalerImplAnnot):
        source = '''
            struct Scaler {
              virtual double scale(double x) = 0;
            };
    
            struct ScalerImpl : public Scaler {
              // Note: here we "forgot" to implement scale() (on purpose, for this test) so ScalerImpl is an abstract class.
            };
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addMultibinding<ScalerAnnot, ScalerImplAnnot>();
            }
            '''
        expect_compile_error(
            'NoBindingFoundForAbstractClassError<ScalerImplAnnot,ScalerImpl>',
            'No explicit binding was found for T, and note that C is an abstract class',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('Scaler', 'ScalerImpl'),
        ('fruit::Annotated<Annotation1, Scaler>', 'fruit::Annotated<Annotation2, ScalerImpl>'),
    ])
    def test_error_abstract_class_clang(self, ScalerAnnot, ScalerImplAnnot):
        if re.search('Clang', CXX_COMPILER_NAME) is None:
            # This is Clang-only because GCC >=4.9 refuses to even mention the type C() when C is an abstract class,
            # while Clang allows to mention the type (but of course there can be no functions with this type)
            return
        source = '''
            struct Scaler {
              virtual double scale(double x) = 0;
            };
    
            struct ScalerImpl : public Scaler {
              INJECT(ScalerImpl()) = default;
    
              // Note: here we "forgot" to implement scale() (on purpose, for this test) so ScalerImpl is an abstract class.
            };
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .addMultibinding<ScalerAnnot, ScalerImplAnnot>();
            }
            '''
        expect_compile_error(
            'CannotConstructAbstractClassError<ScalerImpl>',
            'The specified class can.t be constructed because it.s an abstract class.',
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
