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
#include <fruit/fruit.h>
#include <vector>
#include "test_macros.h"

struct X;
struct Y;

struct Annotation {};
using XAnnot = fruit::Annotated<Annotation, X>;
using YAnnot = fruit::Annotated<Annotation, Y>;

struct Annotation1 {};
using XAnnot1 = fruit::Annotated<Annotation1, X>;
using YAnnot1 = fruit::Annotated<Annotation1, Y>;

struct Annotation2 {};
using XAnnot2 = fruit::Annotated<Annotation2, X>;
using YAnnot2 = fruit::Annotated<Annotation2, Y>;
'''

def test_empty_injector():
    expect_success(
    COMMON_DEFINITIONS + '''
fruit::Component<> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<> injector(getComponent());
  return 0;
}
''')

def test_error_component_with_requirements():
    expect_compile_error(
    'ComponentWithRequirementsInInjectorError<X>',
    'When using the two-argument constructor of Injector, the component used as second parameter must not have requirements',
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

fruit::Component<fruit::Required<X>> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<X> normalizedComponent(fruit::createComponent());
  fruit::Injector<X> injector(normalizedComponent, getComponent());

  return 0;
}
''')

def test_error_component_with_requirements_with_annotation():
    expect_compile_error(
    'ComponentWithRequirementsInInjectorError<fruit::Annotated<Annotation,X>>',
    'When using the two-argument constructor of Injector, the component used as second parameter must not have requirements',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
};

fruit::Component<fruit::Required<XAnnot>> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<XAnnot> normalizedComponent(fruit::createComponent());
  fruit::Injector<XAnnot> injector(normalizedComponent, getComponent());

  return 0;
}
''')

def test_error_types_not_provided():
    expect_compile_error(
    'TypesInInjectorNotProvidedError<X>',
    'The types in TypesNotProvided are declared as provided by the injector, but none of the two components passed to the Injector constructor provides them.',
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

int main() {
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  fruit::Injector<X> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));

  return 0;
}
''')

def test_error_types_not_provided_with_annotation():
    expect_compile_error(
    'TypesInInjectorNotProvidedError<fruit::Annotated<Annotation,X>>',
    'The types in TypesNotProvided are declared as provided by the injector, but none of the two components passed to the Injector constructor provides them.',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = XAnnot();
};

int main() {
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  fruit::Injector<XAnnot> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));

  return 0;
}
''')

def test_error_repeated_type():
    expect_compile_error(
    'RepeatedTypesError<X,X>',
    'A type was specified more than once.',
    COMMON_DEFINITIONS + '''
struct X {};

void f() {
    (void) sizeof(fruit::Injector<X, X>);
}
''')

def test_error_repeated_type_with_annotation():
    expect_compile_error(
    'RepeatedTypesError<fruit::Annotated<Annotation,X>,fruit::Annotated<Annotation,X>>',
    'A type was specified more than once.',
    COMMON_DEFINITIONS + '''
struct X {};

void f() {
    (void) sizeof(fruit::Injector<XAnnot, XAnnot>);
}
''')

def test_repeated_type_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

int main() {
    (void) sizeof(fruit::Injector<XAnnot1, XAnnot2>);
    return 0;
}
''')

def test_error_non_class_type():
    expect_compile_error(
    'NonClassTypeError<X\*,X>',
    'A non-class type T was specified. Use C instead.',
    COMMON_DEFINITIONS + '''
struct X {};

void f() {
    (void) sizeof(fruit::Injector<X*>);
}
''')

def test_error_non_class_type_with_annotation():
    expect_compile_error(
    'NonClassTypeError<X\*,X>',
    'A non-class type T was specified. Use C instead.',
    COMMON_DEFINITIONS + '''
struct X {};

void f() {
    (void) sizeof(fruit::Injector<fruit::Annotated<Annotation, X*>>);
}
''')

def test_error_requirements_in_injector():
    expect_compile_error(
    'InjectorWithRequirementsError<Y>',
    'Injectors can.t have requirements.',
    COMMON_DEFINITIONS + '''
struct Y {};

struct X {
  INJECT(X(Y)) {
  }
};

fruit::Component<fruit::Required<Y>, X> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<fruit::Required<Y>, X> injector(getComponent());
  return 0;
}
''')

def test_error_requirements_in_injector_with_annotation():
    expect_compile_error(
    'InjectorWithRequirementsError<fruit::Annotated<Annotation,Y>>',
    'Injectors can.t have requirements.',
    COMMON_DEFINITIONS + '''
struct Y {};

struct X {
  using Inject = XAnnot(YAnnot);
  X(Y) {
  }
};

fruit::Component<fruit::Required<YAnnot>, XAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<fruit::Required<YAnnot>, XAnnot> injector(getComponent());
  return 0;
}
''')

def test_error_type_not_provided():
    expect_compile_error(
    'TypeNotProvidedError<Y>',
    'Trying to get an instance of T, but it is not provided by this Provider/Injector.',
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

struct Y {};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<Y>();

  return 0;
}
''')

def test_error_type_not_provided_with_annotation():
    expect_compile_error(
    'TypeNotProvidedError<fruit::Annotated<Annotation2,Y>>',
    'Trying to get an instance of T, but it is not provided by this Provider/Injector.',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
};

struct Y {};

fruit::Component<XAnnot1> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<XAnnot1> injector(getComponent());
  injector.get<YAnnot2>();

  return 0;
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
