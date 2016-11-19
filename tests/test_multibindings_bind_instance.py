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
'''

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_simple(XAnnot):
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

X x;

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .addInstanceMultibinding<XAnnot, X>(x);
}

int main() {
  fruit::Injector<> injector(getComponent());

  std::vector<X*> multibindings = injector.getMultibindings<XAnnot>();
  Assert(multibindings.size() == 1);
  Assert(multibindings[0] == &x);
}
''',
    locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_instance_vector_with_annotation(XAnnot):
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

std::vector<X> values = {X(), X()};

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .addInstanceMultibindings<XAnnot, X>(values);
}

int main() {
  fruit::Injector<> injector(getComponent());

  std::vector<X*> multibindings = injector.getMultibindings<XAnnot>();
  Assert(multibindings.size() == 2);
  Assert(multibindings[0] == &(values[0]));
  Assert(multibindings[1] == &(values[1]));
}
''',
    locals())

if __name__ == '__main__':
    import nose2
    nose2.main()
