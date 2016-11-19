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

struct Annotation1 {};
using XAnnot1 = fruit::Annotated<Annotation1, X>;
using YAnnot1 = fruit::Annotated<Annotation1, Y>;

struct Annotation2 {};
using XAnnot2 = fruit::Annotated<Annotation2, X>;
using YAnnot2 = fruit::Annotated<Annotation2, Y>;
'''

def test_success_normalized_component_provides_unused():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

struct Y {
  INJECT(Y(X)) {};
};

fruit::Component<fruit::Required<X>, Y> getComponent() {
  return fruit::createComponent();
}

fruit::Component<X> getXComponent(X& x) {
  return fruit::createComponent()
    .bindInstance(x);
}

int main() {
  fruit::NormalizedComponent<fruit::Required<X>, Y> normalizedComponent(getComponent());

  X x{};

  fruit::Injector<X> injector(normalizedComponent, getXComponent(x));
  injector.get<X*>();
}
''')

def test_success():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

struct Y {
  INJECT(Y(X)) {};
};

fruit::Component<fruit::Required<X>, Y> getComponent() {
  return fruit::createComponent();
}

fruit::Component<X> getXComponent(X& x) {
  return fruit::createComponent()
    .bindInstance(x);
}

int main() {
  fruit::NormalizedComponent<fruit::Required<X>, Y> normalizedComponent(getComponent());

  X x{};

  fruit::Injector<Y> injector(normalizedComponent, getXComponent(x));
  injector.get<Y*>();
}
''')

def test_success_inline_component():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

struct Y {
  INJECT(Y(X)) {};
};

fruit::Component<fruit::Required<X>, Y> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<fruit::Required<X>, Y> normalizedComponent(getComponent());

  X x{};

  fruit::Injector<Y> injector(normalizedComponent, fruit::Component<X>(fruit::createComponent().bindInstance(x)));
  injector.get<Y*>();
}
''')

def test_success_normalized_component_provides_unused_with_annotations():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

struct Y {
  INJECT(Y(ANNOTATED(Annotation1, X))) {};
};

fruit::Component<fruit::Required<XAnnot1>, YAnnot2> getComponent() {
  return fruit::createComponent();
}

fruit::Component<XAnnot1> getXComponent(X& x) {
  return fruit::createComponent()
    .bindInstance<XAnnot1>(x);
}

int main() {
  fruit::NormalizedComponent<fruit::Required<XAnnot1>, YAnnot2> normalizedComponent(getComponent());

  X x{};

  fruit::Injector<XAnnot1> injector(normalizedComponent, getXComponent(x));
  injector.get<XAnnot1>();
}
''')

def test_success_with_annotations():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

struct Y {
  INJECT(Y(ANNOTATED(Annotation1, X))) {};
};

fruit::Component<fruit::Required<XAnnot1>, YAnnot2> getComponent() {
  return fruit::createComponent();
}

fruit::Component<XAnnot1> getXComponent(X& x) {
  return fruit::createComponent()
    .bindInstance<XAnnot1>(x);
}

int main() {
  fruit::NormalizedComponent<fruit::Required<XAnnot1>, YAnnot2> normalizedComponent(getComponent());

  X x{};

  fruit::Injector<YAnnot2> injector(normalizedComponent, getXComponent(x));
  injector.get<YAnnot2>();
}
''')

def test_success_inline_component_with_annotations():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

struct Y {
  INJECT(Y(ANNOTATED(Annotation1, X))) {};
};

fruit::Component<fruit::Required<XAnnot1>, YAnnot2> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<fruit::Required<XAnnot1>, YAnnot2> normalizedComponent(getComponent());

  X x{};

  fruit::Injector<YAnnot2> injector(normalizedComponent, fruit::Component<XAnnot1>(fruit::createComponent().bindInstance<XAnnot1>(x)));
  injector.get<YAnnot2>();
}
''')

def test_unsatisfied_requirements():
    expect_compile_error(
    'UnsatisfiedRequirementsInNormalizedComponentError<X>',
    'The requirements in UnsatisfiedRequirements are required by the NormalizedComponent but are not provided by the Component',
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

fruit::Component<fruit::Required<X>> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<fruit::Required<X>> normalizedComponent(getComponent());
  fruit::Injector<> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));
}
''')

def test_unsatisfied_requirements_with_annotation():
    expect_compile_error(
    'UnsatisfiedRequirementsInNormalizedComponentError<fruit::Annotated<Annotation,X>>',
    'The requirements in UnsatisfiedRequirements are required by the NormalizedComponent but are not provided by the Component',
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
};

fruit::Component<fruit::Required<XAnnot>> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<fruit::Required<XAnnot>> normalizedComponent(getComponent());
  fruit::Injector<> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));
}
''')

def test_error_repeated_type():
    expect_compile_error(
    'RepeatedTypesError<X,X>',
    'A type was specified more than once.',
    COMMON_DEFINITIONS + '''
struct X {};

void f() {
    (void) sizeof(fruit::NormalizedComponent<X, X>);
}
''')

def test_error_repeated_type_with_annotation():
    expect_compile_error(
    'RepeatedTypesError<fruit::Annotated<Annotation,X>,fruit::Annotated<Annotation,X>>',
    'A type was specified more than once.',
    COMMON_DEFINITIONS + '''
struct X {};

void f() {
    (void) sizeof(fruit::NormalizedComponent<XAnnot, XAnnot>);
}
''')

def test_error_repeated_type_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

void f() {
  (void) sizeof(fruit::NormalizedComponent<XAnnot1, XAnnot2>);
}
''')

def test_error_type_required_and_provided():
    expect_compile_error(
    'RepeatedTypesError<X,X>',
    'A type was specified more than once.',
    COMMON_DEFINITIONS + '''
struct X {};

void f() {
    (void) sizeof(fruit::NormalizedComponent<fruit::Required<X>, X>);
}
''')

def test_error_type_required_and_provided_with_annotation():
    expect_compile_error(
    'RepeatedTypesError<fruit::Annotated<Annotation,X>,fruit::Annotated<Annotation,X>>',
    'A type was specified more than once.',
    COMMON_DEFINITIONS + '''
struct X {};

void f() {
    (void) sizeof(fruit::NormalizedComponent<fruit::Required<XAnnot>, XAnnot>);
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
