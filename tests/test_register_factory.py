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

    struct Scaler;
    struct ScalerImpl;

    struct Annotation1 {};
    using ScalerAnnot1 = fruit::Annotated<Annotation1, Scaler>;
    using ScalerImplAnnot1 = fruit::Annotated<Annotation1, ScalerImpl>;

    struct Annotation2 {};
    using ScalerAnnot2 = fruit::Annotated<Annotation2, Scaler>;
    using ScalerImplAnnot2 = fruit::Annotated<Annotation2, ScalerImpl>;
    '''

@params('std::function<X()>', 'fruit::Annotated<Annotation1, std::function<X()>>')
def test_success_no_params_autoinject(XFactoryAnnot):
    source = '''
        struct X {
          INJECT(X()) = default;
        };

        fruit::Component<XFactoryAnnot> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<XFactoryAnnot> injector(getComponent());
          injector.get<XFactoryAnnot>()();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'std::function<X()>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, std::function<X()>>'))
def test_success_no_params_returning_value(XAnnot, XFactoryAnnot):
    source = '''
        struct X {};

        fruit::Component<XFactoryAnnot> getComponent() {
          return fruit::createComponent()
            .registerFactory<XAnnot()>([](){return X();});
        }

        int main() {
          fruit::Injector<XFactoryAnnot> injector(getComponent());
          injector.get<XFactoryAnnot>()();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'std::unique_ptr<X>', 'std::function<std::unique_ptr<X>()>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, std::unique_ptr<X>>', 'fruit::Annotated<Annotation1, std::function<std::unique_ptr<X>()>>'))
def test_success_no_params_returning_pointer(XAnnot, XPtrAnnot, XPtrFactoryAnnot):
    source = '''
        struct X {};

        fruit::Component<XPtrFactoryAnnot> getComponent() {
          return fruit::createComponent()
            .registerFactory<XPtrAnnot()>([](){return std::unique_ptr<X>();});
        }

        int main() {
          fruit::Injector<XPtrFactoryAnnot> injector(getComponent());
          injector.get<XPtrFactoryAnnot>()();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_autoinject_success():
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          INJECT(ScalerImpl(ASSISTED(double) factor))
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent()
            .bind<Scaler, ScalerImpl>();
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params(
    ('Scaler',
     'std::function<std::unique_ptr<Scaler>(double)>'),
    ('fruit::Annotated<Annotation1, Scaler>',
     'fruit::Annotated<Annotation1, std::function<std::unique_ptr<Scaler>(double)>>'))
def test_autoinject(ScalerAnnot, ScalerFactoryAnnot):
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          INJECT(ScalerImpl(ASSISTED(double) factor))
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactoryAnnot> getScalerComponent() {
          return fruit::createComponent()
            .bind<ScalerAnnot, ScalerImpl>();
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_autoinject_returning_value():
    source = '''
        struct X {
          INJECT(X()) = default;
        };

        struct Scaler {
        private:
          double factor;

        public:
          INJECT(Scaler(ASSISTED(double) factor, X))
            : factor(factor) {
          }

          double scale(double x) {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<Scaler(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          Scaler scaler = scalerFactory(12.1);
          std::cout << scaler.scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params(
    ('Scaler',
     'ScalerImpl',
     'std::function<std::unique_ptr<Scaler>(double)>',
     'std::function<std::unique_ptr<ScalerImpl(,std::default_delete<ScalerImpl>)?>\(double\)>',
    ),
    ('fruit::Annotated<Annotation1, Scaler>',
     'fruit::Annotated<Annotation2, ScalerImpl>',
     'fruit::Annotated<Annotation1, std::function<std::unique_ptr<Scaler>(double)>>',
     'fruit::Annotated<Annotation2,std::function<std::unique_ptr<ScalerImpl(,std::default_delete<ScalerImpl>)?>\(double\)>>',
    )
)
def test_autoinject_error_abstract_class(ScalerAnnot, ScalerImplAnnot, ScalerFactoryAnnot, ScalerImplFactoryAnnotRegex):
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor)
            : factor(factor) {
          }

          // Note: here we "forgot" to implement scale() (on purpose, for this test) so ScalerImpl is an abstract class.
        };

        fruit::Component<ScalerFactoryAnnot> getScalerComponent() {
          return fruit::createComponent()
            .bind<ScalerAnnot, ScalerImplAnnot>();
        }
        '''
    expect_compile_error(
        'NoBindingFoundForAbstractClassError<ScalerImplFactoryAnnotRegex,ScalerImpl>',
        'No explicit binding was found for T, and note that C is an abstract class',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_autoinject_nonmovable_ok():
    source = '''
        struct I {};

        struct C : public I {
          INJECT(C()) = default;

          C(const C&) = delete;
          C(C&&) = delete;
          C& operator=(const C&) = delete;
          C& operator=(C&&) = delete;
        };

        using IFactory = std::function<std::unique_ptr<I>()>;

        fruit::Component<IFactory> getIFactory() {
          return fruit::createComponent()
              .bind<I, C>();
        }

        int main() {
          fruit::Injector<IFactory> injector(getIFactory());
          IFactory iFactory(injector);
          std::unique_ptr<I> i = iFactory();
          (void)i;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_autoinject_2_assisted_params():
    source = '''
        struct Foo {
          Foo(int x, float y) {
            (void)x;
            (void)y;
          }
        };

        using FooFactory = std::function<Foo(int, float)>;

        fruit::Component<FooFactory> getComponent() {
          return fruit::createComponent()
              .registerFactory<Foo(fruit::Assisted<int>, fruit::Assisted<float>)>(
                  [](int x, float y) {
                    return Foo(x, y);
                  });
        }

        int main() {
          fruit::Injector<FooFactory> injector(getComponent());
          FooFactory fooFactory(injector);
          Foo foo = fooFactory(1, 2.3f);
          (void)foo;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_autoinject_2_assisted_params_returning_value():
    source = '''
        struct Foo {
          Foo(int x, float y) {
            (void)x;
            (void)y;
          }
        };

        using FooFactory = std::function<Foo(int, float)>;

        fruit::Component<FooFactory> getComponent() {
          return fruit::createComponent()
              .registerFactory<Foo(fruit::Assisted<int>, fruit::Assisted<float>)>(
                  [](int x, float y) {
                    return Foo(x, y);
                  });
        }

        int main() {
          fruit::Injector<FooFactory> injector(getComponent());
          FooFactory fooFactory(injector);
          Foo foo = fooFactory(1, 2.3f);
          (void)foo;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_autoinject_instances_bound_to_assisted_params():
    source = '''
        struct X {};
        struct Y {};

        struct Foo {
          Foo(X x, Y y) {
            (void)x;
            (void)y;
          }
        };

        using FooFactory = std::function<Foo()>;

        fruit::Component<FooFactory> getComponent() {
          static X x = X();
          static Y y = Y();
          return fruit::createComponent()
              .bindInstance(x)
              .bindInstance(y)
              .registerFactory<Foo(X, Y)>(
                  [](X x, Y y) {
                    return Foo(x, y);
                  });
        }


        int main() {
          fruit::Injector<FooFactory> injector(getComponent());
          FooFactory fooFactory(injector);
          Foo foo = fooFactory();
          (void)foo;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_autoinject_2_assisted_params_plus_nonassisted_params():
    source = '''
        struct X {};
        struct Y {};
        struct Z {};

        struct Foo {
          Foo(X, Y, int, float, Z) {
          }
        };

        using FooPtrFactory = std::function<std::unique_ptr<Foo>(int, float)>;

        fruit::Component<FooPtrFactory> getComponent() {
          static X x = X();
          static Y y = Y();
          static Z z = Z();
          return fruit::createComponent()
              .bindInstance(x)
              .bindInstance(y)
              .bindInstance(z)
              .registerFactory<std::unique_ptr<Foo>(X, Y, fruit::Assisted<int>, fruit::Assisted<float>, Z)>(
                  [](X x, Y y, int n, float a, Z z) {
                    return std::unique_ptr<Foo>(new Foo(x, y, n, a, z));
                  });
        }

        int main() {
          fruit::Injector<FooPtrFactory> injector(getComponent());
          FooPtrFactory fooPtrFactory(injector);
          std::unique_ptr<Foo> foo = fooPtrFactory(1, 3.4f);
          (void)foo;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_autoinject_2_assisted_params_plus_nonassisted_params_returning_value():
    source = '''
        struct X {};
        struct Y {};
        struct Z {};

        struct Foo {
          Foo(X, Y, int, float, Z) {
          }
        };

        using FooFactory = std::function<Foo(int, float)>;

        fruit::Component<FooFactory> getComponent() {
          static X x = X();
          static Y y = Y();
          static Z z = Z();
          return fruit::createComponent()
              .bindInstance(x)
              .bindInstance(y)
              .bindInstance(z)
              .registerFactory<Foo(X, Y, fruit::Assisted<int>, fruit::Assisted<float>, Z)>(
                  [](X x, Y y, int n, float a, Z z) {
                    return Foo(x, y, n, a, z);
                  });
        }

        int main() {
          fruit::Injector<FooFactory> injector(getComponent());
          FooFactory fooFactory(injector);
          Foo foo = fooFactory(1, 3.4f);
          (void)foo;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_autoinject_mixed_assisted_and_injected_params():
    source = '''
        struct X {};
        struct Y {};

        struct Foo {
          Foo(int, float, X, Y, double) {
          }
        };

        using FooFactory = std::function<Foo(int, float, double)>;

        fruit::Component<FooFactory> getComponent() {
          static X x = X();
          static Y y = Y();
          return fruit::createComponent()
              .bindInstance(x)
              .bindInstance(y)
              .registerFactory<Foo(fruit::Assisted<int>, fruit::Assisted<float>, X, Y, fruit::Assisted<double>)>(
                  [](int n, float a, X x, Y y, double d) {
                    return Foo(n, a, x, y, d);
                  });
        }

        int main() {
          fruit::Injector<FooFactory> injector(getComponent());
          FooFactory fooFactory(injector);
          Foo foo = fooFactory(1, 3.4f, 3.456);
          (void)foo;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_autoinject_annotation_in_signature_return_type():
    source = '''
        struct X {
          using Inject = fruit::Annotated<Annotation1, X>();
        };

        fruit::Component<fruit::Annotated<Annotation1, std::function<std::unique_ptr<X>()>>> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'InjectTypedefWithAnnotationError<X>',
        'C::Inject is a signature that returns an annotated type',
        COMMON_DEFINITIONS,
        source)

def test_autoinject_annotation_in_signature_return_type_returning_value():
    source = '''
        struct X {
          using Inject = fruit::Annotated<Annotation1, X>();
        };

        fruit::Component<fruit::Annotated<Annotation1, std::function<X()>>> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'InjectTypedefWithAnnotationError<X>',
        'C::Inject is a signature that returns an annotated type',
        COMMON_DEFINITIONS,
        source)

def test_autoinject_from_provider():
    source = '''
        struct X {
          INJECT(X()) = default;
        };

        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor, X)
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent()
            .registerProvider([](X x) {
              return std::function<std::unique_ptr<ScalerImpl>(double)>([x](double n){
                return std::unique_ptr<ScalerImpl>(new ScalerImpl(n, x));
              });
            })
            .bind<Scaler, ScalerImpl>();
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params(
    ('Scaler',
     'std::function<std::unique_ptr<Scaler>(double)>',
     'ScalerImpl',
     'std::function<std::unique_ptr<ScalerImpl>(double)>'),
    ('fruit::Annotated<Annotation1, Scaler>',
     'fruit::Annotated<Annotation1, std::function<std::unique_ptr<Scaler>(double)>>',
     'fruit::Annotated<Annotation2, ScalerImpl>',
     'fruit::Annotated<Annotation2, std::function<std::unique_ptr<ScalerImpl>(double)>>'))
def test_autoinject_from_provider(ScalerAnnot, ScalerFactoryAnnot, ScalerImplAnnot, ScalerImplFactoryAnnot):
    source = '''
        struct X {
          INJECT(X()) = default;
        };

        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor, X)
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;
        using ScalerImplFactory = std::function<std::unique_ptr<ScalerImpl>(double)>;

        fruit::Component<ScalerFactoryAnnot> getScalerComponent() {
          return fruit::createComponent()
            .registerProvider<ScalerImplFactoryAnnot(X)>([](X x) {
              return std::function<std::unique_ptr<ScalerImpl>(double)>([x](double n){
                return std::unique_ptr<ScalerImpl>(new ScalerImpl(n, x));
              });
            })
            .bind<ScalerAnnot, ScalerImplAnnot>();
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('ScalerFactory', 'fruit::Annotated<Annotation1, ScalerFactory>')
def test_autoinject_from_provider_returning_value(ScalerFactoryAnnot):
    source = '''
        struct X {
          INJECT(X()) = default;
        };

        struct Scaler {
        private:
          double factor;

        public:
          Scaler(double factor, X)
            : factor(factor) {
          }

          double scale(double x) {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<Scaler(double)>;

        fruit::Component<ScalerFactoryAnnot> getScalerComponent() {
          return fruit::createComponent()
            .registerProvider<ScalerFactoryAnnot(X)>([](X x) {
              return std::function<Scaler(double)>([x](double n){
                return Scaler(n, x);
              });
            });
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
          Scaler scaler = scalerFactory(12.1);
          std::cout << scaler.scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'ANNOTATED(Annotation1, X)')
def test_autoinject_with_binding(X_ANNOT):
    source = '''
        struct X {
          using Inject = X();
        };

        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          INJECT(ScalerImpl(ASSISTED(double) factor, X_ANNOT x))
            : factor(factor) {
              (void)x;
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent()
            .bind<Scaler, ScalerImpl>();
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'ANNOTATED(Annotation1, X)')
def test_autoinject_with_binding_returning_value(X_ANNOT):
    source = '''
        struct X {
          using Inject = X();
        };

        struct Scaler {
        private:
          double factor;

        public:
          INJECT(Scaler(ASSISTED(double) factor, X_ANNOT x))
            : factor(factor) {
              (void)x;
          }

          double scale(double x) {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<Scaler(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          Scaler scaler = scalerFactory(12.1);
          std::cout << scaler.scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_autoinject_with_binding2():
    source = '''
        struct X {
          INJECT(X()) = default;
        };

        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          INJECT(ScalerImpl(X, ASSISTED(double) factor))
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent()
            .bind<Scaler, ScalerImpl>();
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_autoinject_with_binding2_returning_value():
    source = '''
        struct X {
          INJECT(X()) = default;
        };

        struct Scaler {
        private:
          double factor;

        public:
          INJECT(Scaler(X, ASSISTED(double) factor))
            : factor(factor) {
          }

          double scale(double x) {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<Scaler(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          Scaler scaler = scalerFactory(12.1);
          std::cout << scaler.scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params(
    ('Scaler',
     'ScalerImpl',
     'std::function<std::unique_ptr<Scaler>(double)>'),
    ('fruit::Annotated<Annotation1, Scaler>',
     'fruit::Annotated<Annotation2, ScalerImpl>',
     'fruit::Annotated<Annotation1, std::function<std::unique_ptr<Scaler>(double)>>'))
def test_success(ScalerAnnot, ScalerImplAnnot, ScalerFactoryAnnot):
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor)
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactoryAnnot> getScalerComponent() {
          return fruit::createComponent()
            .bind<ScalerAnnot, ScalerImplAnnot>()
            .registerFactory<ScalerImplAnnot(fruit::Assisted<double>)>([](double factor) { return ScalerImpl(factor); });
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_with_annotation_returning_value():
    source = '''
        struct Scaler {
        private:
          double factor;

        public:
          Scaler(double factor)
            : factor(factor) {
          }

          double scale(double x) {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<Scaler(double)>;
        using ScalerFactoryAnnot1 = fruit::Annotated<Annotation1, ScalerFactory>;

        fruit::Component<ScalerFactoryAnnot1> getScalerComponent() {
          return fruit::createComponent()
            .registerFactory<ScalerAnnot1(fruit::Assisted<double>)>(
              [](double factor) {
                  return Scaler(factor);
              });
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
          Scaler scaler = scalerFactory(12.1);
          std::cout << scaler.scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_with_different_annotation():
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor)
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;
        using ScalerFactoryAnnot1 = fruit::Annotated<Annotation1, ScalerFactory>;

        fruit::Component<ScalerFactoryAnnot1> getScalerComponent() {
          return fruit::createComponent()
            .bind<ScalerAnnot1, ScalerImplAnnot2>()
            .registerFactory<ScalerImplAnnot2(fruit::Assisted<double>)>(
                [](double factor) {
                    return ScalerImpl(factor);
                });
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_with_different_annotation_error():
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor)
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;
        using ScalerFactoryAnnot1 = fruit::Annotated<Annotation1, ScalerFactory>;

        fruit::Component<ScalerFactoryAnnot1> getScalerComponent() {
          return fruit::createComponent()
            .bind<ScalerAnnot1, ScalerImplAnnot1>()
            .registerFactory<ScalerImplAnnot2(fruit::Assisted<double>)>([](double factor) { return ScalerImpl(factor); });
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_compile_error(
        'NoBindingFoundError<fruit::Annotated<Annotation1,std::function<std::unique_ptr<ScalerImpl(,std::default_delete<ScalerImpl>)?>\(double\)>>>',
        '',
        COMMON_DEFINITIONS,
        source)


def test_dep_on_provider():
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor)
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent()
            .bind<Scaler, ScalerImpl>()
            .registerProvider([](){return 23;})
            .registerFactory<ScalerImpl(fruit::Assisted<double>, fruit::Provider<int>)>(
                [](double factor, fruit::Provider<int> provider) {
                    return ScalerImpl(factor * provider.get<int>());
                });
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_dep_on_provider_returning_value():
    source = '''
        struct Scaler {
        private:
          double factor;

        public:
          Scaler(double factor)
            : factor(factor) {
          }

          double scale(double x) {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<Scaler(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent()
            .registerProvider([](){return 23;})
            .registerFactory<Scaler(fruit::Assisted<double>, fruit::Provider<int>)>(
                [](double factor, fruit::Provider<int> provider) {
                    return Scaler(factor * provider.get<int>());
                });
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          Scaler scaler = scalerFactory(12.1);
          std::cout << scaler.scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

def test_error_abstract_class():
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor)
            : factor(factor) {
          }

          // Note: here we "forgot" to implement scale() (on purpose, for this test) so ScalerImpl is an abstract class.
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent()
            .bind<Scaler, ScalerImpl>()
            .registerFactory<fruit::Annotated<Annotation1, ScalerImpl>(fruit::Assisted<double>)>([](double) { return (ScalerImpl*)nullptr; });
        }
        '''
    expect_compile_error(
        'CannotConstructAbstractClassError<ScalerImpl>',
        'The specified class can.t be constructed because it.s an abstract class.',
        COMMON_DEFINITIONS,
        source)

def test_error_not_function():
    source = '''
        struct X {
          X(int) {}
        };

        fruit::Component<std::function<X()>> getComponent() {
          int n = 3;
          return fruit::createComponent()
            .registerFactory<X()>([=]{return X(n);});
        }
        '''
    expect_compile_error(
        'LambdaWithCapturesError<.*>',
        'Only lambdas with no captures are supported',
        COMMON_DEFINITIONS,
        source)

@params(
    ('Scaler',
     'ScalerImpl',
     'ScalerImpl*',
     'std::function<std::unique_ptr<Scaler>(double)>',
     'ScalerImpl\*\(fruit::Assisted<double>\)'),
    ('fruit::Annotated<Annotation1, Scaler>',
     'fruit::Annotated<Annotation2, ScalerImpl>',
     'fruit::Annotated<Annotation2, ScalerImpl*>',
     'fruit::Annotated<Annotation2, std::function<std::unique_ptr<Scaler>(double)>>',
     'fruit::Annotated<Annotation2,ScalerImpl\*>\(fruit::Assisted<double>\)'))
def test_for_pointer(ScalerAnnot, ScalerImplAnnot, ScalerImplPtrAnnot, ScalerFactoryAnnot, ScalerImplFactorySignatureAnnotRegex):
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor)
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactoryAnnot> getScalerComponent() {
          return fruit::createComponent()
            .bind<ScalerAnnot, ScalerImplAnnot>()
            .registerFactory<ScalerImplPtrAnnot(fruit::Assisted<double>)>([](double factor) { return new ScalerImpl(factor); });
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_compile_error(
        'FactoryReturningPointerError<ScalerImplFactorySignatureAnnotRegex>',
        'The specified factory returns a pointer. This is not supported',
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('Scaler*',
     'std::function<Scaler(double)>',
     'Scaler\*\(fruit::Assisted<double>\)'),
    ('fruit::Annotated<Annotation1, Scaler*>',
     'fruit::Annotated<Annotation1, std::function<Scaler(double)>>',
     'fruit::Annotated<Annotation1,Scaler\*>\(fruit::Assisted<double>\)'),
)
def test_for_pointer_returning_value(ScalerPtrAnnot, ScalerFactoryAnnot, ScalerFactorySignatureAnnotRegex):
    source = '''
        struct Scaler {
        private:
          double factor;

        public:
          Scaler(double factor)
            : factor(factor) {
          }

          double scale(double x) {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<Scaler(double)>;

        fruit::Component<ScalerFactoryAnnot> getScalerComponent() {
          return fruit::createComponent()
            .registerFactory<ScalerPtrAnnot(fruit::Assisted<double>)>([](double factor) { return new Scaler(factor); });
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
          Scaler scaler = scalerFactory(12.1);
          std::cout << scaler.scale(3) << std::endl;
        }
        '''
    expect_compile_error(
        'FactoryReturningPointerError<ScalerFactorySignatureAnnotRegex>',
        'The specified factory returns a pointer. This is not supported',
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('Scaler',
     'ScalerImpl',
     'std::unique_ptr<ScalerImpl>',
     'std::function<std::unique_ptr<Scaler>(double)>'),
    ('fruit::Annotated<Annotation1, Scaler>',
     'fruit::Annotated<Annotation2, ScalerImpl>',
     'fruit::Annotated<Annotation2, std::unique_ptr<ScalerImpl>>',
     'fruit::Annotated<Annotation1, std::function<std::unique_ptr<Scaler>(double)>>'))
def test_for_unique_pointer(ScalerAnnot, ScalerImplAnnot, ScalerImplPtrAnnot, ScalerFactoryAnnot):
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor)
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactoryAnnot> getScalerComponent() {
          return fruit::createComponent()
            .bind<ScalerAnnot, ScalerImplAnnot>()
            .registerFactory<ScalerImplPtrAnnot(fruit::Assisted<double>)>(
                [](double factor) {
                    return std::unique_ptr<ScalerImpl>(new ScalerImpl(factor));
                });
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('Scaler',
     'std::function<Scaler(double)>'),
    ('fruit::Annotated<Annotation1, Scaler>',
     'fruit::Annotated<Annotation1, std::function<Scaler(double)>>'))
def test_for_unique_pointer_returning_value(ScalerAnnot, ScalerFactoryAnnot):
    source = '''
        struct Scaler {
        private:
          double factor;

        public:
          Scaler(double factor)
            : factor(factor) {
          }

          double scale(double x) {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<Scaler(double)>;

        fruit::Component<ScalerFactoryAnnot> getScalerComponent() {
          return fruit::createComponent()
            .registerFactory<ScalerAnnot(fruit::Assisted<double>)>(
                [](double factor) {
                    return Scaler(factor);
                });
        }

        int main() {
          fruit::Injector<ScalerFactoryAnnot> injector(getScalerComponent());
          ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
          Scaler scaler = scalerFactory(12.1);
          std::cout << scaler.scale(3) << std::endl;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('ScalerImpl', 'fruit::Annotated<Annotation1, ScalerImpl>')
def test_inconsistent_signature(ScalerImplAnnot):
    source = '''
        struct Scaler {
          virtual double scale(double x) = 0;
        };

        struct ScalerImpl : public Scaler {
        private:
          double factor;

        public:
          ScalerImpl(double factor)
            : factor(factor) {
          }

          double scale(double x) override {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent()
            .bind<Scaler, ScalerImplAnnot>()
            .registerFactory<ScalerImplAnnot(fruit::Assisted<double>)>([](float factor) { return ScalerImpl(factor); });
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
          std::cout << scaler->scale(3) << std::endl;
        }
        '''
    expect_compile_error(
        'FunctorSignatureDoesNotMatchError<ScalerImpl\(double\),ScalerImpl\(float\)>',
        'Unexpected functor signature',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_inconsistent_signature_returning_value():
    source = '''
        struct Scaler {
        private:
          double factor;

        public:
          Scaler(double factor)
            : factor(factor) {
          }

          double scale(double x) {
            return x * factor;
          }
        };

        using ScalerFactory = std::function<Scaler(double)>;

        fruit::Component<ScalerFactory> getScalerComponent() {
          return fruit::createComponent()
            .registerFactory<Scaler(fruit::Assisted<double>)>([](float factor) { return Scaler(factor); });
        }

        int main() {
          fruit::Injector<ScalerFactory> injector(getScalerComponent());
          ScalerFactory scalerFactory(injector);
          Scaler scaler = scalerFactory(12.1);
          std::cout << scaler.scale(3) << std::endl;
        }
        '''
    expect_compile_error(
        'FunctorSignatureDoesNotMatchError<Scaler\(double\),Scaler\(float\)>',
        'Unexpected functor signature',
        COMMON_DEFINITIONS,
        source)

def test_nonmovable_ok():
    source = '''
        struct C {
          INJECT(C()) = default;

          C(const C&) = delete;
          C(C&&) = delete;
          C& operator=(const C&) = delete;
          C& operator=(C&&) = delete;
        };

        using CFactory = std::function<std::unique_ptr<C>()>;

        fruit::Component<CFactory> getCFactory() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<CFactory> injector(getCFactory());
          CFactory cFactory(injector);
          std::unique_ptr<C> c = cFactory();
          (void)c;
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

# TODO: this might not be the best error message, maybe we should ignore the constructor entirely in the message,
# or mention that there are other ways to satisfy that dependency.
@params(
    ('X',
     'std::function<X(int)>'),
    ('fruit::Annotated<Annotation1, X>',
     'fruit::Annotated<Annotation1, std::function<X(int)>>'))
def test_not_existing_constructor1(XAnnot, XFactoryAnnot):
    source = '''
        struct X {
          INJECT(X()) = default;
        };

        fruit::Component<XFactoryAnnot> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'FunctorSignatureDoesNotMatchError<XAnnot\(int\),XAnnot\((void)?\)>',
        'Unexpected functor signature',
        COMMON_DEFINITIONS,
        source,
        locals())

# TODO: this might not be the best error message, maybe we should ignore the constructor entirely in the message,
# or mention that there are other ways to satisfy that dependency.
@params(
    ('std::function<std::unique_ptr<X>(int)>',
     'std::unique_ptr<X(,std::default_delete<X>)?>\(int\)',
     'std::unique_ptr<X(,std::default_delete<X>)?>\((void)?\)'),
    ('fruit::Annotated<Annotation1, std::function<std::unique_ptr<X>(int)>>',
     'fruit::Annotated<Annotation1,std::unique_ptr<X(,std::default_delete<X>)?>>\(int\)',
     'fruit::Annotated<Annotation1,std::unique_ptr<X(,std::default_delete<X>)?>>\((void)?\)'))
def test_not_existing_constructor2(XIntFactoryAnnot, XIntFactoryAnnotRegex, XVoidFactoryAnnotRegex):
    source = '''
        struct X {
          using Inject = X();
        };

        fruit::Component<XIntFactoryAnnot> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'FunctorSignatureDoesNotMatchError<XIntFactoryAnnotRegex,XVoidFactoryAnnotRegex>',
        'Unexpected functor signature',
        COMMON_DEFINITIONS,
        source,
        locals())

# TODO: this might not be the best error message, maybe we should ignore the constructor entirely in the message,
# or mention that there are other ways to satisfy that dependency.
@params(
    ('X',
     'std::function<X(int)>'),
    ('fruit::Annotated<Annotation1, X>',
     'fruit::Annotated<Annotation1, std::function<X(int)>>'))
def test_not_existing_constructor2_returning_value(XAnnot, XFactoryAnnot):
    source = '''
        struct X {
          using Inject = X();
        };

        fruit::Component<XFactoryAnnot> getComponent() {
          return fruit::createComponent();
        }
        '''
    expect_compile_error(
        'FunctorSignatureDoesNotMatchError<XAnnot\(int\), XAnnot\((void)?\)>',
        'Unexpected functor signature',
        COMMON_DEFINITIONS,
        source,
        locals())


@params('std::function<X()>', 'fruit::Annotated<Annotation1, std::function<X()>>')
def test_success_factory_movable_only_implicit(XFactoryAnnot):
    source = '''
        struct X {
          INJECT(X()) = default;
          X(X&&) = default;
          X(const X&) = delete;
        };

        fruit::Component<XFactoryAnnot> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<XFactoryAnnot> injector(getComponent());
          injector.get<XFactoryAnnot>()();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'std::function<X()>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, std::function<X()>>'))
def test_success_factory_movable_only_explicit_returning_value(XAnnot, XFactoryAnnot):
    source = '''
        struct X {
          X() = default;
          X(X&&) = default;
          X(const X&) = delete;
        };

        fruit::Component<XFactoryAnnot> getComponent() {
          return fruit::createComponent()
            .registerFactory<XAnnot()>([](){return X();});
        }

        int main() {
          fruit::Injector<XFactoryAnnot> injector(getComponent());
          injector.get<XFactoryAnnot>()();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'std::unique_ptr<X>', 'std::function<std::unique_ptr<X>()>'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, std::unique_ptr<X>>', 'fruit::Annotated<Annotation1, std::function<std::unique_ptr<X>()>>'))
def test_success_factory_movable_only_explicit_returning_pointer(XAnnot, XPtrAnnot, XPtrFactoryAnnot):
    source = '''
        struct X {
          X() = default;
          X(X&&) = default;
          X(const X&) = delete;
        };

        fruit::Component<XPtrFactoryAnnot> getComponent() {
          return fruit::createComponent()
            .registerFactory<XPtrAnnot()>([](){return std::unique_ptr<X>();});
        }

        int main() {
          fruit::Injector<XPtrFactoryAnnot> injector(getComponent());
          injector.get<XPtrFactoryAnnot>()();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('std::function<std::unique_ptr<X>()>', 'fruit::Annotated<Annotation1, std::function<std::unique_ptr<X>()>>')
def test_success_factory_not_movable_implicit(XPtrFactoryAnnot):
    source = '''
        struct X {
          INJECT(X()) = default;
          X(X&&) = delete;
          X(const X&) = delete;
        };

        fruit::Component<XPtrFactoryAnnot> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::Injector<XPtrFactoryAnnot> injector(getComponent());
          injector.get<XPtrFactoryAnnot>()();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('std::unique_ptr<X>', 'std::function<std::unique_ptr<X>()>'),
    ('fruit::Annotated<Annotation1, std::unique_ptr<X>>', 'fruit::Annotated<Annotation1, std::function<std::unique_ptr<X>()>>'))
def test_success_factory_not_movable_explicit_returning_pointer_with_annotation(XPtrAnnot, XPtrFactoryAnnot):
    source = '''
        struct X {
          X() = default;
          X(X&&) = delete;
          X(const X&) = delete;
        };

        fruit::Component<XPtrFactoryAnnot> getComponent() {
          return fruit::createComponent()
            .registerFactory<XPtrAnnot()>([](){return std::unique_ptr<X>();});
        }

        int main() {
          fruit::Injector<XPtrFactoryAnnot> injector(getComponent());
          injector.get<XPtrFactoryAnnot>()();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())


if __name__ == '__main__':
    import nose2
    nose2.main()
