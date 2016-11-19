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

struct Annotation1 {};
using XAnnot1 = fruit::Annotated<Annotation1, X>;

struct Annotation2 {};
using XAnnot2 = fruit::Annotated<Annotation2, X>;
'''

def test_simple():
    expect_compile_error(
    'SelfLoopError<MutuallyConstructible1,MutuallyConstructible2>',
    'Found a loop in the dependencies',
    COMMON_DEFINITIONS + '''
struct MutuallyConstructible2;

struct MutuallyConstructible1 {
  INJECT(MutuallyConstructible1(const MutuallyConstructible2&)) {};
};

struct MutuallyConstructible2 {
  INJECT(MutuallyConstructible2(const MutuallyConstructible1&)) {};
};

fruit::Component<MutuallyConstructible1> mutuallyConstructibleComponent() {
  return fruit::createComponent();
}
''')

def test_with_annotations():
    expect_compile_error(
    'SelfLoopError<fruit::Annotated<Annotation1,X>,fruit::Annotated<Annotation2,X>>',
    'Found a loop in the dependencies',
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<XAnnot1> mutuallyConstructibleComponent() {
  return fruit::createComponent()
      .registerProvider<XAnnot1(XAnnot2)>([](X x) {return x;})
      .registerProvider<XAnnot2(XAnnot1)>([](X x) {return x;});
}
''')

def test_with_different_annotations_ok():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};
struct Y {};

fruit::Component<XAnnot2> getComponent() {
  return fruit::createComponent()
      .registerProvider<XAnnot1()>([](){return X();})
      .registerProvider<Y(XAnnot1)>([](X){return Y();})
      .registerProvider<XAnnot2(Y)>([](Y){return X();});
}

int main() {
  fruit::Injector<XAnnot2> injector(getComponent());
  injector.get<XAnnot2>();
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
