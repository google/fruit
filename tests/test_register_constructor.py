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

def test_success():
    expect_success(
    '''
struct X {
  INJECT(X()) = default;
  X(X&&) = default;
  X(const X&) = default;
};

struct X1 {
  X1() = default;
  X1(X1&&) = default;
  X1(const X1&) = default;
};

struct X2 {
  X2() = default;
  X2(X2&&) = default;
  X2(const X2&) = default;
};

struct Y {
  INJECT(Y()) = default;
  Y(Y&&) = default;
  Y(const Y&) = delete;
};

struct Y1 {
  Y1() = default;
  Y1(Y1&&) = default;
  Y1(const Y1&) = delete;
};

struct Y2 {
  Y2() = default;
  Y2(Y2&&) = default;
  Y2(const Y2&) = delete;
};

struct Z {
  INJECT(Z()) = default;
  Z(Z&&) = delete;
  Z(const Z&) = delete;
};

struct Z1 {
  Z1() = default;
  Z1(Z1&&) = delete;
  Z1(const Z1&) = delete;
};

struct Z2 {
  Z2() = default;
  Z2(Z2&&) = delete;
  Z2(const Z2&) = delete;
};

fruit::Component<X, X1, X2, Y, Y1, Y2> getComponent() {
  return fruit::createComponent()
    .registerProvider([](){return X1();})
    .registerProvider([](){return Y1();})
    .registerProvider([](){return new X2();})
    .registerProvider([](){return new Y2();})
    .registerProvider([](){return new Z2();});
}

template <typename T>
using Factory = std::function<T()>;

template <typename T>
using PtrFactory = std::function<std::unique_ptr<T>()>;

fruit::Component<Factory<X1>, PtrFactory<X2>, Factory<Y1>, PtrFactory<Y2>, PtrFactory<Z2>> getFactoryComponent() {
  return fruit::createComponent()
    .registerFactory<X1()>([](){return X1();})
    .registerFactory<Y1()>([](){return Y1();})
    .registerFactory<std::unique_ptr<X2>()>([](){return std::unique_ptr<X2>();})
    .registerFactory<std::unique_ptr<Y2>()>([](){return std::unique_ptr<Y2>();})
    .registerFactory<std::unique_ptr<Z2>()>([](){return std::unique_ptr<Z2>();});
}

int main() {
  Injector<X, X1, X2, Y, Y1, Y2> injector(getComponent());
  injector.get<X*>();
  injector.get<X1*>();
  injector.get<X2*>();
  injector.get<Y*>();
  injector.get<Y1*>();
  injector.get<Y2*>();

  Injector<Factory<X1>, PtrFactory<X2>, Factory<Y1>, PtrFactory<Y2>, PtrFactory<Z2>> injector2(getFactoryComponent());
  injector2.get<Factory<X1>>()();
  injector2.get<PtrFactory<X2>>()();
  injector2.get<Factory<Y1>>()();
  injector2.get<PtrFactory<Y2>>()();
  injector2.get<PtrFactory<Z2>>()();

  return 0;
}
''')

def test_with_annotation_success():
    expect_success(
    '''
struct Annotation {};

struct X {
  using Inject = X();
  X() = default;
  X(X&&) = default;
  X(const X&) = default;
};

struct X1 {
  X1() = default;
  X1(X1&&) = default;
  X1(const X1&) = default;
};

struct X2 {
  X2() = default;
  X2(X2&&) = default;
  X2(const X2&) = default;
};

struct Y {
  using Inject = Y();
  Y() = default;
  Y(Y&&) = default;
  Y(const Y&) = delete;
};

struct Y1 {
  Y1() = default;
  Y1(Y1&&) = default;
  Y1(const Y1&) = delete;
};

struct Y2 {
  Y2() = default;
  Y2(Y2&&) = default;
  Y2(const Y2&) = delete;
};

struct Z {
  using Inject = Z();
  Z() = default;
  Z(Z&&) = delete;
  Z(const Z&) = delete;
};

struct Z1 {
  Z1() = default;
  Z1(Z1&&) = delete;
  Z1(const Z1&) = delete;
};

struct Z2 {
  Z2() = default;
  Z2(Z2&&) = delete;
  Z2(const Z2&) = delete;
};

using XAnnot = fruit::Annotated<Annotation, X>;
using X1Annot = fruit::Annotated<Annotation, X1>;
using X2Annot = fruit::Annotated<Annotation, X2>;
using YAnnot = fruit::Annotated<Annotation, Y>;
using Y1Annot = fruit::Annotated<Annotation, Y1>;
using Y2Annot = fruit::Annotated<Annotation, Y2>;
using ZAnnot = fruit::Annotated<Annotation, Z>;
using Z1Annot = fruit::Annotated<Annotation, Z1>;
using Z2Annot = fruit::Annotated<Annotation, Z2>;

fruit::Component<XAnnot, X1Annot, X2Annot, YAnnot, Y1Annot, Y2Annot> getComponent() {
  return fruit::createComponent()
    .registerProvider<X1Annot()>([](){return X1();})
    .registerProvider<Y1Annot()>([](){return Y1();})
    .registerProvider<fruit::Annotated<Annotation, X2*>()>([](){return new X2();})
    .registerProvider<fruit::Annotated<Annotation, Y2*>()>([](){return new Y2();})
    .registerProvider<fruit::Annotated<Annotation, Z2*>()>([](){return new Z2();});
}

template <typename T>
using FactoryAnnot = fruit::Annotated<Annotation, std::function<T()>>;

template <typename T>
using PtrFactoryAnnot = fruit::Annotated<Annotation, std::function<std::unique_ptr<T>()>>;

fruit::Component<FactoryAnnot<X1>, PtrFactoryAnnot<X2>, FactoryAnnot<Y1>, PtrFactoryAnnot<Y2>, PtrFactoryAnnot<Z2>> getFactoryComponent() {
  return fruit::createComponent()
    .registerFactory<X1Annot()>([](){return X1();})
    .registerFactory<Y1Annot()>([](){return Y1();})
    .registerFactory<fruit::Annotated<Annotation, std::unique_ptr<X2>>()>([](){return std::unique_ptr<X2>();})
    .registerFactory<fruit::Annotated<Annotation, std::unique_ptr<Y2>>()>([](){return std::unique_ptr<Y2>();})
    .registerFactory<fruit::Annotated<Annotation, std::unique_ptr<Z2>>()>([](){return std::unique_ptr<Z2>();});
}

int main() {
  Injector<XAnnot, X1Annot, X2Annot, YAnnot, Y1Annot, Y2Annot> injector(getComponent());
  injector.get<fruit::Annotated<Annotation, X* >>();
  injector.get<fruit::Annotated<Annotation, X1*>>();
  injector.get<fruit::Annotated<Annotation, X2*>>();
  injector.get<fruit::Annotated<Annotation, Y* >>();
  injector.get<fruit::Annotated<Annotation, Y1*>>();
  injector.get<fruit::Annotated<Annotation, Y2*>>();

  Injector<FactoryAnnot<X1>, PtrFactoryAnnot<X2>, FactoryAnnot<Y1>, PtrFactoryAnnot<Y2>, PtrFactoryAnnot<Z2>> injector2(getFactoryComponent());
  injector2.get<FactoryAnnot<X1>>()();
  injector2.get<PtrFactoryAnnot<X2>>()();
  injector2.get<FactoryAnnot<Y1>>()();
  injector2.get<PtrFactoryAnnot<Y2>>()();
  injector2.get<PtrFactoryAnnot<Z2>>()();

  return 0;
}
''')

def test_autoinject_success():
    expect_success(
    '''
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
  Injector<Y> injector(normalizedComponent, getComponent());

  Assert(!Y::constructed);
  injector.get<Y>();
  Assert(Y::constructed);

  return 0;
}
''')

def test_autoinject_with_annotation_success():
    expect_success(
    '''
struct Annotation {};

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

using XAnnot = fruit::Annotated<Annotation, X>;
using YAnnot = fruit::Annotated<Annotation, Y>;
using ZAnnot = fruit::Annotated<Annotation, Z>;

fruit::Component<ZAnnot, YAnnot, XAnnot> getComponent() {
  return fruit::createComponent();
}

int main() {
  fruit::NormalizedComponent<> normalizedComponent(fruit::createComponent());
  Injector<YAnnot> injector(normalizedComponent, getComponent());

  Assert(!Y::constructed);
  injector.get<YAnnot>();
  Assert(Y::constructed);

  return 0;
}
''')

def test_autoinject_annotation_in_signature_return_type():
    expect_compile_error(
    'InjectTypedefWithAnnotationError<X>',
    'C::Inject is a signature that returns an annotated type',
    '''
struct Annotation {};

struct X {
  using Inject = fruit::Annotated<Annotation, X>();
};

fruit::Component<fruit::Annotated<Annotation, X>> getComponent() {
  return fruit::createComponent();
}

int main() {
  return 0;
}
''')

def test_autoinject_wrong_class_in_typedef():
    expect_compile_error(
    'InjectTypedefForWrongClassError<Y,X>',
    'C::Inject is a signature, but does not return a C. Maybe the class C has no Inject typedef and',
    '''
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
    '''
struct Annotation {};

struct X {
  X(int*) {}

  virtual void foo() = 0;
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerConstructor<fruit::Annotated<Annotation, X>(int*)>();
}
''')

def test_error_malformed_signature():
    expect_compile_error(
    'NotASignatureError<X\[\]>',
    'CandidateSignature was specified as parameter, but it.s not a signature. Signatures are of the form',
    '''
struct X {
  X(int) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerConstructor<X[]>();
}

int main() {
  return 0;
}
''')

def test_error_malformed_signature_autoinject():
    expect_compile_error(
    'InjectTypedefNotASignatureError<X,X\[\]>',
    'C::Inject should be a typedef to a signature',
    '''
struct X {
  using Inject = X[];
  X(int) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  return 0;
}
''')

def test_error_does_not_exist():
    expect_compile_error(
    'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
    'contains an Inject typedef but it.s not constructible with the specified types',
    '''
struct X {
  X(int*) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerConstructor<X(char*)>();
}

int main() {
  return 0;
}
''')

def test_error_does_not_exist_with_annotation():
    expect_compile_error(
    'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
    'contains an Inject typedef but it.s not constructible with the specified types',
    '''
struct Annotation {};

struct X {
  X(int*) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent()
    .registerConstructor<X(fruit::Annotated<Annotation, char*>)>();
}

int main() {
  return 0;
}
''')

def test_error_does_not_exist_autoinject():
    expect_compile_error(
    'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
    'contains an Inject typedef but it.s not constructible with the specified types',
    '''
struct X {
  using Inject = X(char*);
  X(int*) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  return 0;
}
''')

def test_error_does_not_exist_autoinject_with_annotation():
    expect_compile_error(
    'NoConstructorMatchingInjectSignatureError<X,X\(char\*\)>',
    'contains an Inject typedef but it.s not constructible with the specified types',
    '''
struct Annotation {};

struct X {
  using Inject = X(fruit::Annotated<Annotation, char*>);
  X(int*) {}
};

fruit::Component<X> getComponent() {
  return fruit::createComponent();
}

int main() {
  return 0;
}
''')

def test_error_abstract_class_autoinject():
    expect_compile_error(
    'CannotConstructAbstractClassError<Z>',
    'The specified class can.t be constructed because it.s an abstract class.',
    '''
struct Annotation {};

struct X {
  using Inject = fruit::Annotated<Annotation, X>();
};

struct Y {
  using Inject = fruit::Annotated<Annotation, Y>();
  Y() {}
};

struct Z {
  using Inject = fruit::Annotated<Annotation, Z>();

  virtual void scale() = 0;
  // Note: here we "forgot" to implement scale() (on purpose, for this test) so Z is an abstract class.
};

using XAnnot = fruit::Annotated<Annotation, X>;
using YAnnot = fruit::Annotated<Annotation, Y>;
using ZAnnot = fruit::Annotated<Annotation, Z>;

fruit::Component<ZAnnot, YAnnot, XAnnot> getComponent() {
  return fruit::createComponent();
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
