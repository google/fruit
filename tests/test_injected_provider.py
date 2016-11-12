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
struct Y;

struct Annotation {};
using XAnnot = fruit::Annotated<Annotation, X>;

struct Annotation1 {};
using XAnnot1 = fruit::Annotated<Annotation1, X>;
using YAnnot1 = fruit::Annotated<Annotation1, Y>;

struct Annotation2 {};
using XAnnot2 = fruit::Annotated<Annotation2, X>;
using YAnnot2 = fruit::Annotated<Annotation2, Y>;
'''

def test_error_non_class_type_parameter():
    expect_compile_error(
    'NonClassTypeError<X\*,X>',
    'A non-class type T was specified. Use C instead',
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Provider<X*> provider;
''')

def test_error_annotated_type_parameter():
    expect_compile_error(
    'AnnotatedTypeError<fruit::Annotated<Annotation,X>,X>',
    'An annotated type was specified where a non-annotated type was expected.',
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Provider<XAnnot> provider;
''')

def test_get_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
bool x_constructed = false;

struct X {
  INJECT(X()) {
    x_constructed = true;
  }
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<X> injector(getComponent());
  Provider<X> provider = injector.get<fruit::Provider<X>>();

  Assert(!x_constructed);

  X& x = provider.get<X&>();
  (void)x;

  Assert(x_constructed);

  return 0;
}
''')

def test_get_ok_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
bool x_constructed = false;

struct X {
  using Inject = X();
  X() {
    x_constructed = true;
  }
};

fruit::Component<XAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<XAnnot> injector(getComponent());
  Provider<X> provider = injector.get<fruit::Annotated<Annotation, fruit::Provider<X>>>();

  Assert(!x_constructed);

  X& x = provider.get<X&>();
  (void)x;

  Assert(x_constructed);

  return 0;
}
''')

def test_get_during_injection_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  void foo() {
  }
};

struct Y {
  X x;
  INJECT(Y(Provider<X> xProvider))
    : x(xProvider.get<X>()) {
  }

  void foo() {
    x.foo();
  }
};

struct Z {
  Y y;
  INJECT(Z(Provider<Y> yProvider))
      : y(yProvider.get<Y>()) {
  }

  void foo() {
    y.foo();
  }
};

Component<Z> getZComponent() {
  return fruit::createComponent();
}

int main() {
  Injector<Z> injector(getZComponent());
  Provider<Z> provider(injector);
  // During provider.get<Z>(), yProvider.get() is called, and during that xProvider.get()
  // is called.
  Z z = provider.get<Z>();
  z.foo();
  return 0;
}
''')

def test_get_error_type_not_provided():
    expect_compile_error(
    'TypeNotProvidedError<Y>',
    'Trying to get an instance of T, but it is not provided by this Provider/Injector.',
    COMMON_DEFINITIONS + '''
struct X {};
struct Y {};

void f(fruit::Provider<X> provider) {
  provider.get<Y>();
}
''')

def test_get_error_type_pointer_pointer_not_provided():
    expect_compile_error(
    'TypeNotProvidedError<X\*\*>',
    'Trying to get an instance of T, but it is not provided by this Provider/Injector.',
    COMMON_DEFINITIONS + '''
struct X {};

void f(fruit::Provider<X> provider) {
  provider.get<X**>();
}
''')

def test_lazy_injection():
    expect_success(
    COMMON_DEFINITIONS + '''
struct Y {
  INJECT(Y()) {
    Assert(!constructed);
    constructed = true;
  }

  static bool constructed;
};

bool Y::constructed = false;

struct X {
  INJECT(X(fruit::Provider<Y> provider)) : provider(provider) {
    Assert(!constructed);
    constructed = true;
  }

  void run() {
    Y* y(provider);
    (void) y;
  }

  fruit::Provider<Y> provider;

  static bool constructed;
};

bool X::constructed = false;

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  Injector<X> injector(normalizedComponent, getComponent());

  Assert(!X::constructed);
  Assert(!Y::constructed);

  X* x(injector);

  Assert(X::constructed);
  Assert(!Y::constructed);

  x->run();

  Assert(X::constructed);
  Assert(Y::constructed);

  return 0;
}
''')

def test_lazy_injection_with_annotations():
    expect_success(
    COMMON_DEFINITIONS + '''
struct Y {
  using Inject = Y();
  Y() {
    Assert(!constructed);
    constructed = true;
  }

  static bool constructed;
};

bool Y::constructed = false;

struct X {
  INJECT(X(ANNOTATED(Annotation1, fruit::Provider<Y>) provider)) : provider(provider) {
    Assert(!constructed);
    constructed = true;
  }

  void run() {
    Y* y(provider);
    (void) y;
  }

  fruit::Provider<Y> provider;

  static bool constructed;
};

bool X::constructed = false;

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  Injector<X> injector(normalizedComponent, getComponent());

  Assert(!X::constructed);
  Assert(!Y::constructed);

  X* x(injector);

  Assert(X::constructed);
  Assert(!Y::constructed);

  x->run();

  Assert(X::constructed);
  Assert(Y::constructed);

  return 0;
}
''')

def test_lazy_injection_with_annotations2():
    expect_success(
    COMMON_DEFINITIONS + '''
struct Y {
  using Inject = Y();
  Y() {
    Assert(!constructed);
    constructed = true;
  }

  static bool constructed;
};

bool Y::constructed = false;

struct X {
  using Inject = X(fruit::Annotated<Annotation1, fruit::Provider<Y>>);
  X(fruit::Provider<Y> provider) : provider(provider) {
    Assert(!constructed);
    constructed = true;
  }

  void run() {
    Y* y(provider);
    (void) y;
  }

  fruit::Provider<Y> provider;

  static bool constructed;
};

bool X::constructed = false;

fruit::Component<XAnnot2> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  Injector<XAnnot2> injector(normalizedComponent, getComponent());

  Assert(!X::constructed);
  Assert(!Y::constructed);

  X* x = injector.get<fruit::Annotated<Annotation2, X*>>();

  Assert(X::constructed);
  Assert(!Y::constructed);

  x->run();

  Assert(X::constructed);
  Assert(Y::constructed);

  return 0;
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
