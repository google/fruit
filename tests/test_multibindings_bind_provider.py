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
          fruit::Injector<> injector(getComponent());

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
@pytest.mark.parametrize('YAnnot,YVariant,YVariantAnnot', [
    ('Y', 'Y', 'Y'),
    ('Y', 'const Y', 'const Y'),
    ('Y', 'Y*', 'Y*'),
    ('Y', 'const Y*', 'const Y*'),
    ('Y', 'Y&', 'Y&'),
    ('Y', 'const Y&', 'const Y&'),
    ('Y', 'std::shared_ptr<Y>', 'std::shared_ptr<Y>'),
    ('Y', 'fruit::Provider<Y>', 'fruit::Provider<Y>'),
    ('fruit::Annotated<Annotation1, Y>', 'Y', 'fruit::Annotated<Annotation1, Y>'),
    ('fruit::Annotated<Annotation1, Y>', 'const Y', 'fruit::Annotated<Annotation1, const Y>'),
    ('fruit::Annotated<Annotation1, Y>', 'Y*', 'fruit::Annotated<Annotation1, Y*>'),
    ('fruit::Annotated<Annotation1, Y>', 'const Y*', 'fruit::Annotated<Annotation1, const Y*>'),
    ('fruit::Annotated<Annotation1, Y>', 'Y&', 'fruit::Annotated<Annotation1, Y&>'),
    ('fruit::Annotated<Annotation1, Y>', 'const Y&', 'fruit::Annotated<Annotation1, const Y&>'),
    ('fruit::Annotated<Annotation1, Y>', 'std::shared_ptr<Y>', 'fruit::Annotated<Annotation1, std::shared_ptr<Y>>'),
    ('fruit::Annotated<Annotation1, Y>', 'fruit::Provider<Y>', 'fruit::Annotated<Annotation1, fruit::Provider<Y>>'),
])
def test_bind_multibinding_provider_with_param_success(ConstructX, XPtr, YAnnot, YVariant, YVariantAnnot):
    source = '''
        struct Y {};
    
        struct X : public ConstructionTracker<X> {};
        
        fruit::Component<YAnnot> getYComponent() {
          return fruit::createComponent()
            .registerConstructor<YAnnot()>();
        }

        fruit::Component<> getComponent() {
          return fruit::createComponent()
            .install(getYComponent)
            .addMultibindingProvider<XPtr(YVariantAnnot)>([](YVariant){ return ConstructX; });
        }

        int main() {
          fruit::Injector<> injector(getComponent());

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
@pytest.mark.parametrize('YVariant,YVariantRegex', [
    ('Y**', r'Y\*\*'),
    ('std::shared_ptr<Y>*', r'std::shared_ptr<Y>\*'),
    ('std::nullptr_t', r'(std::)?nullptr_t'),
    ('Y*&', r'Y\*&'),
    ('Y(*)()', r'Y(\(\*\))?\(\)'),
    ('fruit::Annotated<Annotation1, Y**>', r'Y\*\*'),
])
def test_bind_multibinding_provider_with_param_error_type_not_injectable(ConstructX, XPtr, YVariant, YVariantRegex):
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
          fruit::Injector<> injector(getComponent());

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

        int main() {
          fruit::NormalizedComponent<> normalizedComponent(getComponent());
          fruit::Injector<> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));

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
          fruit::Injector<> injector(getComponent());

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
          fruit::Injector<> injector(getComponent());
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
