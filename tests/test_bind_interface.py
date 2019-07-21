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
    struct Annotation2 {};
    '''

class TestBindInstance(parameterized.TestCase):
    @parameterized.parameters([
        ('X', 'X', 'const X&', 'Y'),
        ('X', 'const X', 'const X&', 'Y'),
        ('X', 'X', 'const X&', 'fruit::Annotated<Annotation1, Y>'),
        ('X', 'const X', 'const X&', 'fruit::Annotated<Annotation1, Y>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X&>', 'Y'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, const X&>', 'Y'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X&>', 'fruit::Annotated<Annotation1, Y>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, const X&>', 'fruit::Annotated<Annotation1, Y>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X&>', 'fruit::Annotated<Annotation2, Y>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, const X&>', 'fruit::Annotated<Annotation2, Y>'),
    ])
    def test_bind_interface(self, XAnnot, MaybeConstXAnnot, XConstRefAnnot, YAnnot):
        source = '''
            struct X {
              virtual void f() const = 0;
            };
    
            struct Y : public X {
              INJECT(Y()) = default;
              
              void f() const override {
              }
            };
    
            fruit::Component<MaybeConstXAnnot> getComponent() {
              return fruit::createComponent()
                .bind<XAnnot, YAnnot>();
            }
    
            int main() {
              fruit::Injector<MaybeConstXAnnot> injector(getComponent);
              const X& x = injector.get<XConstRefAnnot>();
              x.f();
            }
        '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X', 'const X', 'const X&', 'Y'),
        ('X', 'const X', 'const X&', 'fruit::Annotated<Annotation1, Y>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, const X&>', 'Y'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, const X&>', 'fruit::Annotated<Annotation1, Y>'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, const X>', 'fruit::Annotated<Annotation1, const X&>', 'fruit::Annotated<Annotation2, Y>'),
    ])
    def test_bind_interface_to_constant(self, XAnnot, ConstXAnnot, XConstRefAnnot, YAnnot):
        source = '''
            struct X {
              virtual void f() const = 0;
            };
    
            struct Y : public X {
              void f() const override {
              }
            };
            
            const Y y{};
    
            fruit::Component<ConstXAnnot> getComponent() {
              return fruit::createComponent()
                .bindInstance<YAnnot, Y>(y)
                .bind<XAnnot, YAnnot>();
            }
    
            int main() {
              fruit::Injector<ConstXAnnot> injector(getComponent);
              const X& x = injector.get<XConstRefAnnot>();
              x.f();
            }
        '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X', 'X&', 'Y'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X&>', 'fruit::Annotated<Annotation2, Y>'),
    ])
    def test_bind_interface_target_bound_in_other_component(self, XAnnot, XRefAnnot, YAnnot):
        source = '''
            struct X {
              virtual void f() = 0;
            };
    
            struct Y : public X {
              void f() override {
              }
            };
    
            fruit::Component<fruit::Required<YAnnot>, XAnnot> getComponent() {
              return fruit::createComponent()
                .bind<XAnnot, YAnnot>();
            }
    
            fruit::Component<XAnnot> getRootComponent() {
              return fruit::createComponent()
                .registerConstructor<YAnnot()>()
                .install(getComponent);
            }
    
            int main() {
              fruit::Injector<XAnnot> injector(getRootComponent);
              X& x = injector.get<XRefAnnot>();
              x.f();
            }
        '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X', 'X&', 'Y'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X&>', 'fruit::Annotated<Annotation2, Y>'),
    ])
    def test_bind_nonconst_interface_requires_nonconst_target(self, XAnnot, XRefAnnot, YAnnot):
        source = '''
            struct X {
              virtual void f() = 0;
            };
    
            struct Y : public X {
              void f() override {
              }
            };
    
            fruit::Component<fruit::Required<const YAnnot>, XAnnot> getComponent() {
              return fruit::createComponent()
                .bind<XAnnot, YAnnot>();
            }
        '''
        expect_compile_error(
            'ConstBindingDeclaredAsRequiredButNonConstBindingRequiredError<YAnnot>',
            'The type T was declared as a const Required type in the returned Component, however a non-const binding',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X', 'Y'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, Y>'),
    ])
    def test_bind_interface_to_constant_nonconst_required_const_bound_error(self, XAnnot, YAnnot):
        source = '''
            struct X {
              virtual void f() const = 0;
            };
    
            struct Y : public X {
              void f() const override {
              }
            };
            
            const Y y{};
    
            fruit::Component<XAnnot> getComponent() {
              return fruit::createComponent()
                .bindInstance<YAnnot, Y>(y)
                .bind<XAnnot, YAnnot>();
            }
        '''
        expect_compile_error(
            'NonConstBindingRequiredButConstBindingProvidedError<YAnnot>',
            'The type T was provided as constant, however one of the constructors/providers/factories in this component',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('X', 'X&', 'Y'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X&>', 'fruit::Annotated<Annotation2, Y>'),
    ])
    def test_bind_nonconst_interface_requires_nonconst_target_abstract(self, XAnnot, XRefAnnot, YAnnot):
        source = '''
            struct X {
              virtual void f() = 0;
            };
    
            struct Y : public X {};
    
            fruit::Component<fruit::Required<const YAnnot>, XAnnot> getComponent() {
              return fruit::createComponent()
                .bind<XAnnot, YAnnot>();
            }
        '''
        expect_compile_error(
            'ConstBindingDeclaredAsRequiredButNonConstBindingRequiredError<YAnnot>',
            'The type T was declared as a const Required type in the returned Component, however a non-const binding',
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
    
            fruit::Component<intAnnot> getComponent() {
              return fruit::createComponent()
                .bind<XAnnot, intAnnot>();
            }
            '''
        expect_compile_error(
            'NotABaseClassOfError<X,int>',
            'I is not a base class of C.',
            COMMON_DEFINITIONS,
            source,
            locals())

    # TODO: maybe the error should include the annotation here.
    @parameterized.parameters([
        'X',
        'fruit::Annotated<Annotation1, X>',
    ])
    def test_error_bound_to_itself(self, XAnnot):
        source = '''
            struct X {};
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .bind<XAnnot, XAnnot>();
            }
            '''
        expect_compile_error(
            'InterfaceBindingToSelfError<X>',
            'The type C was bound to itself.',
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_bound_to_itself_with_annotation_error(self):
        source = '''
            struct X {};
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .registerConstructor<X()>()
                .bind<fruit::Annotated<Annotation1, X>, X>();
            }
            '''
        expect_compile_error(
            'InterfaceBindingToSelfError<X>',
            'The type C was bound to itself.',
            COMMON_DEFINITIONS,
            source)

    def test_bound_chain_ok(self):
        source = '''
            struct X {
              virtual void f() = 0;
            };
    
            struct Y : public X {};
    
            struct Z : public Y {
              INJECT(Z()) = default;
              void f() override {
              }
            };
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .bind<X, Y>()
                .bind<Y, Z>();
            }
    
            int main() {
              fruit::Injector<X> injector(getComponent);
              X& x = injector.get<X&>();
              x.f();
            }
        '''
        expect_success(COMMON_DEFINITIONS, source)

    def test_bind_non_normalized_types_error(self):
        source = '''
            struct X {};
    
            struct Y : public std::shared_ptr<X> {};
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .bind<std::shared_ptr<X>, Y>();
            }
            '''
        expect_compile_error(
            'NonClassTypeError<std::shared_ptr<X>,X>',
            'A non-class type T was specified. Use C instead',
            COMMON_DEFINITIONS,
            source)

if __name__ == '__main__':
    absltest.main()
