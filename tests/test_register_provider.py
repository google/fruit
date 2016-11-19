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
using XAnnot = fruit::Annotated<Annotation, X>;
using intAnnot = fruit::Annotated<Annotation, int>;
'''

def test_success_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() {
    ++num_constructions;
  }

  static unsigned num_constructions;

  int value = 5;
};

unsigned X::num_constructions = 0;

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerProvider([](){return X();});
}

int main() {
  fruit::Injector<X> injector(getComponent());
  Assert(injector.get<X>().value == 5);
  Assert(injector.get<X*>()->value == 5);
  Assert(injector.get<X&>().value == 5);
  Assert(injector.get<const X>().value == 5);
  Assert(injector.get<const X*>()->value == 5);
  Assert(injector.get<const X&>().value == 5);
  Assert(injector.get<std::shared_ptr<X>>()->value == 5);

  Assert(X::num_constructions == 1);
}
''')

def test_success_returning_pointer():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() {
    ++num_constructions;
  }

  static unsigned num_constructions;

  int value = 5;
};

unsigned X::num_constructions = 0;

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerProvider([](){return new X();});
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X*>();

  Assert(injector.get<X>().value == 5);
  Assert(injector.get<X*>()->value == 5);
  Assert(injector.get<X&>().value == 5);
  Assert(injector.get<const X>().value == 5);
  Assert(injector.get<const X*>()->value == 5);
  Assert(injector.get<const X&>().value == 5);
  Assert(injector.get<std::shared_ptr<X>>()->value == 5);

  Assert(X::num_constructions == 1);
}
''')

def test_success_with_annotation_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() {
    ++num_constructions;
  }

  static unsigned num_constructions;

  int value = 5;
};

unsigned X::num_constructions = 0;

fruit::Component<XAnnot> getComponent() {
  return fruit::createComponent()
    .registerProvider<XAnnot()>([](){return X();});
}

int main() {
  fruit::Injector<XAnnot> injector(getComponent());

  Assert((injector.get<fruit::Annotated<Annotation, X                 >>(). value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, X*                >>()->value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, X&                >>(). value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, const X           >>(). value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, const X*          >>()->value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, const X&          >>(). value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, std::shared_ptr<X>>>()->value == 5));

  Assert(X::num_constructions == 1);
}
''')

def test_success_with_annotation_returning_pointer():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() {
    ++num_constructions;
  }

  static unsigned num_constructions;

  int value = 5;
};

unsigned X::num_constructions = 0;

fruit::Component<XAnnot> getComponent() {
  return fruit::createComponent()
    .registerProvider<fruit::Annotated<Annotation, X*>()>([](){return new X();});
}

int main() {
  fruit::Injector<XAnnot> injector(getComponent());
  Assert((injector.get<fruit::Annotated<Annotation, X                 >>(). value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, X*                >>()->value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, X&                >>(). value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, const X           >>(). value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, const X*          >>()->value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, const X&          >>(). value == 5));
  Assert((injector.get<fruit::Annotated<Annotation, std::shared_ptr<X>>>()->value == 5));

  Assert(X::num_constructions == 1);
}
''')

def test_error_not_function():
    expect_compile_error(
    'FunctorUsedAsProviderError<.*>',
    'A stateful lambda or a non-lambda functor was used as provider',
    COMMON_DEFINITIONS + '''
struct X {
  X(int) {}
};

fruit::Component<int> getComponent() {
  int n = 3;
  return fruit::createComponent()
    .registerProvider([=]{return X(n);});
}
''')

def test_error_not_function_with_annotation():
    expect_compile_error(
    'FunctorUsedAsProviderError<.*>',
    'A stateful lambda or a non-lambda functor was used as provider',
    COMMON_DEFINITIONS + '''
struct X {
  X(int) {}
};

fruit::Component<XAnnot> getComponent() {
  int n = 3;
  return fruit::createComponent()
    .registerProvider<XAnnot()>([=]{return X(n);});
}
''')

def test_error_malformed_signature():
    expect_compile_error(
    'NotASignatureError<fruit::Annotated<Annotation,int>>',
    'CandidateSignature was specified as parameter, but it.s not a signature. Signatures are of the form',
    COMMON_DEFINITIONS + '''
fruit::Component<int> getComponent() {
  return fruit::createComponent()
    .registerProvider<intAnnot>([](){return 42;});
}
''')

def test_error_returned_nullptr():
    expect_runtime_error(
    'Fatal injection error: attempting to get an instance for the type X but the provider returned nullptr',
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
      .registerProvider([](){return (X*)nullptr;});
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X>();
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
