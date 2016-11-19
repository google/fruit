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
struct Z;

struct Annotation1 {};
using XAnnot1 = fruit::Annotated<Annotation1, X>;
using YAnnot1 = fruit::Annotated<Annotation1, Y>;
using ZAnnot1 = fruit::Annotated<Annotation1, Z>;

struct Annotation2 {};
using XAnnot2 = fruit::Annotated<Annotation2, X>;
using YAnnot2 = fruit::Annotated<Annotation2, Y>;
using ZAnnot2 = fruit::Annotated<Annotation2, Z>;
'''

def test_success():
    expect_success(
    COMMON_DEFINITIONS + '''
struct Y {
  INJECT(Y()) = default;
};

struct X {
  INJECT(X(Y)) {
  }
};

struct Z {
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<> injector(getComponent());
  X* x = injector.unsafeGet<X>();
  Y* y = injector.unsafeGet<Y>();
  Z* z = injector.unsafeGet<Z>();

  (void) x;
  (void) y;
  (void) z;
  Assert(x != nullptr);
  Assert(y != nullptr);
  Assert(z == nullptr);
}
''')

def test_with_annotation_success():
    expect_success(
    COMMON_DEFINITIONS + '''
struct Y {
  using Inject = Y();
  Y() = default;
};

struct X {
  using Inject = X(YAnnot2);
  X(Y) {
  }
};

struct Z {};

fruit::Component<XAnnot1> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<> injector(getComponent());
  X* x = injector.unsafeGet<XAnnot1>();
  Y* y = injector.unsafeGet<YAnnot2>();
  Z* z = injector.unsafeGet<ZAnnot2>();

  (void) x;
  (void) y;
  (void) z;
  Assert(x != nullptr);
  Assert(y != nullptr);
  Assert(z == nullptr);
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
