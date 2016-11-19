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
struct X1;
struct X2;
struct Y;
struct Y1;
struct Y2;
struct Z;
struct Z1;
struct Z2;

struct Annotation {};
using XAnnot = fruit::Annotated<Annotation, X>;
using X1Annot = fruit::Annotated<Annotation, X1>;
using X2Annot = fruit::Annotated<Annotation, X2>;
using YAnnot = fruit::Annotated<Annotation, Y>;
using Y1Annot = fruit::Annotated<Annotation, Y1>;
using Y2Annot = fruit::Annotated<Annotation, Y2>;
using ZAnnot = fruit::Annotated<Annotation, Z>;
using Z1Annot = fruit::Annotated<Annotation, Z1>;
using Z2Annot = fruit::Annotated<Annotation, Z2>;
'''

def test_success_copyable_and_movable():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  X(X&&) = default;
  X(const X&) = default;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X*>();
}
''')

# TODO: move to test_register_provider.py
def test_success_copyable_and_movable_provider_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = default;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerProvider([](){return X();});
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X*>();
}
''')

# TODO: move to test_register_provider.py
def test_success_copyable_and_movable_provider_returning_pointer():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = default;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerProvider([](){return new X();});
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X*>();
}
''')

def test_success_movable_only():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  X(X&&) = default;
  X(const X&) = delete;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X*>();
}
''')

# TODO: move to test_register_provider.py
def test_success_movable_only_provider_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = delete;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerProvider([](){return X();});
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X*>();
}
''')

# TODO: move to test_register_provider.py
def test_success_movable_only_provider_returning_pointer():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = delete;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerProvider([](){return new X();});
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X*>();
}
''')

def test_success_not_movable():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  X(X&&) = delete;
  X(const X&) = delete;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X*>();
}
''')

# TODO: move to test_register_provider.py
def test_success_not_movable_provider_returning_pointer():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = delete;
  X(const X&) = delete;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerProvider([](){return new X();});
}

int main() {
  fruit::Injector<X> injector(getComponent());
  injector.get<X*>();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_copyable_and_movable_implicit():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  X(X&&) = default;
  X(const X&) = default;
};

using XFactory = std::function<X()>;

fruit::Component<XFactory> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<XFactory> injector(getComponent());
  injector.get<XFactory>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_copyable_and_movable_explicit_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = default;
};

using XFactory = std::function<X()>;

fruit::Component<XFactory> getComponent() {
  return fruit::createComponent()
    .registerFactory<X()>([](){return X();});
}

int main() {
  fruit::Injector<XFactory> injector(getComponent());
  injector.get<XFactory>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_copyable_and_movable_explicit_returning_pointer():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = default;
};

using XPtrFactory = std::function<std::unique_ptr<X>()>;

fruit::Component<XPtrFactory> getComponent() {
  return fruit::createComponent()
    .registerFactory<std::unique_ptr<X>()>([](){return std::unique_ptr<X>();});
}

int main() {
  fruit::Injector<XPtrFactory> injector(getComponent());
  injector.get<XPtrFactory>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_movable_only_implicit():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  X(X&&) = default;
  X(const X&) = delete;
};

using XFactory = std::function<X()>;

fruit::Component<XFactory> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<XFactory> injector(getComponent());
  injector.get<XFactory>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_movable_only_explicit_returning_value():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = delete;
};

using XFactory = std::function<X()>;

fruit::Component<XFactory> getComponent() {
  return fruit::createComponent()
    .registerFactory<X()>([](){return X();});
}

int main() {
  fruit::Injector<XFactory> injector(getComponent());
  injector.get<XFactory>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_movable_only_explicit_returning_pointer():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = delete;
};

using XPtrFactory = std::function<std::unique_ptr<X>()>;

fruit::Component<XPtrFactory> getComponent() {
  return fruit::createComponent()
    .registerFactory<std::unique_ptr<X>()>([](){return std::unique_ptr<X>();});
}

int main() {
  fruit::Injector<XPtrFactory> injector(getComponent());
  injector.get<XPtrFactory>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_not_movable_implicit():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  X(X&&) = delete;
  X(const X&) = delete;
};

using XPtrFactory = std::function<std::unique_ptr<X>()>;

fruit::Component<XPtrFactory> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<XPtrFactory> injector(getComponent());
  injector.get<XPtrFactory>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_not_movable_explicit_returning_pointer():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = delete;
  X(const X&) = delete;
};

using XPtrFactory = std::function<std::unique_ptr<X>()>;

fruit::Component<XPtrFactory> getComponent() {
  return fruit::createComponent()
    .registerFactory<std::unique_ptr<X>()>([](){return std::unique_ptr<X>();});
}

int main() {
  fruit::Injector<XPtrFactory> injector(getComponent());
  injector.get<XPtrFactory>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_copyable_and_movable_implicit_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  X(X&&) = default;
  X(const X&) = default;
};

using XFactoryAnnot = fruit::Annotated<Annotation, std::function<X()>>;

fruit::Component<XFactoryAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<XFactoryAnnot> injector(getComponent());
  injector.get<XFactoryAnnot>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_copyable_and_movable_explicit_returning_value_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = default;
};

using XFactoryAnnot = fruit::Annotated<Annotation, std::function<X()>>;

fruit::Component<XFactoryAnnot> getComponent() {
  return fruit::createComponent()
    .registerFactory<XAnnot()>([](){return X();});
}

int main() {
  fruit::Injector<XFactoryAnnot> injector(getComponent());
  injector.get<XFactoryAnnot>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_copyable_and_movable_explicit_returning_pointer_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = default;
};

using XPtrFactoryAnnot = fruit::Annotated<Annotation, std::function<std::unique_ptr<X>()>>;

fruit::Component<XPtrFactoryAnnot> getComponent() {
  return fruit::createComponent()
    .registerFactory<fruit::Annotated<Annotation, std::unique_ptr<X>>()>([](){return std::unique_ptr<X>();});
}

int main() {
  fruit::Injector<XPtrFactoryAnnot> injector(getComponent());
  injector.get<XPtrFactoryAnnot>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_movable_only_implicit_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  X(X&&) = default;
  X(const X&) = delete;
};

using XFactoryAnnot = fruit::Annotated<Annotation, std::function<X()>>;

fruit::Component<XFactoryAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<XFactoryAnnot> injector(getComponent());
  injector.get<XFactoryAnnot>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_movable_only_explicit_returning_value_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = delete;
};

using XFactoryAnnot = fruit::Annotated<Annotation, std::function<X()>>;

fruit::Component<XFactoryAnnot> getComponent() {
  return fruit::createComponent()
    .registerFactory<fruit::Annotated<Annotation, X>()>([](){return X();});
}

int main() {
  fruit::Injector<XFactoryAnnot> injector(getComponent());
  injector.get<XFactoryAnnot>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_movable_only_explicit_returning_pointer_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = default;
  X(const X&) = delete;
};

using XPtrFactoryAnnot = fruit::Annotated<Annotation, std::function<std::unique_ptr<X>()>>;

fruit::Component<XPtrFactoryAnnot> getComponent() {
  return fruit::createComponent()
    .registerFactory<fruit::Annotated<Annotation, std::unique_ptr<X>>()>([](){return std::unique_ptr<X>();});
}

int main() {
  fruit::Injector<XPtrFactoryAnnot> injector(getComponent());
  injector.get<XPtrFactoryAnnot>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_not_movable_implicit_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
  X(X&&) = delete;
  X(const X&) = delete;
};

using XPtrFactoryAnnot = fruit::Annotated<Annotation, std::function<std::unique_ptr<X>()>>;

fruit::Component<XPtrFactoryAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::Injector<XPtrFactoryAnnot> injector(getComponent());
  injector.get<XPtrFactoryAnnot>()();
}
''')

# TODO: move to test_register_factory.py
def test_success_factory_not_movable_explicit_returning_pointer_with_annotation():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  X() = default;
  X(X&&) = delete;
  X(const X&) = delete;
};

using XPtrFactoryAnnot = fruit::Annotated<Annotation, std::function<std::unique_ptr<X>()>>;

fruit::Component<XPtrFactoryAnnot> getComponent() {
  return fruit::createComponent()
    .registerFactory<fruit::Annotated<Annotation, std::unique_ptr<X>>()>([](){return std::unique_ptr<X>();});
}

int main() {
  fruit::Injector<XPtrFactoryAnnot> injector(getComponent());
  injector.get<XPtrFactoryAnnot>()();
}
''')

# TODO: consider moving to test_normalized_component.py
def test_autoinject_success():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  INJECT(X()) = default;
};

struct Y {
  INJECT(Y()) {
    Assert(!constructed);
    constructed = true;
  }

  static bool constructed;
};

bool Y::constructed = false;

struct Z {
  INJECT(Z()) = default;
};

fruit::Component<Z, Y, X> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  fruit::Injector<Y> injector(normalizedComponent, getComponent());

  Assert(!Y::constructed);
  injector.get<Y>();
  Assert(Y::constructed);
}
''')

# TODO: consider moving to test_normalized_component.py
def test_autoinject_with_annotation_success():
    expect_success(
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
};

struct Y {
  using Inject = Y();
  Y() {
    Assert(!constructed);
    constructed = true;
  }

  static bool constructed;
};

bool Y::constructed = false;

struct Z {
  using Inject = Z();
};

fruit::Component<ZAnnot, YAnnot, XAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  fruit::Injector<YAnnot> injector(normalizedComponent, getComponent());

  Assert(!Y::constructed);
  injector.get<YAnnot>();
  Assert(Y::constructed);
}
''')

def test_autoinject_annotation_in_signature_return_type():
    expect_compile_error(
    'InjectTypedefWithAnnotationError<X>',
    'C::Inject is a signature that returns an annotated type',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = XAnnot();
};

fruit::Component<XAnnot> getComponent() {
  return fruit::createComponent();
}
''')

def test_autoinject_wrong_class_in_typedef():
    expect_compile_error(
    'InjectTypedefForWrongClassError<Y,X>',
    'C::Inject is a signature, but does not return a C. Maybe the class C has no Inject typedef and',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X();
};

struct Y : public X {
};

fruit::Component<Y> getComponent() {
  return fruit::createComponent();
}
''')

def test_error_abstract_class():
    expect_compile_error(
    'CannotConstructAbstractClassError<X>',
    'The specified class can.t be constructed because it.s an abstract class.',
    COMMON_DEFINITIONS + '''
struct X {
  X(int*) {}

  virtual void foo() = 0;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerConstructor<XAnnot(int*)>();
}
''')

def test_error_malformed_signature():
    expect_compile_error(
    'NotASignatureError<X\[\]>',
    'CandidateSignature was specified as parameter, but it.s not a signature. Signatures are of the form',
    COMMON_DEFINITIONS + '''
struct X {
  X(int) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerConstructor<X[]>();
}
''')

def test_error_malformed_signature_autoinject():
    expect_compile_error(
    'InjectTypedefNotASignatureError<X,X\[\]>',
    'C::Inject should be a typedef to a signature',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X[];
  X(int) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}
''')

def test_error_does_not_exist():
    expect_compile_error(
    'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
    'contains an Inject typedef but it.s not constructible with the specified types',
    COMMON_DEFINITIONS + '''
struct X {
  X(int*) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerConstructor<X(char*)>();
}
''')

def test_error_does_not_exist_with_annotation():
    expect_compile_error(
    'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
    'contains an Inject typedef but it.s not constructible with the specified types',
    COMMON_DEFINITIONS + '''
struct X {
  X(int*) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerConstructor<X(fruit::Annotated<Annotation, char*>)>();
}
''')

def test_error_does_not_exist_autoinject():
    expect_compile_error(
    'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
    'contains an Inject typedef but it.s not constructible with the specified types',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X(char*);
  X(int*) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}
''')

def test_error_does_not_exist_autoinject_with_annotation():
    expect_compile_error(
    'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
    'contains an Inject typedef but it.s not constructible with the specified types',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = X(fruit::Annotated<Annotation, char*>);
  X(int*) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}
''')

def test_error_abstract_class_autoinject():
    expect_compile_error(
    'CannotConstructAbstractClassError<Z>',
    'The specified class can.t be constructed because it.s an abstract class.',
    COMMON_DEFINITIONS + '''
struct X {
  using Inject = XAnnot();
};

struct Y {
  using Inject = YAnnot();
  Y() {}
};

struct Z {
  using Inject = ZAnnot();

  virtual void scale() = 0;
  // Note: here we "forgot" to implement scale() (on purpose, for this test) so Z is an abstract class.
};

fruit::Component<ZAnnot, YAnnot, XAnnot> getComponent() {
  return fruit::createComponent();
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
