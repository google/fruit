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
    using XAnnot = fruit::Annotated<Annotation1, X>;

    struct Annotation2 {};

    struct Annotation3 {};
    '''

def test_success_copyable_and_movable():
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
          fruit::Injector<X> injector(getComponent());
          injector.get<X*>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_success_movable_only():
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
          fruit::Injector<X> injector(getComponent());
          injector.get<X*>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_success_not_movable():
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
          fruit::Injector<X> injector(getComponent());
          injector.get<X*>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

# TODO: consider moving to test_normalized_component.py
@params(
    ('X', 'Y', 'Z'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, Y>', 'fruit::Annotated<Annotation3, Z>'),)
def test_autoinject_with_annotation_success(XAnnot, YAnnot, ZAnnot):
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

        fruit::Component<ZAnnot, YAnnot, XAnnot> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
          fruit::Injector<YAnnot> injector(normalizedComponent, getComponent());

          Assert(Y::num_objects_constructed == 0);
          injector.get<YAnnot>();
          Assert(Y::num_objects_constructed == 1);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_autoinject_annotation_in_signature_return_type():
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

def test_autoinject_wrong_class_in_typedef():
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

def test_error_abstract_class():
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
    expect_compile_error(
        'CannotConstructAbstractClassError<X>',
        'The specified class can.t be constructed because it.s an abstract class.',
        COMMON_DEFINITIONS,
        source)

def test_error_malformed_signature():
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
        'NotASignatureError<X\[\]>',
        'CandidateSignature was specified as parameter, but it.s not a signature. Signatures are of the form',
        COMMON_DEFINITIONS,
        source)

def test_error_malformed_signature_autoinject():
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
        'InjectTypedefNotASignatureError<X,X\[\]>',
        'C::Inject should be a typedef to a signature',
        COMMON_DEFINITIONS,
        source)

@params('char*', 'fruit::Annotated<Annotation1, char*>')
def test_error_does_not_exist(charPtrAnnot):
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
        'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
        'contains an Inject typedef but it.s not constructible with the specified types',
        COMMON_DEFINITIONS,
        source,
        locals())

@params('char*', 'fruit::Annotated<Annotation1, char*>')
def test_error_does_not_exist_autoinject(charPtrAnnot):
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
        'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
        'contains an Inject typedef but it.s not constructible with the specified types',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_error_abstract_class_autoinject():
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

if __name__ == '__main__':
    import nose2
    nose2.main()
