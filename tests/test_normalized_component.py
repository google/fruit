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

def test_success():
    expect_success(
    '''
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

  fruit::Injector<X> injector(normalizedComponent, fruit::Component<X>(fruit::createComponent().bindInstance(x)));
  injector.get<X*>();

  fruit::Injector<Y> injector2(normalizedComponent, getXComponent(x));
  injector2.get<Y*>();

  fruit::Injector<Y> injector3(normalizedComponent, getXComponent(x));
  injector3.get<Y*>();

  fruit::Injector<X> injector4(normalizedComponent, fruit::Component<X>(fruit::createComponent().bindInstance(x)));
  injector4.get<X*>();

  return 0;
}
''')

def test_success_with_annotations():
    expect_success(
    '''
struct Annotation {};

struct X {
};

using XAnnot = fruit::Annotated<Annotation, X>;

struct Y {
  INJECT(Y(ANNOTATED(Annotation, X))) {};
};

fruit::Component<fruit::Required<XAnnot>, Y> getComponent() {
  return fruit::createComponent();
}

fruit::Component<XAnnot> getXComponent(X& x) {
  return fruit::createComponent()
    .bindInstance<XAnnot>(x);
}

int main() {
  fruit::NormalizedComponent<fruit::Required<XAnnot>, Y> normalizedComponent(getComponent());

  X x{};

  fruit::Injector<XAnnot> injector(normalizedComponent, fruit::Component<XAnnot>(fruit::createComponent().bindInstance<XAnnot>(x)));
  injector.get<fruit::Annotated<Annotation, X*>>();

  fruit::Injector<Y> injector2(normalizedComponent, getXComponent(x));
  injector2.get<Y*>();

  fruit::Injector<Y> injector3(normalizedComponent, getXComponent(x));
  injector3.get<Y*>();

  fruit::Injector<XAnnot> injector4(normalizedComponent, fruit::Component<XAnnot>(fruit::createComponent().bindInstance<XAnnot>(x)));
  injector4.get<fruit::Annotated<Annotation, X*>>();

  return 0;
}
''')

def test_unsatisfied_requirements():
    expect_compile_error(
    'UnsatisfiedRequirementsInNormalizedComponentError<X>',
    'The requirements in UnsatisfiedRequirements are required by the NormalizedComponent but are not provided by the Component',
    '''
struct X {
  INJECT(X()) = default;
};

fruit::Component<fruit::Required<X>> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<fruit::Required<X>> normalizedComponent(getComponent());
  fruit::Injector<> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));

  return 0;
}
''')

def test_unsatisfied_requirements_with_annotation():
    expect_compile_error(
    'UnsatisfiedRequirementsInNormalizedComponentError<fruit::Annotated<Annotation,X>>',
    'The requirements in UnsatisfiedRequirements are required by the NormalizedComponent but are not provided by the Component',
    '''
struct Annotation {};

struct X {
  X() = default;
};

fruit::Component<fruit::Required<fruit::Annotated<Annotation, X>>> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<fruit::Required<fruit::Annotated<Annotation, X>>> normalizedComponent(getComponent());
  fruit::Injector<> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));

  return 0;
}
''')

def test_error_repeated_type():
    expect_compile_error(
    'RepeatedTypesError<X,X>',
    'A type was specified more than once.',
    '''
struct X {};

void f() {
    (void) sizeof(fruit::NormalizedComponent<X, X>);
}
''')

def test_error_repeated_type_with_annotation():
    expect_compile_error(
    'RepeatedTypesError<fruit::Annotated<Annotation,X>,fruit::Annotated<Annotation,X>>',
    'A type was specified more than once.',
    '''
struct Annotation {};

struct X {
};

using XAnnot = fruit::Annotated<Annotation, X>;

void f() {
    (void) sizeof(fruit::NormalizedComponent<XAnnot, XAnnot>);
}
''')

def test_error_repeated_type_with_different_annotation_ok():
    expect_success(
    '''
struct Annotation1 {};
struct Annotation2 {};

struct X {};

using XAnnot1 = fruit::Annotated<Annotation1, X>;
using XAnnot2 = fruit::Annotated<Annotation2, X>;

int main() {
  (void) sizeof(fruit::NormalizedComponent<XAnnot1, XAnnot2>);
  return 0;
}
''')

def test_error_type_required_and_provided():
    expect_compile_error(
    'RepeatedTypesError<X,X>',
    'A type was specified more than once.',
    '''
struct X {};

fruit::NormalizedComponent<fruit::Required<X>, X> nc;
''')

def test_error_type_required_and_provided_with_annotation():
    expect_compile_error(
    'RepeatedTypesError<fruit::Annotated<Annotation,X>,fruit::Annotated<Annotation,X>>',
    'A type was specified more than once.',
    '''
struct Annotation {};

struct X {};

using XAnnot = fruit::Annotated<Annotation, X>;

void f() {
    (void) sizeof(fruit::NormalizedComponent<fruit::Required<XAnnot>, XAnnot>);
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
