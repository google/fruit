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

struct Annotation {};
using intAnnot = fruit::Annotated<Annotation, int>;
using XAnnot = fruit::Annotated<Annotation, X>;

struct Annotation1 {};
using intAnnot1 = fruit::Annotated<Annotation1, int>;

struct Annotation2 {};
using intAnnot2 = fruit::Annotated<Annotation2, int>;
'''

def test_binding_and_binding():
    expect_compile_error(
    'TypeAlreadyBoundError<int>',
    'Trying to bind C but it is already bound.',
    COMMON_DEFINITIONS + '''
Component<int> getComponent() {
  return fruit::createComponent()
    .registerConstructor<int()>()
    .registerConstructor<int()>();
}
''')

def test_binding_and_binding_with_annotation():
    expect_compile_error(
    'TypeAlreadyBoundError<fruit::Annotated<Annotation,int>>',
    'Trying to bind C but it is already bound.',
    COMMON_DEFINITIONS + '''
Component<intAnnot> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>()
    .registerConstructor<intAnnot()>();
}
''')

def test_binding_and_binding_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
Component<intAnnot1, intAnnot2> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot1()>()
    .registerConstructor<intAnnot2()>();
}

int main() {
  return 0;
}
''')

def test_binding_and_install():
    expect_compile_error(
    'DuplicateTypesInComponentError<int>',
    'The installed component provides some types that are already provided by the current component.',
    COMMON_DEFINITIONS + '''
Component<int> getParentComponent() {
  return fruit::createComponent()
    .registerConstructor<int()>();
}

Component<int> getComponent() {
  return fruit::createComponent()
    .registerConstructor<int()>()
    .install(getParentComponent());
}
''')

def test_binding_and_install_with_annotation():
    expect_compile_error(
    'DuplicateTypesInComponentError<fruit::Annotated<Annotation,int>>',
    'The installed component provides some types that are already provided by the current component.',
    COMMON_DEFINITIONS + '''
Component<intAnnot> getParentComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>();
}

Component<intAnnot> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>()
    .install(getParentComponent());
}
''')

def test_binding_and_install_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
Component<intAnnot1> getParentComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot1()>();
}

Component<intAnnot1, intAnnot2> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot2()>()
    .install(getParentComponent());
}

int main() {
  fruit::Injector<intAnnot1, intAnnot2> injector(getComponent());
  int& n1 = injector.get<fruit::Annotated<Annotation1, int&>>();
  int& n2 = injector.get<fruit::Annotated<Annotation2, int&>>();
  Assert(&n1 != &n2);
  return 0;
}
''')

def test_type_already_bound_install_and_install():
    expect_compile_error(
    'DuplicateTypesInComponentError<int>',
    'The installed component provides some types that are already provided',
    COMMON_DEFINITIONS + '''
Component<int> getParentComponent() {
  return fruit::createComponent()
    .registerConstructor<int()>();
}

Component<int> getComponent() {
  return fruit::createComponent()
    .install(getParentComponent())
    .install(getParentComponent());
}
''')

def test_install_and_install_with_annotation():
    expect_compile_error(
    'DuplicateTypesInComponentError<fruit::Annotated<Annotation,int>>',
    'The installed component provides some types that are already provided',
    COMMON_DEFINITIONS + '''
Component<intAnnot> getParentComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>();
}

Component<intAnnot> getComponent() {
  return fruit::createComponent()
    .install(getParentComponent())
    .install(getParentComponent());
}
''')

def test_install_and_install_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
Component<intAnnot1> getParentComponent1() {
  return fruit::createComponent()
    .registerConstructor<intAnnot1()>();
}

Component<intAnnot2> getParentComponent2() {
  return fruit::createComponent()
    .registerConstructor<intAnnot2()>();
}

Component<intAnnot1, intAnnot2> getComponent() {
  return fruit::createComponent()
    .install(getParentComponent1())
    .install(getParentComponent2());
}

int main() {
  fruit::Injector<intAnnot1, intAnnot2> injector(getComponent());
  injector.get<intAnnot1>();
  injector.get<intAnnot2>();
  return 0;
}
''')

def test_binding_and_interface_binding():
    expect_compile_error(
    'TypeAlreadyBoundError<X>',
    'Trying to bind C but it is already bound.',
    COMMON_DEFINITIONS + '''
struct X {};

struct Y : public X {};

Component<Y> getComponent() {
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

Component<Y> getComponent() {
  return fruit::createComponent()
    .registerConstructor<Y()>()
    .bind<X, Y>()
    .registerConstructor<X()>();
}
''')

def test_during_component_merge():
    expect_compile_error(
    'DuplicateTypesInComponentError<int>',
    'The installed component provides some types that are already provided',
    COMMON_DEFINITIONS + '''
Component<int> getComponent() {
  return fruit::createComponent()
    .registerConstructor<int()>();
}

void f() {
  fruit::NormalizedComponent<int> nc(getComponent());
  fruit::Injector<int> injector(nc, getComponent());
  (void) injector;
}
''')

def test_during_component_merge_with_annotation():
    expect_compile_error(
    'DuplicateTypesInComponentError<fruit::Annotated<Annotation,int>>',
    'The installed component provides some types that are already provided',
    COMMON_DEFINITIONS + '''
Component<intAnnot> getComponent() {
  return fruit::createComponent()
    .registerConstructor<intAnnot()>();
}

void f() {
  fruit::NormalizedComponent<intAnnot> nc(getComponent());
  fruit::Injector<intAnnot> injector(nc, getComponent());
  (void) injector;
}
''')

def test_during_component_merge_with_different_annotation_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
Component<intAnnot1> getComponent1() {
  return fruit::createComponent()
    .registerConstructor<intAnnot1()>();
}

Component<intAnnot2> getComponent2() {
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

def test_bind_instance_and_bind_instance_runtime():
    expect_runtime_error(
    'Fatal injection error: the type int was provided more than once, with different bindings.',
    COMMON_DEFINITIONS + '''
Component<int> getComponentForInstance() {
  // Note: don't do this in real code, leaks memory.
  Component<> comp = fruit::createComponent()
    .bindInstance(*(new int(5)));
  return fruit::createComponent()
    .install(comp)
    .bindInstance(*(new int(5)));
}

int main() {
  Injector<int> injector(getComponentForInstance());
  injector.get<int*>();
  return 0;
}
'''
)

def test_bind_instance_and_bind_instance_annotated_runtime():
    expect_runtime_error(
    'Fatal injection error: the type fruit::Annotated<Annotation, int> was provided more than once, with different bindings.',
    COMMON_DEFINITIONS + '''
Component<intAnnot> getComponentForInstance() {
  // Note: don't do this in real code, leaks memory.
  Component<> comp = fruit::createComponent()
    .bindInstance<intAnnot>(*(new int(5)));
  return fruit::createComponent()
    .install(comp)
    .bindInstance<intAnnot>(*(new int(5)));
}

int main() {
  Injector<intAnnot> injector(getComponentForInstance());
  injector.get<intAnnot>();
  return 0;
}
'''
)

def test_bind_instance_and_binding_runtime():
    expect_runtime_error(
    'Fatal injection error: the type int was provided more than once, with different bindings.',
    COMMON_DEFINITIONS + '''
Component<int> getComponentForInstance(int& n) {
  Component<> comp = fruit::createComponent()
    .bindInstance(n);
  return fruit::createComponent()
    .install(comp)
    .registerConstructor<int()>();
}

int main() {
  int n = 5;
  Injector<int> injector(getComponentForInstance(n));
  if (injector.get<int*>() != &n)
    abort();
  return 0;
}
''')

def test_bind_instance_and_binding_annotated_runtime():
    expect_runtime_error(
    'Fatal injection error: the type fruit::Annotated<Annotation, ?int> was provided more than once, with different bindings.',
    COMMON_DEFINITIONS + '''
Component<intAnnot> getComponentForInstance(int& n) {
  Component<> comp = fruit::createComponent()
    .bindInstance<intAnnot>(n);
  return fruit::createComponent()
    .install(comp)
    .registerConstructor<intAnnot()>();
}

int main() {
  int n = 5;
  Injector<intAnnot> injector(getComponentForInstance(n));
  injector.get<intAnnot>();
  return 0;
}
''')

def test_during_component_merge_consistent_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) {
    Assert(!constructed);
    constructed = true;
  }

  static bool constructed;
};

bool X::constructed = false;

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {

  fruit::NormalizedComponent<> normalizedComponent(getComponent());
  Injector<X> injector(normalizedComponent, getComponent());

  Assert(!X::constructed);
  injector.get<X>();
  Assert(X::constructed);

  return 0;
}
''')

def test_during_component_merge_consistent_with_annotation_ok():
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
  Injector<XAnnot> injector(normalizedComponent, getComponent());

  Assert(!X::constructed);
  injector.get<XAnnot>();
  Assert(X::constructed);

  return 0;
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
