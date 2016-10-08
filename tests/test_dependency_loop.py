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

def test_simple():
    expect_compile_error(
    'SelfLoopError<MutuallyConstructible1,MutuallyConstructible2>',
    'Found a loop in the dependencies',
    '''
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
    '''
struct Annotation1 {};
struct Annotation2 {};

struct X {
};

using XAnnot1 = fruit::Annotated<Annotation1, X>;
using XAnnot2 = fruit::Annotated<Annotation2, X>;

fruit::Component<XAnnot1> mutuallyConstructibleComponent() {
  return fruit::createComponent()
      .registerProvider<XAnnot1(XAnnot2)>([](X x) {return x;})
      .registerProvider<XAnnot2(XAnnot1)>([](X x) {return x;});
}
''')

def test_with_different_annotations_ok():
    expect_success(
    '''
struct Annotation1 {};
struct Annotation2 {};

struct X {
};

struct Y {
};

fruit::Component<fruit::Annotated<Annotation2, X>> getComponent() {
  return fruit::createComponent()
      .registerProvider<fruit::Annotated<Annotation1, X>()>([](){return X();})
      .registerProvider<Y(fruit::Annotated<Annotation1, X>)>([](X){return Y();})
      .registerProvider<fruit::Annotated<Annotation2, X>(Y)>([](Y){return X();});
}

int main() {

  fruit::Injector<fruit::Annotated<Annotation2, X>> injector(getComponent());
  injector.get<fruit::Annotated<Annotation2, X>>();

  return 0;
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
