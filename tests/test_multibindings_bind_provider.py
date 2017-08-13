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

    struct Annotation1 {};
    struct Annotation2 {};
    
    template <typename T>
    using WithNoAnnot = T;
    
    template <typename T>
    using WithAnnot1 = fruit::Annotated<Annotation1, T>;
    '''

@pytest.mark.parametrize('ConstructX', [
    'X()',
    'new X()',
])
def test_bind_multibinding_provider_success(ConstructX):
    source = '''
        struct X : public ConstructionTracker<X> {
          INJECT(X()) = default;
        };

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .addMultibindingProvider([](){return ConstructX;});
        }

        int main() {
          fruit::Injector<> injector(getComponent);

          Assert(X::num_objects_constructed == 0);
          Assert(injector.getMultibindings<X>().size() == 1);
          Assert(X::num_objects_constructed == 1);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('WithAnnot', [
    'WithNoAnnot',
    'WithAnnot1',
])
def test_bind_multibinding_provider_abstract_class_success(WithAnnot):
    source = '''
        struct I {
          virtual int foo() = 0;
          virtual ~I() = default;
        };
        
        struct X : public I {
          int foo() override {
            return 5;
          }
        };

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .addMultibindingProvider<WithAnnot<I*>()>([](){return static_cast<I*>(new X());});
        }

        int main() {
          fruit::Injector<> injector(getComponent);

          Assert(injector.getMultibindings<WithAnnot<I>>().size() == 1);
          Assert(injector.getMultibindings<WithAnnot<I>>()[0]->foo() == 5);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('WithAnnot', [
    'WithNoAnnot',
    'WithAnnot1',
])
def test_bind_multibinding_provider_abstract_class_with_no_virtual_destructor_error(WithAnnot):
    source = '''
        struct I {
          virtual int foo() = 0;
        };
        
        struct X : public I {
          int foo() override {
            return 5;
          }
        };

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .addMultibindingProvider<WithAnnot<I*>()>([](){return static_cast<I*>(new X());});
        }
        '''
    expect_compile_error(
        'MultibindingProviderReturningPointerToAbstractClassWithNoVirtualDestructorError<I>',
        'registerMultibindingProvider\(\) was called with a lambda that returns a pointer to T, but T is an abstract class with no virtual destructor',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstructX,XPtr', [
    ('X()', 'X'),
    ('new X()', 'X*'),
])
@pytest.mark.parametrize('WithAnnot', [
    'WithNoAnnot',
    'WithAnnot1',
])
@pytest.mark.parametrize('YVariant', [
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
def test_bind_multibinding_provider_with_param_success(ConstructX, XPtr, WithAnnot, YVariant):
    source = '''
        struct Y {};
    
        struct X : public ConstructionTracker<X> {};
        
        fruit::Component<WithAnnot<Y>> getYComponent() {
          return fruit::createComponent()
            .registerConstructor<WithAnnot<Y>()>();
        }

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .install(getYComponent)
            .addMultibindingProvider<XPtr(WithAnnot<YVariant>)>([](YVariant){ return ConstructX; });
        }

        int main() {
          fruit::Injector<> injector(getComponent);

          Assert(X::num_objects_constructed == 0);
          Assert(injector.getMultibindings<X>().size() == 1);
          Assert(X::num_objects_constructed == 1);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstructX,XPtr', [
    ('X()', 'X'),
    ('new X()', 'X*'),
])
@pytest.mark.parametrize('WithAnnot', [
    'WithNoAnnot',
    'WithAnnot1',
])
@pytest.mark.parametrize('YVariant', [
    'Y',
    'const Y',
    'const Y*',
    'const Y&',
    'fruit::Provider<const Y>',
])
def test_bind_multibinding_provider_with_param_const_binding_success(ConstructX, XPtr, WithAnnot, YVariant):
    source = '''
        struct Y {};
    
        struct X : public ConstructionTracker<X> {};
        
        const Y y{};
        
        fruit::Component<WithAnnot<const Y>> getYComponent() {
          return fruit::createComponent()
            .bindInstance<WithAnnot<Y>, Y>(y);
        }

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .install(getYComponent)
            .addMultibindingProvider<XPtr(WithAnnot<YVariant>)>([](YVariant){ return ConstructX; });
        }

        int main() {
          fruit::Injector<> injector(getComponent);

          Assert(X::num_objects_constructed == 0);
          Assert(injector.getMultibindings<X>().size() == 1);
          Assert(X::num_objects_constructed == 1);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstructX,XPtr', [
    ('X()', 'X'),
    ('new X()', 'X*'),
])
@pytest.mark.parametrize('WithAnnot,YAnnotRegex', [
    ('WithNoAnnot', 'Y'),
    ('WithAnnot1', 'fruit::Annotated<Annotation1, Y>'),
])
@pytest.mark.parametrize('YVariant', [
    'Y*',
    'Y&',
    'std::shared_ptr<Y>',
    'fruit::Provider<Y>',
])
def test_bind_multibinding_provider_with_param_error_nonconst_param_required(ConstructX, XPtr, WithAnnot, YAnnotRegex, YVariant):
    source = '''
        struct Y {};
        struct X {};
        
        fruit::Component<WithAnnot<const Y>> getYComponent();

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .install(getYComponent)
            .addMultibindingProvider<XPtr(WithAnnot<YVariant>)>([](YVariant){ return ConstructX; });
        }
        '''
    expect_compile_error(
        'NonConstBindingRequiredButConstBindingProvidedError<YAnnotRegex>',
        'The type T was provided as constant, however one of the constructors/providers/factories in this component',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstructX,XPtr', [
    ('X()', 'X'),
    ('new X()', 'X*'),
])
@pytest.mark.parametrize('WithAnnot,YAnnotRegex', [
    ('WithNoAnnot', 'Y'),
    ('WithAnnot1', 'fruit::Annotated<Annotation1, Y>'),
])
@pytest.mark.parametrize('YVariant', [
    'Y*',
    'Y&',
    'std::shared_ptr<Y>',
    'fruit::Provider<Y>',
])
def test_bind_multibinding_provider_with_param_error_nonconst_param_required_install_after(ConstructX, XPtr, WithAnnot, YAnnotRegex, YVariant):
    source = '''
        struct Y {};
        struct X {};
        
        fruit::Component<WithAnnot<const Y>> getYComponent();

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .addMultibindingProvider<XPtr(WithAnnot<YVariant>)>([](YVariant){ return ConstructX; })
            .install(getYComponent);
        }
        '''
    expect_compile_error(
        'NonConstBindingRequiredButConstBindingProvidedError<YAnnotRegex>',
        'The type T was provided as constant, however one of the constructors/providers/factories in this component',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_bind_multibinding_provider_requiring_nonconst_then_requiring_const_ok():
    source = '''
        struct X {};
        struct Y {};

        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
            .addMultibindingProvider([](X&) { return Y(); })
            .addMultibindingProvider([](const X&) { return Y(); })
            .registerConstructor<X()>();
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent);
          injector.getMultibindings<Y>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_bind_multibinding_provider_requiring_nonconst_then_requiring_const_declaring_const_requirement_error():
    source = '''
        struct X {};
        struct Y {};

        fruit::Component<fruit::Required<const X>> getRootComponent() {
          return fruit::createComponent()
            .addMultibindingProvider([](X&) { return Y(); })
            .addMultibindingProvider([](const X&) { return Y(); });
        }
        '''
    expect_compile_error(
        'ConstBindingDeclaredAsRequiredButNonConstBindingRequiredError<X>',
        'The type T was declared as a const Required type in the returned Component, however',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_bind_multibinding_provider_requiring_const_then_requiring_nonconst_ok():
    source = '''
        struct X {};
        struct Y {};

        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
            .addMultibindingProvider([](const X&) { return Y(); })
            .addMultibindingProvider([](X&) { return Y(); })
            .registerConstructor<X()>();
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent);
          injector.getMultibindings<Y>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_bind_multibinding_provider_requiring_const_then_requiring_nonconst_declaring_const_requirement_error():
    source = '''
        struct X {};
        struct Y {};

        fruit::Component<fruit::Required<const X>> getRootComponent() {
          return fruit::createComponent()
            .addMultibindingProvider([](const X&) { return Y(); })
            .addMultibindingProvider([](X&) { return Y(); });
        }
        '''
    expect_compile_error(
        'ConstBindingDeclaredAsRequiredButNonConstBindingRequiredError<X>',
        'The type T was declared as a const Required type in the returned Component, however',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstructX,XPtr', [
    ('X()', 'X'),
    ('new X()', 'X*'),
])
@pytest.mark.parametrize('YAnnot,ConstYAnnot,YVariant,YVariantRegex', [
    ('Y', 'Y', 'Y**', 'Y\*\*'),
    ('Y', 'Y', 'std::shared_ptr<Y>*', 'std::shared_ptr<Y>\*'),
    ('Y', 'const Y', 'Y**', 'Y\*\*'),
    ('Y', 'const Y', 'std::shared_ptr<Y>*', 'std::shared_ptr<Y>\*'),
    ('fruit::Annotated<Annotation1, Y>', 'fruit::Annotated<Annotation1, Y>', 'Y**', 'Y\*\*'),
    ('fruit::Annotated<Annotation1, Y>', 'fruit::Annotated<Annotation1, const Y>', 'Y**', 'Y\*\*'),
])
def test_bind_multibinding_provider_with_param_error_type_not_injectable(ConstructX, XPtr, YAnnot, ConstYAnnot, YVariant, YVariantRegex):
    source = '''
        struct Y {};
        struct X {};
        
        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .addMultibindingProvider<XPtr(YVariant)>([](YVariant){ return ConstructX; });
        }
        '''
    expect_compile_error(
        'NonInjectableTypeError<YVariantRegex>',
        'The type T is not injectable.',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstructX,XAnnot,XPtrAnnot', [
    ('X()', 'X', 'X'),
    ('X()', 'fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>'),
    ('new X()', 'X', 'X*'),
    ('new X()', 'fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>'),
])
def test_bind_multibinding_provider_explicit_signature_success(ConstructX, XAnnot, XPtrAnnot):
    source = '''
        struct X : public ConstructionTracker<X> {
          INJECT(X()) = default;

          static bool constructed;
        };

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .addMultibindingProvider<XPtrAnnot()>([](){return ConstructX;});
        }

        int main() {
          fruit::Injector<> injector(getComponent);

          Assert(X::num_objects_constructed == 0);
          Assert(injector.getMultibindings<XAnnot>().size() == 1);
          Assert(X::num_objects_constructed == 1);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstructX,XAnnot,XPtrAnnot', [
    ('X()', 'X', 'X'),
    ('X()', 'fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X>'),
    ('new X()', 'X', 'X*'),
    ('new X()', 'fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>'),
])
def test_bind_multibinding_provider_explicit_signature_with_normalized_component_success(ConstructX, XAnnot, XPtrAnnot):
    source = '''
        struct X : public ConstructionTracker<X> {
          INJECT(X()) = default;

          static bool constructed;
        };

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .addMultibindingProvider<XPtrAnnot()>([](){return ConstructX;});
        }
        
        fruit::Component<> getEmptyComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::NormalizedComponent<> normalizedComponent(getComponent);
          fruit::Injector<> injector(normalizedComponent, getEmptyComponent);

          Assert(X::num_objects_constructed == 0);
          Assert(injector.getMultibindings<XAnnot>().size() == 1);
          Assert(X::num_objects_constructed == 1);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('XAnnot,XPtrAnnot,intAnnot', [
    ('X', 'X*', 'int'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>', 'fruit::Annotated<Annotation2, int>'),
])
def test_multiple_providers(XAnnot, XPtrAnnot, intAnnot):
    source = '''
        struct X {};

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .registerProvider<intAnnot()>([](){return 42;})
            .addMultibindingProvider<XAnnot(intAnnot)>([](int){return X();})
            .addMultibindingProvider<XPtrAnnot(intAnnot)>([](int){return new X();});
        }

        int main() {
          fruit::Injector<> injector(getComponent);

          std::vector<X*> multibindings = injector.getMultibindings<XAnnot>();
          Assert(multibindings.size() == 2);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstructX', [
    'X()',
    'new X()',
])
@pytest.mark.parametrize('XAnnot', [
    'X',
    'fruit::Annotated<Annotation1, X>',
])
def test_bind_multibinding_provider_malformed_signature(ConstructX, XAnnot):
    source = '''
        struct X {};

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .addMultibindingProvider<XAnnot>([](){return ConstructX;});
        }
        '''
    expect_compile_error(
        'NotASignatureError<XAnnot>',
        'CandidateSignature was specified as parameter, but it.s not a signature.',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ConstructX', [
    'X(n)',
    'new X(n)',
])
@pytest.mark.parametrize('XAnnot', [
    'X',
    'fruit::Annotated<Annotation1, X>',
])
def test_bind_multibinding_provider_lambda_with_captures_error(ConstructX, XAnnot):
    source = '''
        struct X {
          X(int) {}
        };

        fruit::Component<> getComponent() {
          int n = 3;
          return fruit::createComponent()
            .addMultibindingProvider<XAnnot()>([=]{return ConstructX;});
        }
        '''
    expect_compile_error(
        'FunctorUsedAsProviderError<.*>',
        'A stateful lambda or a non-lambda functor was used as provider',
        COMMON_DEFINITIONS,
        source,
        locals())

# TODO: should XPtrAnnot be just XAnnot in the signature?
# Make sure the behavior here is consistent with registerProvider() and registerFactory().
@pytest.mark.parametrize('XAnnot,XPtrAnnot,XAnnotRegex', [
    ('X', 'X*', '(struct )?X'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>', '(struct )?fruit::Annotated<(struct )?Annotation1, ?(struct )?X>'),
])
def test_provider_returns_nullptr_error(XAnnot, XPtrAnnot, XAnnotRegex):
    source = '''
        struct X {};

        fruit::Component<> getComponent() {
          return fruit::createComponent()
              .addMultibindingProvider<XPtrAnnot()>([](){return (X*)nullptr;});
        }

        int main() {
          fruit::Injector<> injector(getComponent);
          injector.getMultibindings<XAnnot>();
        }
        '''
    expect_runtime_error(
        'Fatal injection error: attempting to get an instance for the type XAnnotRegex but the provider returned nullptr',
        COMMON_DEFINITIONS,
        source,
        locals())

if __name__== '__main__':
    main(__file__)
