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
''')

def test_success_returning_pointer():
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
''')

def test_with_annotation_returning_value_success():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<> getComponentWithProviderByValue() {
  return fruit::createComponent()
    .addMultibindingProvider<XAnnot()>([](){return X();});
}
''')

def test_with_annotation_returning_pointer_success():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

using XPtrAnnot = fruit::Annotated<Annotation, X*>;

fruit::Component<> getComponentWithPointerProvider() {
  return fruit::createComponent()
    .addMultibindingProvider<XPtrAnnot()>([](){return new X();});
}
''')

def test_success_returning_value_with_normalized_component():
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
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  fruit::Injector<> injector(normalizedComponent, getComponent());

  Assert(!X::constructed);
  injector.getMultibindings<X>();
  Assert(X::constructed);
}
''')

def test_success_returning_value_with_normalized_component_with_annotation():
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
''')

def test_multiple_providers():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .addMultibindingProvider([](){return X();})
    .addMultibindingProvider([](){return new X();});
}

int main() {
  fruit::Injector<> injector(getComponent());

  std::vector<X*> multibindings = injector.getMultibindings<X>();
  Assert(multibindings.size() == 2);
}
''')

def test_multiple_providers_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .registerProvider<intAnnot()>([](){return 42;})
    .addMultibindingProvider<XAnnot(intAnnot)>([](int){return X();})
    .addMultibindingProvider<fruit::Annotated<Annotation, X*>(intAnnot)>([](int){return new X();});
}

int main() {
  fruit::Injector<> injector(getComponent());

  std::vector<X*> multibindings = injector.getMultibindings<XAnnot>();
  Assert(multibindings.size() == 2);
}
''')

def test_returning_value_with_annotation_malformed_signature():
    expect_compile_error(
    'NotASignatureError<fruit::Annotated<Annotation,X>>',
    'CandidateSignature was specified as parameter, but it.s not a signature.',
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<> getComponent() {
  return fruit::createComponent()
    .addMultibindingProvider<XAnnot>([](){return X();});
}
''')

def test_not_function_with_annotation():
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
''')

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

def test_provider_returns_nullptr_error():
    expect_runtime_error(
    'Fatal injection error: attempting to get an instance for the type X but the provider returned nullptr',
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<> getComponent() {
  return fruit::createComponent()
      .addMultibindingProvider([](){return (X*)nullptr;});
}

int main() {
  fruit::Injector<> injector(getComponent());
  injector.getMultibindings<X>();
}
''')

def test_provider_returns_nullptr_error_with_annotation():
    expect_runtime_error(
    'Fatal injection error: attempting to get an instance for the type fruit::Annotated<Annotation, X> but the provider returned nullptr',
    COMMON_DEFINITIONS + '''
struct X {};

fruit::Component<> getComponent() {
  return fruit::createComponent()
      .addMultibindingProvider<fruit::Annotated<Annotation, X*>()>([](){return (X*)nullptr;});
}

int main() {
  fruit::Injector<> injector(getComponent());
  injector.getMultibindings<XAnnot>();
}
''')

def test_error_abstract_class():
    expect_compile_error(
    'CannotConstructAbstractClassError<X>',
    'The specified class can.t be constructed because it.s an abstract class.',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = XAnnot();
  X() {}

  virtual void foo() = 0;
};

fruit::Component<> getComponent() {
  return fruit::createComponent()
      .addMultibindingProvider<XAnnot()>([](){return (X*)(nullptr);});
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
