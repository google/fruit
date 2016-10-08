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

def test_provider_success():
    expect_success(
    '''
struct I {
  int value = 5;
};

struct X : public I {
  X() {
    ++num_constructions;
  }

  static unsigned num_constructions;
};

unsigned X::num_constructions = 0;

fruit::Component<I> getComponentWithProviderByValue() {
  return fruit::createComponent()
    .registerProvider([](){return X();})
    .bind<I, X>();
}

fruit::Component<I> getComponentWithPointerProvider() {
  return fruit::createComponent()
    .registerProvider([](){return new X();})
    .bind<I, X>();
}

int main() {
  Injector<I> injector1(getComponentWithProviderByValue());
  injector1.get<I*>();
  Injector<I> injector2(getComponentWithPointerProvider());
  injector2.get<I*>();

  Assert(injector2.get<I>().value == 5);
  Assert(injector2.get<I*>()->value == 5);
  Assert(injector2.get<I&>().value == 5);
  Assert(injector2.get<const I>().value == 5);
  Assert(injector2.get<const I*>()->value == 5);
  Assert(injector2.get<const I&>().value == 5);
  Assert(injector2.get<std::shared_ptr<I>>()->value == 5);

  Assert(X::num_constructions == 2);

  return 0;
}
''')

def test_provider_success_with_annotation():
    expect_success(
    '''
struct Annotation1 {};
struct Annotation2 {};

struct I {
  int value = 5;
};

struct X : public I {
  X() {
    ++num_constructions;
  }

  static unsigned num_constructions;
};

using IAnnot = fruit::Annotated<Annotation1, I>;
using XAnnot = fruit::Annotated<Annotation2, X>;

unsigned X::num_constructions = 0;

fruit::Component<IAnnot> getComponentWithProviderByValue() {
  return fruit::createComponent()
    .registerProvider<XAnnot()>([](){return X();})
    .bind<IAnnot, XAnnot>();
}

fruit::Component<IAnnot> getComponentWithPointerProvider() {
  return fruit::createComponent()
    .registerProvider<fruit::Annotated<Annotation2, X*>()>([](){return new X();})
    .bind<IAnnot, XAnnot>();
}

int main() {

  Injector<IAnnot> injector1(getComponentWithProviderByValue());
  injector1.get<fruit::Annotated<Annotation1, I*>>();
  Injector<IAnnot> injector2(getComponentWithPointerProvider());
  injector2.get<fruit::Annotated<Annotation1, I*>>();

  Assert((injector2.get<fruit::Annotated<Annotation1, I                 >>() .value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, I*                >>()->value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, I&                >>() .value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, const I           >>() .value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, const I*          >>()->value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, const I&          >>() .value == 5));
  Assert((injector2.get<fruit::Annotated<Annotation1, std::shared_ptr<I>>>()->value == 5));

  Assert(X::num_constructions == 2);

  return 0;
}
''')

def test_compression_undone():
    expect_success(
    '''
static bool c1_constructed = false;

struct I1 {};
struct C1 : public I1 {
  INJECT(C1()) {
    if (c1_constructed) {
      std::cerr << "C1 constructed twice!" << std::endl;
      exit(1);
    }

    c1_constructed = true;
  }
};

struct I2 {};
struct C2 : public I2 {
  INJECT(C2(I1*)) {}
};

Component<I1> getI1Component() {
  return fruit::createComponent()
      .bind<I1, C1>();
}

Component<I2> getI2Component() {
  return fruit::createComponent()
      .install(getI1Component())
      .bind<I2, C2>();
}

struct X {
  // Intentionally C1 and not I1. This prevents binding compression for the I1->C1 edge.
  INJECT(X(C1*)) {}
};

Component<X> getXComponent() {
  return fruit::createComponent();
}

int main() {
  // Here the binding C2->I1->C1 is compressed into C2->C1.
  fruit::NormalizedComponent<I2> normalizedComponent(getI2Component());

  // However the binding X->C1 prevents binding compression on I1->C1, the binding compression must be undone.
  Injector<I2, X> injector(normalizedComponent, getXComponent());

  // The check in C1's constructor ensures that only one instance of C1 is created.
  injector.get<I2*>();
  injector.get<X*>();

  return 0;
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
