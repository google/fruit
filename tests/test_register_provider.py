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

    struct Annotation1 {};

    template <typename T>
    using WithNoAnnot = T;

    template <typename T>
    using WithAnnot1 = fruit::Annotated<Annotation1, T>;
    '''

@params(
    ('X', 'WithNoAnnot'),
    ('fruit::Annotated<Annotation1, X>', 'WithAnnot1'))
def test_success_returning_value(XAnnot, WithAnnot):
    source = '''
        struct X : public ConstructionTracker<X> {
          int value = 5;
        };

        fruit::Component<XAnnot> getComponent() {
          return fruit::createComponent()
            .registerProvider<XAnnot()>([](){return X();});
        }

        int main() {
          fruit::Injector<XAnnot> injector(getComponent());

          Assert((injector.get<WithAnnot<X                 >>(). value == 5));
          Assert((injector.get<WithAnnot<X*                >>()->value == 5));
          Assert((injector.get<WithAnnot<X&                >>(). value == 5));
          Assert((injector.get<WithAnnot<const X           >>(). value == 5));
          Assert((injector.get<WithAnnot<const X*          >>()->value == 5));
          Assert((injector.get<WithAnnot<const X&          >>(). value == 5));
          Assert((injector.get<WithAnnot<std::shared_ptr<X>>>()->value == 5));
    
          Assert(X::num_objects_constructed == 1);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'WithNoAnnot'),
    ('fruit::Annotated<Annotation1, X>', 'WithAnnot1'))
def test_success_returning_pointer(XAnnot, WithAnnot):
    source = '''
        struct X : public ConstructionTracker<X> {
          int value = 5;
        };

        fruit::Component<XAnnot> getComponent() {
          return fruit::createComponent()
            .registerProvider<WithAnnot<X*>()>([](){return new X();});
        }

        int main() {
          fruit::Injector<XAnnot> injector(getComponent());
          Assert((injector.get<WithAnnot<X                 >>(). value == 5));
          Assert((injector.get<WithAnnot<X*                >>()->value == 5));
          Assert((injector.get<WithAnnot<X&                >>(). value == 5));
          Assert((injector.get<WithAnnot<const X           >>(). value == 5));
          Assert((injector.get<WithAnnot<const X*          >>()->value == 5));
          Assert((injector.get<WithAnnot<const X&          >>(). value == 5));
          Assert((injector.get<WithAnnot<std::shared_ptr<X>>>()->value == 5));

          Assert(X::num_objects_constructed == 1);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_success_not_copyable_returning_value():
    source = '''
        struct X {
          X() = default;
          X(X&&) = default;
          X(const X&) = delete;
        };

        fruit::Component<X> getComponent() {
          return fruit::createComponent()
            .registerProvider([](){return X();});
        }

        int main() {
          fruit::Injector<X> injector(getComponent());
          injector.get<X*>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_success_not_copyable_returning_pointer():
    source = '''
        struct X {
          X() = default;
          X(X&&) = default;
          X(const X&) = delete;
        };

        fruit::Component<X> getComponent() {
          return fruit::createComponent()
            .registerProvider([](){return new X();});
        }

        int main() {
          fruit::Injector<X> injector(getComponent());
          injector.get<X*>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_success_not_movable_returning_pointer():
    source = '''
        struct X {
          X() = default;
          X(X&&) = delete;
          X(const X&) = delete;
        };

        fruit::Component<X> getComponent() {
          return fruit::createComponent()
            .registerProvider([](){return new X();});
        }

        int main() {
          fruit::Injector<X> injector(getComponent());
          injector.get<X*>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_not_function(XAnnot):
    source = '''
        struct X {
          X(int) {}
        };

        fruit::Component<XAnnot> getComponent() {
          int n = 3;
          return fruit::createComponent()
            .registerProvider<XAnnot()>([=]{return X(n);});
        }
        '''
    expect_compile_error(
        'FunctorUsedAsProviderError<.*>',
        'A stateful lambda or a non-lambda functor was used as provider',
        COMMON_DEFINITIONS,
        source,
        locals())

@params('int', 'fruit::Annotated<Annotation1, int>')
def test_error_malformed_signature(intAnnot):
    source = '''
        fruit::Component<intAnnot> getComponent() {
          return fruit::createComponent()
            .registerProvider<intAnnot>([](){return 42;});
        }
        '''
    expect_compile_error(
        'NotASignatureError<intAnnot>',
        'CandidateSignature was specified as parameter, but it.s not a signature. Signatures are of the form',
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'X*', '(struct )?X'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>', '(struct )?fruit::Annotated<(struct )?Annotation1, ?(struct )?X>'))
def test_error_returned_nullptr(XAnnot, XPtrAnnot, XAnnotRegex):
    source = '''
        struct X {};

        fruit::Component<XAnnot> getComponent() {
          return fruit::createComponent()
              .registerProvider<XPtrAnnot()>([](){return (X*)nullptr;});
        }

        int main() {
          fruit::Injector<XAnnot> injector(getComponent());
          injector.get<XAnnot>();
        }
        '''
    expect_runtime_error(
        'Fatal injection error: attempting to get an instance for the type XAnnotRegex but the provider returned nullptr',
        COMMON_DEFINITIONS,
        source,
        locals())

if __name__ == '__main__':
    import nose2
    nose2.main()
