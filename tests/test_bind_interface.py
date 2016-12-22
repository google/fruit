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
    struct Annotation2 {};
    '''

@params(
    ('X', 'int'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation2, int>'))
def test_error_not_base(XAnnot, intAnnot):
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
@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_bound_to_itself(XAnnot):
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

def test_bound_to_itself_with_annotation_error():
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

def test_bound_chain_ok():
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
          fruit::Injector<X> injector(getComponent());
          X& x = injector.get<X&>();
          x.f();
        }
    '''
    expect_success(COMMON_DEFINITIONS, source)

def test_bind_non_normalized_types_error():
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

def test_bind_factory_no_args():
    source = '''
        struct X {
          virtual void foo() = 0;
          ~X() {}
        };

        struct Y : public X {
          void foo() override {
          }
        };

        template <typename T>
        using Factory = std::function<std::unique_ptr<T>()>;

        fruit::Component<Factory<Y>> getYComponent() {
          return fruit::createComponent()
            .registerFactory<std::unique_ptr<Y>()>([]() { return std::unique_ptr<Y>(new Y()); });
        }

        fruit::Component<Factory<X>> getComponent() {
          return fruit::createComponent()
            .install(getYComponent())
            .bind<X, Y>();
        }

        int main() {
          fruit::Injector<Factory<X>> injector(getComponent());
          Factory<X> xFactory = injector.get<Factory<X>>();
          std::unique_ptr<X> xPtr = xFactory();
          xPtr->foo();
        }
    '''
    expect_success(COMMON_DEFINITIONS, source)

def test_bind_factory_no_args_invalid_unique_ptr():
    source = '''
        struct X {
          virtual void foo() = 0;
          ~X() {}
        };

        struct Y : public X {
          void foo() override {
          }
        };

        template <typename T>
        using Factory = std::function<std::unique_ptr<T>()>;

        fruit::Component<Factory<Y>> getYComponent() {
          return fruit::createComponent()
            .registerFactory<std::unique_ptr<Y>()>([]() { return std::unique_ptr<Y>(); });
        }

        fruit::Component<Factory<X>> getComponent() {
          return fruit::createComponent()
            .install(getYComponent())
            .bind<X, Y>();
        }

        int main() {
          fruit::Injector<Factory<X>> injector(getComponent());
          Factory<X> xFactory = injector.get<Factory<X>>();
          std::unique_ptr<X> xPtr = xFactory();
          Assert(xPtr.get() == nullptr);
        }
    '''
    expect_success(COMMON_DEFINITIONS, source)

def test_bind_factory_1_arg():
    source = '''
        struct X {
          virtual void foo() = 0;
          ~X() {}
        };

        struct Y : public X {
          void foo() override {
          }
        };

        template <typename T>
        using Factory = std::function<std::unique_ptr<T>(char)>;

        fruit::Component<Factory<Y>> getYComponent() {
          return fruit::createComponent()
            .registerFactory<std::unique_ptr<Y>(fruit::Assisted<char>)>(
                [](char) { return std::unique_ptr<Y>(new Y()); });
        }

        fruit::Component<Factory<X>> getComponent() {
          return fruit::createComponent()
            .install(getYComponent())
            .bind<X, Y>();
        }

        int main() {
          fruit::Injector<Factory<X>> injector(getComponent());
          Factory<X> xFactory = injector.get<Factory<X>>();
          std::unique_ptr<X> xPtr = xFactory('w');
          xPtr->foo();
        }
    '''
    expect_success(COMMON_DEFINITIONS, source)

def test_bind_factory_2_arg():
    source = '''
        struct X {
          virtual void foo() = 0;
          ~X() {}
        };

        struct Y : public X {
          void foo() override {
          }
        };

        template <typename T>
        using Factory = std::function<std::unique_ptr<T>(char, double)>;

        fruit::Component<Factory<Y>> getYComponent() {
          return fruit::createComponent()
            .registerFactory<std::unique_ptr<Y>(fruit::Assisted<char>, fruit::Assisted<double>)>(
                [](char, double) { return std::unique_ptr<Y>(new Y()); });
        }

        fruit::Component<Factory<X>> getComponent() {
          return fruit::createComponent()
            .install(getYComponent())
            .bind<X, Y>();
        }

        int main() {
          fruit::Injector<Factory<X>> injector(getComponent());
          Factory<X> xFactory = injector.get<Factory<X>>();
          std::unique_ptr<X> xPtr = xFactory('w', 3.2);
          xPtr->foo();
        }
    '''
    expect_success(COMMON_DEFINITIONS, source)

if __name__ == '__main__':
    import nose2
    nose2.main()
