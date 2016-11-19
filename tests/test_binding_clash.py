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
#include <fruit/fruit.h>
#include <vector>
#include "test_macros.h"

struct Annotation1 {};
struct Annotation2 {};
'''

@params('int', 'fruit::Annotated<Annotation1, int>')
def test_binding_and_binding(intAnnot):
    expect_compile_error(
    'TypeAlreadyBoundError<intAnnot>',
    'Trying to bind C but it is already bound.',
    COMMON_DEFINITIONS + '''
fruit::Component<intAnnot> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>()
    .registerConstructor<intAnnot()>();
}
''',
    locals())

def test_binding_and_binding_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
using intAnnot1 = fruit::Annotated<Annotation1, int>;
using intAnnot2 = fruit::Annotated<Annotation2, int>;

fruit::Component<intAnnot1, intAnnot2> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot1()>()
    .registerConstructor<intAnnot2()>();
}
''')

@params('int', 'fruit::Annotated<Annotation1, int>')
def test_binding_and_install(intAnnot):
    expect_compile_error(
    'DuplicateTypesInComponentError<intAnnot>',
    'The installed component provides some types that are already provided by the current component.',
    COMMON_DEFINITIONS + '''
fruit::Component<intAnnot> getParentComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>();
}

fruit::Component<intAnnot> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>()
    .install(getParentComponent());
}
''',
    locals())

def test_binding_and_install_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
using intAnnot1 = fruit::Annotated<Annotation1, int>;
using intAnnot2 = fruit::Annotated<Annotation2, int>;

fruit::Component<intAnnot1> getParentComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot1()>();
}

fruit::Component<intAnnot1, intAnnot2> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot2()>()
    .install(getParentComponent());
}

int main() {
  fruit::Injector<intAnnot1, intAnnot2> injector(getComponent());
  int& n1 = injector.get<fruit::Annotated<Annotation1, int&>>();
  int& n2 = injector.get<fruit::Annotated<Annotation2, int&>>();
  Assert(&n1 != &n2);
}
''')

@params('int', 'fruit::Annotated<Annotation1, int>')
def test_install_and_install_with_annotation(intAnnot):
    expect_compile_error(
    'DuplicateTypesInComponentError<intAnnot>',
    'The installed component provides some types that are already provided',
    COMMON_DEFINITIONS + '''
fruit::Component<intAnnot> getParentComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>();
}

fruit::Component<intAnnot> getComponent() {
  return fruit::createComponent()
    .install(getParentComponent())
    .install(getParentComponent());
}
''',
    locals())

def test_install_and_install_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
using intAnnot1 = fruit::Annotated<Annotation1, int>;
using intAnnot2 = fruit::Annotated<Annotation2, int>;

fruit::Component<intAnnot1> getParentComponent1() {
  return fruit::createComponent()
    .registerConstructor<intAnnot1()>();
}

fruit::Component<intAnnot2> getParentComponent2() {
  return fruit::createComponent()
    .registerConstructor<intAnnot2()>();
}

fruit::Component<intAnnot1, intAnnot2> getComponent() {
  return fruit::createComponent()
    .install(getParentComponent1())
    .install(getParentComponent2());
}

int main() {
  fruit::Injector<intAnnot1, intAnnot2> injector(getComponent());
  injector.get<intAnnot1>();
  injector.get<intAnnot2>();
}
''')

def test_binding_and_interface_binding():
    expect_compile_error(
    'TypeAlreadyBoundError<X>',
    'Trying to bind C but it is already bound.',
    COMMON_DEFINITIONS + '''
struct X {};

struct Y : public X {};

fruit::Component<Y> getComponent() {
  return fruit::createComponent()
    .registerConstructor<X()>()
    .registerConstructor<Y()>()
    .bind<X, Y>();
}
''')

def test_interface_binding_and_binding():
    expect_compile_error(
    'TypeAlreadyBoundError<X>',
    'Trying to bind C but it is already bound.',
    COMMON_DEFINITIONS + '''
struct X {};

struct Y : public X {};

fruit::Component<Y> getComponent() {
  return fruit::createComponent()
    .registerConstructor<Y()>()
    .bind<X, Y>()
    .registerConstructor<X()>();
}
''')

@params('int', 'fruit::Annotated<Annotation1, int>')
def test_during_component_merge(intAnnot):
    expect_compile_error(
    'DuplicateTypesInComponentError<intAnnot>',
    'The installed component provides some types that are already provided',
    COMMON_DEFINITIONS + '''
fruit::Component<intAnnot> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>();
}

void f() {
  fruit::NormalizedComponent<intAnnot> nc(getComponent());
  fruit::Injector<intAnnot> injector(nc, getComponent());
  (void) injector;
}
''',
    locals())

def test_during_component_merge_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
using intAnnot1 = fruit::Annotated<Annotation1, int>;
using intAnnot2 = fruit::Annotated<Annotation2, int>;

fruit::Component<intAnnot1> getComponent1() {
  return fruit::createComponent()
    .registerConstructor<intAnnot1()>();
}

fruit::Component<intAnnot2> getComponent2() {
  return fruit::createComponent()
    .registerConstructor<intAnnot2()>();
}

int main() {
  fruit::NormalizedComponent<intAnnot1> nc(getComponent1());
  fruit::Injector<intAnnot1, intAnnot2> injector(nc, getComponent2());
  injector.get<intAnnot1>();
  injector.get<intAnnot2>();
}
''')

@params('int', 'fruit::Annotated<Annotation1, int>')
def test_bind_instance_and_bind_instance_runtime(intAnnot):
    expect_runtime_error(
    'Fatal injection error: the type intAnnot was provided more than once, with different bindings.',
    COMMON_DEFINITIONS + '''
fruit::Component<intAnnot> getComponentForInstance() {
  // Note: don't do this in real code, leaks memory.
  fruit::Component<> comp = fruit::createComponent()
    .bindInstance<intAnnot, int>(*(new int(5)));
  return fruit::createComponent()
    .install(comp)
    .bindInstance<intAnnot, int>(*(new int(5)));
}

int main() {
  fruit::Injector<intAnnot> injector(getComponentForInstance());
  injector.get<intAnnot>();
}
''',
    locals())

@params('int', 'fruit::Annotated<Annotation1, int>')
def test_bind_instance_and_binding_runtime(intAnnot):
    expect_runtime_error(
    'Fatal injection error: the type intAnnot was provided more than once, with different bindings.',
    COMMON_DEFINITIONS + '''
fruit::Component<intAnnot> getComponentForInstance(int& n) {
  fruit::Component<> comp = fruit::createComponent()
    .bindInstance<intAnnot, int>(n);
  return fruit::createComponent()
    .install(comp)
    .registerConstructor<intAnnot()>();
}

int main() {
  int n = 5;
  fruit::Injector<intAnnot> injector(getComponentForInstance(n));
  injector.get<intAnnot>();
}
''',
    locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_during_component_merge_consistent_ok(XAnnot):
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
  X() {
    Assert(!constructed);
    constructed = true;
  }

  static bool constructed;
};

bool X::constructed = false;

fruit::Component<XAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<> normalizedComponent(getComponent());
  fruit::Injector<XAnnot> injector(normalizedComponent, getComponent());

  Assert(!X::constructed);
  injector.get<XAnnot>();
  Assert(X::constructed);
}
''',
    locals())

if __name__ == '__main__':
    import nose2
    nose2.main()
