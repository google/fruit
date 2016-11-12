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

from fruit_test_common import *

COMMON_DEFINITIONS = '''
struct X;
struct Scaler;
struct ScalerImpl;

struct Annotation {};
using XAnnot = fruit::Annotated<Annotation, X>;
using ScalerAnnot = fruit::Annotated<Annotation, Scaler>;
using ScalerImplAnnot = fruit::Annotated<Annotation, ScalerImpl>;

struct Annotation1 {};
using ScalerAnnot1 = fruit::Annotated<Annotation1, Scaler>;
using ScalerImplAnnot1 = fruit::Annotated<Annotation1, ScalerImpl>;

struct Annotation2 {};
using ScalerAnnot2 = fruit::Annotated<Annotation2, Scaler>;
using ScalerImplAnnot2 = fruit::Annotated<Annotation2, ScalerImpl>;
'''

def test_success():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

class ScalerImpl : public Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>()
    .registerFactory<ScalerImpl(Assisted<double>)>([](double factor) { return ScalerImpl(factor); });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_success():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
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

using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_error_abstract_class():
    expect_compile_error(
    'NoBindingFoundForAbstractClassError<Scaler>',
    'No explicit binding was found for C, and C is an abstract class',
    COMMON_DEFINITIONS + '''
struct X {};

class Scaler {
private:
  double factor;

public:
  Scaler(double factor, X)
    : factor(factor) {
  }

  virtual double scale(double x) = 0;
};

using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent();
}
''')

def test_autoinject_error_abstract_class_with_annotation():
    expect_compile_error(
    'NoBindingFoundForAbstractClassError<ScalerImpl>',
    'No explicit binding was found for C, and C is an abstract class',
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
private:
  double factor;

public:
  ScalerImpl(double factor)
    : factor(factor) {
  }

  // Note: here we "forgot" to implement scale() (on purpose, for this test) so ScalerImpl is an abstract class.
};

using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;
using ScalerFactoryAnnot = fruit::Annotated<Annotation, std::function<std::unique_ptr<Scaler>(double)>>;

Component<ScalerFactoryAnnot> getScalerComponent() {
  return fruit::createComponent()
    .bind<ScalerAnnot, ScalerImplAnnot>();
}
''')


def test_autoinject_nonmovable_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
struct I {};

struct C : public I {
  INJECT(C()) = default;

  C(const C&) = delete;
  C(C&&) = delete;
  C& operator=(const C&) = delete;
  C& operator=(C&&) = delete;
};

using IFactory = std::function<std::unique_ptr<I>()>;

Component<IFactory> getIFactory() {
  return fruit::createComponent()
      .bind<I, C>();
}

int main() {
  Injector<IFactory> injector(getIFactory());
  IFactory iFactory(injector);
  std::unique_ptr<I> i = iFactory();
  (void)i;

  return 0;
}
''')

def test_autoinject_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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
using ScalerFactoryAnnot = fruit::Annotated<Annotation, std::function<std::unique_ptr<Scaler>(double)>>;

Component<ScalerFactoryAnnot> getScalerComponent() {
  return fruit::createComponent()
    .bind<ScalerAnnot, ScalerImpl>();
}

int main() {
  Injector<ScalerFactoryAnnot> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot>();
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject2():
    expect_success(
    COMMON_DEFINITIONS + '''
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
  Injector<FooFactory> injector(getComponent());
  FooFactory fooFactory(injector);
  Foo foo = fooFactory(1, 2.3);
  (void)foo;

  return 0;
}
''')

def test_autoinject2_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
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
  Injector<FooFactory> injector(getComponent());
  FooFactory fooFactory(injector);
  Foo foo = fooFactory(1, 2.3);
  (void)foo;

  return 0;
}
''')

def test_autoinject3():
    expect_success(
    COMMON_DEFINITIONS + '''
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

  return 0;
}
''')

def test_autoinject4():
    expect_success(
    COMMON_DEFINITIONS + '''
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
  Foo foo = fooFactory(1, 3.4);
  (void)foo;

  return 0;
}
''')

def test_autoinject4_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
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
  Foo foo = fooFactory(1, 3.4);
  (void)foo;

  return 0;
}
''')

def test_autoinject5():
    expect_success(
    COMMON_DEFINITIONS + '''
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
  Foo foo = fooFactory(1, 3.4, 3.456);
  (void)foo;

  return 0;
}
''')

def test_autoinject_annotation_in_signature_return_type():
    expect_compile_error(
    'InjectTypedefWithAnnotationError<X>',
    'C::Inject is a signature that returns an annotated type',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = XAnnot();
};

fruit::Component<fruit::Annotated<Annotation, std::function<std::unique_ptr<X>()>>> getComponent() {
  return fruit::createComponent();
}
''')

def test_autoinject_annotation_in_signature_return_type_returning_value():
    expect_compile_error(
    'InjectTypedefWithAnnotationError<X>',
    'C::Inject is a signature that returns an annotated type',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = XAnnot();
};

fruit::Component<fruit::Annotated<Annotation, std::function<X()>>> getComponent() {
  return fruit::createComponent();
}
''')

def test_autoinject_from_provider():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .registerProvider([](X x) {
      return std::function<std::unique_ptr<ScalerImpl>(double)>([x](double n){
        return std::unique_ptr<ScalerImpl>(new ScalerImpl(n, x));
      });
    })
    .bind<Scaler, ScalerImpl>();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_from_provider_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .registerProvider([](X x) {
      return std::function<Scaler(double)>([x](double n){
        return Scaler(n, x);
      });
    });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_from_provider_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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
using ScalerFactoryAnnot1 = fruit::Annotated<Annotation1, ScalerFactory>;
using ScalerImplFactory = std::function<std::unique_ptr<ScalerImpl>(double)>;
using ScalerImplFactoryAnnot2 = fruit::Annotated<Annotation2, ScalerImplFactory>;

Component<ScalerFactoryAnnot1> getScalerComponent() {
  return fruit::createComponent()
    .registerProvider<ScalerImplFactoryAnnot2(X)>([](X x) {
      return std::function<std::unique_ptr<ScalerImpl>(double)>([x](double n){
        return std::unique_ptr<ScalerImpl>(new ScalerImpl(n, x));
      });
    })
    .bind<ScalerAnnot1, ScalerImplAnnot2>();
}

int main() {
  Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_from_provider_with_annotation_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
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
using ScalerFactoryAnnot1 = fruit::Annotated<Annotation1, ScalerFactory>;

Component<ScalerFactoryAnnot1> getScalerComponent() {
  return fruit::createComponent()
    .registerProvider<ScalerFactoryAnnot1(X)>([](X x) {
      return std::function<Scaler(double)>([x](double n){
        return Scaler(n, x);
      });
    });
}

int main() {
  Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_with_binding():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
private:
  double factor;

public:
  INJECT(ScalerImpl(ASSISTED(double) factor, X))
    : factor(factor) {
  }

  double scale(double x) override {
    return x * factor;
  }
};

using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_with_binding_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_with_binding2():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_with_binding_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
};

class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
private:
  double factor;

public:
  INJECT(ScalerImpl(ASSISTED(double) factor, ANNOTATED(Annotation, X) x))
    : factor(factor) {
      (void)x;
  }

  double scale(double x) override {
    return x * factor;
  }
};

using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_with_binding_with_annotation_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
};

class Scaler {
private:
  double factor;

public:
  INJECT(Scaler(ASSISTED(double) factor, ANNOTATED(Annotation, X) x))
    : factor(factor) {
      (void)x;
  }

  double scale(double x) {
    return x * factor;
  }
};

using ScalerFactory = std::function<Scaler(double)>;

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_autoinject_with_binding2_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

class Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_dep_on_provider():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>()
    .registerProvider([](){return 23;})
    .registerFactory<ScalerImpl(Assisted<double>, Provider<int>)>(
        [](double factor, Provider<int> provider) {
            return ScalerImpl(factor * provider.get<int>());
        });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_dep_on_provider_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .registerProvider([](){return 23;})
    .registerFactory<Scaler(Assisted<double>, Provider<int>)>(
        [](double factor, Provider<int> provider) {
            return Scaler(factor * provider.get<int>());
        });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_error_abstract_class():
    expect_compile_error(
    'CannotConstructAbstractClassError<ScalerImpl>',
    'The specified class can.t be constructed because it.s an abstract class.',
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
private:
  double factor;

public:
  ScalerImpl(double factor)
    : factor(factor) {
  }

  // Note: here we "forgot" to implement scale() (on purpose, for this test) so ScalerImpl is an abstract class.
};

using ScalerFactory = std::function<std::unique_ptr<Scaler>(double)>;

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>()
    .registerFactory<ScalerImplAnnot(Assisted<double>)>([](double) { return (ScalerImpl*)nullptr; });
}
''')

def test_error_not_function():
    expect_compile_error(
    'LambdaWithCapturesError<.*>',
    'Only lambdas with no captures are supported',
    COMMON_DEFINITIONS + '''
struct X {
  X(int) {}
};

Component<std::function<X()>> getComponent() {
  int n = 3;
  return fruit::createComponent()
    .registerFactory<X()>([=]{return X(n);});
}
''')

def test_for_pointer():
    expect_compile_error(
    'FactoryReturningPointerError<ScalerImpl\*\(fruit::Assisted<double>\)>',
    'The specified factory returns a pointer. This is not supported',
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>()
    .registerFactory<ScalerImpl*(Assisted<double>)>([](double factor) { return new ScalerImpl(factor); });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_for_pointer_returning_value():
    expect_compile_error(
    'FactoryReturningPointerError<Scaler\*\(fruit::Assisted<double>\)>',
    'The specified factory returns a pointer. This is not supported',
    COMMON_DEFINITIONS + '''
class Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .registerFactory<Scaler*(Assisted<double>)>([](double factor) { return new Scaler(factor); });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_for_pointer_with_annotation():
    expect_compile_error(
    'FactoryReturningPointerError<fruit::Annotated<Annotation2,ScalerImpl\*>\(fruit::Assisted<double>\)>',
    'The specified factory returns a pointer. This is not supported',
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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
using ScalerFactoryAnnot2 = fruit::Annotated<Annotation2, ScalerFactory>;

Component<ScalerFactoryAnnot2> getScalerComponent() {
  return fruit::createComponent()
    .bind<ScalerAnnot1, ScalerImplAnnot2>()
    .registerFactory<fruit::Annotated<Annotation2, ScalerImpl*>(Assisted<double>)>([](double factor) { return new ScalerImpl(factor); });
}

int main() {
  Injector<ScalerFactoryAnnot2> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot2>();
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_for_pointer_with_annotation_returning_value():
    expect_compile_error(
    'FactoryReturningPointerError<fruit::Annotated<Annotation1,Scaler\*>\(fruit::Assisted<double>\)>',
    'The specified factory returns a pointer. This is not supported',
    COMMON_DEFINITIONS + '''
class Scaler {
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

Component<ScalerFactoryAnnot1> getScalerComponent() {
  return fruit::createComponent()
    .registerFactory<fruit::Annotated<Annotation1, Scaler*>(Assisted<double>)>([](double factor) { return new Scaler(factor); });
}

int main() {
  Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_for_unique_pointer():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>()
    .registerFactory<std::unique_ptr<ScalerImpl>(Assisted<double>)>([](double factor) { return std::unique_ptr<ScalerImpl>(new ScalerImpl(factor)); });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_for_unique_pointer_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .registerFactory<Scaler(Assisted<double>)>([](double factor) { return Scaler(factor); });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_for_unique_pointer_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactoryAnnot1> getScalerComponent() {
  return fruit::createComponent()
    .bind<ScalerAnnot1, ScalerImplAnnot2>()
    .registerFactory<fruit::Annotated<Annotation2, std::unique_ptr<ScalerImpl>>(Assisted<double>)>(
        [](double factor) {
            return std::unique_ptr<ScalerImpl>(new ScalerImpl(factor));
        });
}

int main() {
  Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_for_unique_pointer_with_annotation_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
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

Component<ScalerFactoryAnnot1> getScalerComponent() {
  return fruit::createComponent()
    .registerFactory<ScalerAnnot1(Assisted<double>)>(
        [](double factor) {
            return Scaler(factor);
        });
}

int main() {
  Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_inconsistent_signature():
    expect_compile_error(
    'FunctorSignatureDoesNotMatchError<ScalerImpl\(double\),ScalerImpl\(float\)>',
    'Unexpected functor signature',
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImpl>()
    .registerFactory<ScalerImpl(Assisted<double>)>([](float factor) { return ScalerImpl(factor); });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_inconsistent_signature_returning_value():
    expect_compile_error(
    'FunctorSignatureDoesNotMatchError<Scaler\(double\),Scaler\(float\)>',
    'Unexpected functor signature',
    COMMON_DEFINITIONS + '''
class Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .registerFactory<Scaler(Assisted<double>)>([](float factor) { return Scaler(factor); });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_inconsistent_signature_with_annotations():
    expect_compile_error(
    'FunctorSignatureDoesNotMatchError<ScalerImpl\(double\),ScalerImpl\(float\)>',
    'Unexpected functor signature',
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactory> getScalerComponent() {
  return fruit::createComponent()
    .bind<Scaler, ScalerImplAnnot>()
    .registerFactory<ScalerImplAnnot(Assisted<double>)>([](float factor) { return ScalerImpl(factor); });
}

int main() {
  Injector<ScalerFactory> injector(getScalerComponent());
  ScalerFactory scalerFactory(injector);
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_nonmovable_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
struct C {
  INJECT(C()) = default;

  C(const C&) = delete;
  C(C&&) = delete;
  C& operator=(const C&) = delete;
  C& operator=(C&&) = delete;
};

using CFactory = std::function<std::unique_ptr<C>()>;

Component<CFactory> getCFactory() {
  return fruit::createComponent();
}

int main() {
  Injector<CFactory> injector(getCFactory());
  CFactory cFactory(injector);
  std::unique_ptr<C> c = cFactory();
  (void)c;

  return 0;
}
''')

def test_not_existing_constructor1():
    expect_compile_error(
    'FunctorSignatureDoesNotMatchError<X\(int\),X\(\)>',
    'Unexpected functor signature',
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

fruit::Component<std::function<X(int)>> getComponent() {
  return fruit::createComponent();
}

int main() {
  return 0;
}
''')

def test_not_existing_constructor1_with_annotation():
    expect_compile_error(
    'FunctorSignatureDoesNotMatchError<fruit::Annotated<Annotation,X>\(int\),fruit::Annotated<Annotation,X>\(\)>',
    'Unexpected functor signature',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
  X() = default;
};

fruit::Component<fruit::Annotated<Annotation, std::function<X(int)>>> getComponent() {
  return fruit::createComponent();
}
''')

def test_not_existing_constructor2():
    expect_compile_error(
    'FunctorSignatureDoesNotMatchError<std::unique_ptr<X(,std::default_delete<X>)?>\(int\),std::unique_ptr<X(,std::default_delete<X>)?>\(\)>',
    'Unexpected functor signature',
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

fruit::Component<std::function<std::unique_ptr<X>(int)>> getComponent() {
  return fruit::createComponent();
}
''')

def test_not_existing_constructor2_returning_value():
    expect_compile_error(
    'FunctorSignatureDoesNotMatchError<X\(int\),X\(\)>',
    'Unexpected functor signature',
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

fruit::Component<std::function<X(int)>> getComponent() {
  return fruit::createComponent();
}
''')

def test_not_existing_constructor2_with_annotation():
    expect_compile_error(
    'FunctorSignatureDoesNotMatchError<fruit::Annotated<Annotation,std::unique_ptr<X(,std::default_delete<X>)?>>\(int\),fruit::Annotated<Annotation,std::unique_ptr<X(,std::default_delete<X>)?>>\(\)>',
    'Unexpected functor signature',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
};

fruit::Component<fruit::Annotated<Annotation, std::function<std::unique_ptr<X>(int)>>> getComponent() {
  return fruit::createComponent();
}
''')

def test_not_existing_constructor2_with_annotation_returning_value():
    expect_compile_error(
    'FunctorSignatureDoesNotMatchError<fruit::Annotated<Annotation,X>\(int\),fruit::Annotated<Annotation,X>\(\)>',
    'Unexpected functor signature',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
};

fruit::Component<fruit::Annotated<Annotation, std::function<X(int)>>> getComponent() {
  return fruit::createComponent();
}

int main() {
  return 0;
}
''')

def test_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactoryAnnot1> getScalerComponent() {
  return fruit::createComponent()
    .bind<ScalerAnnot1, ScalerImplAnnot2>()
    .registerFactory<ScalerImplAnnot2(Assisted<double>)>(
      [](double factor) {
          return ScalerImpl(factor);
      });
}

int main() {
  Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_with_annotation_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
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

Component<ScalerFactoryAnnot1> getScalerComponent() {
  return fruit::createComponent()
    .registerFactory<ScalerAnnot1(Assisted<double>)>(
      [](double factor) {
          return Scaler(factor);
      });
}

int main() {
  Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
  Scaler scaler = scalerFactory(12.1);
  std::cout << scaler.scale(3) << std::endl;

  return 0;
}
''')

def test_with_different_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactoryAnnot1> getScalerComponent() {
  return fruit::createComponent()
    .bind<ScalerAnnot1, ScalerImplAnnot2>()
    .registerFactory<ScalerImplAnnot2(Assisted<double>)>(
        [](double factor) {
            return ScalerImpl(factor);
        });
}

int main() {
  Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

def test_with_different_annotation_error():
    expect_compile_error(
    'NoBindingFoundError<fruit::Annotated<Annotation1,std::function<std::unique_ptr<ScalerImpl(,std::default_delete<ScalerImpl>)?>\(double\)>>>',
    '',
    COMMON_DEFINITIONS + '''
class Scaler {
public:
  virtual double scale(double x) = 0;
};

class ScalerImpl : public Scaler {
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

Component<ScalerFactoryAnnot1> getScalerComponent() {
  return fruit::createComponent()
    .bind<ScalerAnnot1, ScalerImplAnnot1>()
    .registerFactory<ScalerImplAnnot2(Assisted<double>)>([](double factor) { return ScalerImpl(factor); });
}

int main() {
  Injector<ScalerFactoryAnnot1> injector(getScalerComponent());
  ScalerFactory scalerFactory = injector.get<ScalerFactoryAnnot1>();
  std::unique_ptr<Scaler> scaler = scalerFactory(12.1);
  std::cout << scaler->scale(3) << std::endl;

  return 0;
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
