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

struct Annotation {};
using intAnnot = fruit::Annotated<Annotation, int>;
using XAnnot = fruit::Annotated<Annotation, X>;
'''

def test_success():
    expect_success(
    COMMON_DEFINITIONS + '''
fruit::Component<int> getComponentForInstance(int& n) {
  fruit::Component<> comp = fruit::createComponent()
    .bindInstance(n);
  return fruit::createComponent()
    .install(comp)
    .bindInstance(n);
}

int main() {
  int n = 5;
  fruit::Injector<int> injector(getComponentForInstance(n));
  if (injector.get<int*>() != &n)
    abort();
  return 0;
}
''')

def test_success_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
fruit::Component<intAnnot> getComponentForInstance(int& n) {
  fruit::Component<> comp = fruit::createComponent()
    .bindInstance<intAnnot>(n);
  return fruit::createComponent()
    .install(comp)
    .bindInstance<intAnnot>(n);
}

int main() {
  int n = 5;
  fruit::Injector<intAnnot> injector(getComponentForInstance(n));
  if (injector.get<fruit::Annotated<Annotation, int*>>() != &n)
    abort();
  return 0;
}
''')

def test_abstract_class_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  virtual void foo() = 0;
};

fruit::Component<XAnnot> getComponentForInstance(X& x) {
  fruit::Component<> comp = fruit::createComponent()
    .bindInstance<XAnnot>(x);
  return fruit::createComponent()
    .install(comp)
    .bindInstance<XAnnot>(x);
}

int main() {
  return 0;
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
