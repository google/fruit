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

def test_success_returning_value_implicit_signature():
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

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .addMultibindingProvider([](){return X();});
}

int main() {
  fruit::Injector<> injector(getComponent());

  Assert(!X::constructed);
  Assert(injector.getMultibindings<X>().size() == 1);
  Assert(X::constructed);
}
''',
    locals())

def test_success_returning_pointer_implicit_signature():
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

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .addMultibindingProvider([](){return new X();});
}

int main() {
  fruit::Injector<> injector(getComponent());

  Assert(!X::constructed);
  Assert(injector.getMultibindings<X>().size() == 1);
  Assert(X::constructed);
}
''',
    locals())


@params('X', 'fruit::Annotated<Annotation1, X>')
def test_success_returning_value(XAnnot):
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

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .addMultibindingProvider<XAnnot()>([](){return X();});
}

int main() {
  fruit::Injector<> injector(getComponent());

  Assert(!X::constructed);
  Assert(injector.getMultibindings<XAnnot>().size() == 1);
  Assert(X::constructed);
}
''',
    locals())

@params(
    ('X', 'X*'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>'))
def test_success_returning_pointer(XAnnot, XPtrAnnot):
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

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .addMultibindingProvider<XPtrAnnot()>([](){return new X();});
}

int main() {
  fruit::Injector<> injector(getComponent());

  Assert(!X::constructed);
  Assert(injector.getMultibindings<XAnnot>().size() == 1);
  Assert(X::constructed);
}
''',
    locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_success_returning_value_with_normalized_component(XAnnot):
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = XAnnot();
  X() {
    Assert(!constructed);
    constructed = true;
  }

  static bool constructed;
};

bool X::constructed = false;

fruit::Component<> getComponent() {
  return fruit::createComponent()
      .addMultibindingProvider<XAnnot()>([](){return X();});
}

int main() {
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  fruit::Injector<> injector(normalizedComponent, getComponent());

  Assert(!X::constructed);
  const std::vector<X*>& bindings = injector.getMultibindings<XAnnot>();
  Assert(bindings.size() == 1);
  Assert(X::constructed);
}
''',
    locals())

@params(
    ('X', 'X*', 'int'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>', 'fruit::Annotated<Annotation2, int>'))
def test_multiple_providers(XAnnot, XPtrAnnot, intAnnot):
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .registerProvider<intAnnot()>([](){return 42;})
    .addMultibindingProvider<XAnnot(intAnnot)>([](int){return X();})
    .addMultibindingProvider<XPtrAnnot(intAnnot)>([](int){return new X();});
}

int main() {
  fruit::Injector<> injector(getComponent());

  std::vector<X*> multibindings = injector.getMultibindings<XAnnot>();
  Assert(multibindings.size() == 2);
}
''',
    locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_returning_value_malformed_signature(XAnnot):
    expect_compile_error(
    'NotASignatureError<XAnnot>',
    'CandidateSignature was specified as parameter, but it.s not a signature.',
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .addMultibindingProvider<XAnnot>([](){return X();});
}
''',
    locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_not_function(XAnnot):
    expect_compile_error(
    'FunctorUsedAsProviderError<.*>',
    'A stateful lambda or a non-lambda functor was used as provider',
    COMMON_DEFINITIONS + '''
struct X {
  X(int) {}
};

fruit::Component<> getComponent() {
  int n = 3;
  return fruit::createComponent()
    .addMultibindingProvider<XAnnot()>([=]{return X(n);});
}
''',
    locals())

def test_lambda_with_captures_error():
    expect_compile_error(
    'FunctorUsedAsProviderError<.*>',
    'A stateful lambda or a non-lambda functor was used as provider',
    COMMON_DEFINITIONS + '''
struct X {
  X(int) {}
};

fruit::Component<> getComponent() {
  int n = 3;
  return fruit::createComponent()
    .addMultibindingProvider([=]{return X(n);});
}
''')

# TODO: should XPtrAnnot be just XAnnot in the signature?
# Make sure the behavior here is consistent with registerProvider() and registerFactory().
@params(
    ('X', 'X*'),
    ('fruit::Annotated<Annotation1, X>', 'fruit::Annotated<Annotation1, X*>'))
def test_provider_returns_nullptr_error(XAnnot, XPtrAnnot):
    expect_runtime_error(
    'Fatal injection error: attempting to get an instance for the type XAnnot but the provider returned nullptr',
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<> getComponent() {
  return fruit::createComponent()
      .addMultibindingProvider<XPtrAnnot()>([](){return (X*)nullptr;});
}

int main() {
  fruit::Injector<> injector(getComponent());
  injector.getMultibindings<XAnnot>();
}
''',
    locals())

if __name__ == '__main__':
    import nose2
    nose2.main()
