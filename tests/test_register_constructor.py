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

    struct X;

    struct Annotation1 {};
    using XAnnot = fruit::Annotated<Annotation1, X>;

    struct Annotation2 {};

    struct Annotation3 {};
    
    template <typename T>
    using WithNoAnnotation = T;
    
    template <typename T>
    using WithAnnotation1 = fruit::Annotated<Annotation1, T>;
    '''

class TestRegisterConstructor(parameterized.TestCase):

    def test_register_constructor_success_copyable_and_movable(self):
        source = '''
            struct X {
              INJECT(X()) = default;
              X(X&&) = default;
              X(const X&) = default;
            };
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent();
            }
    
            int main() {
              fruit::Injector<X> injector(getComponent);
              injector.get<X*>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source)

    def test_register_constructor_success_movable_only(self):
        source = '''
            struct X {
              INJECT(X()) = default;
              X(X&&) = default;
              X(const X&) = delete;
            };
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent();
            }
    
            int main() {
              fruit::Injector<X> injector(getComponent);
              injector.get<X*>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source)

    def test_register_constructor_success_not_movable(self):
        source = '''
            struct X {
              INJECT(X()) = default;
              X(X&&) = delete;
              X(const X&) = delete;
            };
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent();
            }
    
            int main() {
              fruit::Injector<X> injector(getComponent);
              injector.get<X*>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source)

    # TODO: consider moving to test_normalized_component.py
    @parameterized.parameters([
        ('X', 'Y', 'Y', 'Z'),
        ('X', 'Y', 'const Y', 'Z'),
        ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, Y>', 'fruit::Annotated<Annotation2, const Y>', 'fruit::Annotated<Annotation3, Z>'),
    ])
    def test_autoinject_with_annotation_success(self, XAnnot, YAnnot, MaybeConstYAnnot, ZAnnot):
        source = '''
            struct X {
              using Inject = X();
            };
    
            struct Y : public ConstructionTracker<Y> {
              using Inject = Y();
            };
    
            struct Z {
              using Inject = Z();
            };
    
            fruit::Component<ZAnnot, MaybeConstYAnnot, XAnnot> getComponent() {
              return fruit::createComponent();
            }
            
            fruit::Component<> getEmptyComponent() {
              return fruit::createComponent();
            }
    
            int main() {
              fruit::NormalizedComponent<> normalizedComponent(getEmptyComponent);
              fruit::Injector<MaybeConstYAnnot> injector(normalizedComponent, getComponent);
    
              Assert(Y::num_objects_constructed == 0);
              injector.get<YAnnot>();
              Assert(Y::num_objects_constructed == 1);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_autoinject_annotation_in_signature_return_type(self):
        source = '''
            struct X {
              using Inject = XAnnot();
            };
    
            fruit::Component<XAnnot> getComponent() {
              return fruit::createComponent();
            }
            '''
        expect_compile_error(
            'InjectTypedefWithAnnotationError<X>',
            'C::Inject is a signature that returns an annotated type',
            COMMON_DEFINITIONS,
            source)

    def test_autoinject_wrong_class_in_typedef(self):
        source = '''
            struct X {
              using Inject = X();
            };
    
            struct Y : public X {
            };
    
            fruit::Component<Y> getComponent() {
              return fruit::createComponent();
            }
            '''
        expect_compile_error(
            'InjectTypedefForWrongClassError<Y,X>',
            'C::Inject is a signature, but does not return a C. Maybe the class C has no Inject typedef and',
            COMMON_DEFINITIONS,
            source)

    def test_register_constructor_error_abstract_class(self):
        source = '''
            struct X {
              X(int*) {}
    
              virtual void foo() = 0;
            };
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .registerConstructor<fruit::Annotated<Annotation1, X>(int*)>();
            }
            '''
        if re.search('GNU|MSVC', CXX_COMPILER_NAME) is not None:
            expect_generic_compile_error(
                'invalid abstract return type'
                '|.X.: cannot instantiate abstract class',
                COMMON_DEFINITIONS,
                source)
        else:
            expect_compile_error(
                'CannotConstructAbstractClassError<X>',
                'The specified class can.t be constructed because it.s an abstract class',
                COMMON_DEFINITIONS,
                source)

    def test_register_constructor_error_malformed_signature(self):
        source = '''
            struct X {
              X(int) {}
            };
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .registerConstructor<X[]>();
            }
            '''
        expect_compile_error(
            r'NotASignatureError<X\[\]>',
            r'CandidateSignature was specified as parameter, but it.s not a signature. Signatures are of the form',
            COMMON_DEFINITIONS,
            source)

    def test_register_constructor_error_malformed_signature_autoinject(self):
        source = '''
            struct X {
              using Inject = X[];
              X(int) {}
            };
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent();
            }
            '''
        expect_compile_error(
            r'InjectTypedefNotASignatureError<X,X\[\]>',
            r'C::Inject should be a typedef to a signature',
            COMMON_DEFINITIONS,
            source)

    @parameterized.parameters([
        'char*',
        'fruit::Annotated<Annotation1, char*>',
    ])
    def test_register_constructor_does_not_exist_error(self, charPtrAnnot):
        source = '''
            struct X {
              X(int*) {}
            };
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .registerConstructor<X(charPtrAnnot)>();
            }
            '''
        expect_compile_error(
            r'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
            r'contains an Inject typedef but it.s not constructible with the specified types',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        'char*',
        'fruit::Annotated<Annotation1, char*>',
    ])
    def test_autoinject_constructor_does_not_exist_error(self, charPtrAnnot):
        source = '''
            struct X {
              using Inject = X(charPtrAnnot);
              X(int*) {}
            };
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent();
            }
            '''
        expect_compile_error(
            r'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
            r'contains an Inject typedef but it.s not constructible with the specified types',
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_autoinject_abstract_class_error(self):
        source = '''
            struct X {
              using Inject = fruit::Annotated<Annotation1, X>();
    
              virtual void scale() = 0;
              // Note: here we "forgot" to implement scale() (on purpose, for this test) so X is an abstract class.
            };
    
            fruit::Component<fruit::Annotated<Annotation1, X>> getComponent() {
              return fruit::createComponent();
            }
            '''
        expect_compile_error(
            'CannotConstructAbstractClassError<X>',
            'The specified class can.t be constructed because it.s an abstract class.',
            COMMON_DEFINITIONS,
            source)

    @multiple_parameters([
        'WithNoAnnotation',
        'WithAnnotation1',
    ], [
        'Y',
        'const Y',
        'Y*',
        'const Y*',
        'Y&',
        'const Y&',
        'std::shared_ptr<Y>',
        'fruit::Provider<Y>',
        'fruit::Provider<const Y>',
    ])
    def test_register_constructor_with_param_success(self, WithAnnotation, YVariant):
        source = '''
            struct Y {};
            struct X {
              X(YVariant) {
              }
            };
            
            fruit::Component<WithAnnotation<Y>> getYComponent() {
              return fruit::createComponent()
                .registerConstructor<WithAnnotation<Y>()>();
            }
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .install(getYComponent)
                .registerConstructor<X(WithAnnotation<YVariant>)>();
            }
    
            int main() {
              fruit::Injector<X> injector(getComponent);
              injector.get<X>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @multiple_parameters([
        'WithNoAnnotation',
        'WithAnnotation1',
    ], [
        'Y',
        'const Y',
        'const Y*',
        'const Y&',
        'fruit::Provider<const Y>',
    ])
    def test_register_constructor_with_param_const_binding_success(self, WithAnnotation, YVariant):
        source = '''
            struct Y {};
            struct X {
              X(YVariant) {
              }
            };
            
            const Y y{};
            
            fruit::Component<WithAnnotation<const Y>> getYComponent() {
              return fruit::createComponent()
                .bindInstance<WithAnnotation<Y>, Y>(y);
            }
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .install(getYComponent)
                .registerConstructor<X(WithAnnotation<YVariant>)>();
            }
    
            int main() {
              fruit::Injector<X> injector(getComponent);
              injector.get<X>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @multiple_parameters([
        ('WithNoAnnotation', 'Y'),
        ('WithAnnotation1', 'fruit::Annotated<Annotation1,Y>'),
    ], [
        'Y*',
        'Y&',
        'std::shared_ptr<Y>',
        'fruit::Provider<Y>',
    ])
    def test_register_constructor_with_param_error_nonconst_param_required(self, WithAnnotation, YAnnotRegex, YVariant):
        source = '''
            struct Y {};
            struct X {
              X(YVariant);
            };
            
            fruit::Component<WithAnnotation<const Y>> getYComponent();
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .install(getYComponent)
                .registerConstructor<X(WithAnnotation<YVariant>)>();
            }
            '''
        expect_compile_error(
            'NonConstBindingRequiredButConstBindingProvidedError<YAnnotRegex>',
            'The type T was provided as constant, however one of the constructors/providers/factories in this component',
            COMMON_DEFINITIONS,
            source,
            locals())

    @multiple_parameters([
        ('WithNoAnnotation', 'Y'),
        ('WithAnnotation1', 'fruit::Annotated<Annotation1, Y>'),
    ], [
        'Y*',
        'Y&',
        'std::shared_ptr<Y>',
        'fruit::Provider<Y>',
    ])
    def test_register_constructor_with_param_error_nonconst_param_required_install_after(self, WithAnnotation, YAnnotRegex, YVariant):
        source = '''
            struct Y {};
            struct X {
              X(YVariant);
            };
            
            fruit::Component<WithAnnotation<const Y>> getYComponent();
    
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .registerConstructor<X(WithAnnotation<YVariant>)>()
                .install(getYComponent);
            }
            '''
        expect_compile_error(
            'NonConstBindingRequiredButConstBindingProvidedError<YAnnotRegex>',
            'The type T was provided as constant, however one of the constructors/providers/factories in this component',
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_register_constructor_requiring_nonconst_then_requiring_const_ok(self):
        source = '''
            struct X {};
            
            struct Y {
              Y(X&) {}
            };
            
            struct Z {
              Z(const X&) {}
            };
    
            fruit::Component<Y, Z> getRootComponent() {
              return fruit::createComponent()
                .registerConstructor<Y(X&)>()
                .registerConstructor<Z(const X&)>()
                .registerConstructor<X()>();
            }
            
            int main() {
              fruit::Injector<Y, Z> injector(getRootComponent);
              injector.get<Y>();
              injector.get<Z>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_register_constructor_requiring_nonconst_then_requiring_const_declaring_const_requirement_error(self):
        source = '''
            struct X {};
            
            struct Y {
              Y(X&) {}
            };
            
            struct Z {
              Z(const X&) {}
            };
    
            fruit::Component<fruit::Required<const X>, Y, Z> getRootComponent() {
              return fruit::createComponent()
                .registerConstructor<Y(X&)>()
                .registerConstructor<Z(const X&)>();
            }
            '''
        expect_compile_error(
            'ConstBindingDeclaredAsRequiredButNonConstBindingRequiredError<X>',
            'The type T was declared as a const Required type in the returned Component, however',
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_register_constructor_requiring_const_then_requiring_nonconst_ok(self):
        source = '''
            struct X {};
            
            struct Y {
              Y(const X&) {}
            };
            
            struct Z {
              Z(X&) {}
            };
    
            fruit::Component<Y, Z> getRootComponent() {
              return fruit::createComponent()
                .registerConstructor<Y(const X&)>()
                .registerConstructor<Z(X&)>()
                .registerConstructor<X()>();
            }
            
            int main() {
              fruit::Injector<Y, Z> injector(getRootComponent);
              injector.get<Y>();
              injector.get<Z>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_register_constructor_requiring_const_then_requiring_nonconst_declaring_const_requirement_error(self):
        source = '''
            struct X {};
            
            struct Y {
              Y(const X&) {}
            };
            
            struct Z {
              Z(X&) {}
            };
    
            fruit::Component<fruit::Required<const X>, Y, Z> getRootComponent() {
              return fruit::createComponent()
                .registerConstructor<Y(const X&)>()
                .registerConstructor<Z(X&)>();
            }
            '''
        expect_compile_error(
            'ConstBindingDeclaredAsRequiredButNonConstBindingRequiredError<X>',
            'The type T was declared as a const Required type in the returned Component, however',
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('Y**', r'Y\*\*'),
        ('std::shared_ptr<Y>*', r'std::shared_ptr<Y>\*'),
        ('std::nullptr_t', r'(std::)?nullptr(_t)?'),
        ('Y*&', r'Y\*&'),
        ('Y(*)()', r'Y(\((__cdecl)?\*\))?\((void)?\)'),
        ('fruit::Annotated<Annotation1, Y**>', r'Y\*\*'),
    ])
    def test_register_constructor_with_param_error_type_not_injectable(self, YVariant, YVariantRegex):
        source = '''
            struct Y {};
            struct X {
              X(YVariant);
            };
            
            fruit::Component<> getComponent() {
              return fruit::createComponent()
                .registerConstructor<X(YVariant)>();
            }
            '''
        expect_compile_error(
            'NonInjectableTypeError<YVariantRegex>',
            'The type T is not injectable.',
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
