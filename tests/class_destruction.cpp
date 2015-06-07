// expect-success
/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fruit/fruit.h>
#include "test_macros.h"

using fruit::Component;
using fruit::Injector;

// The shared_ptr objects below ensure (since these tests are run under Valgrind) that deletion occurs, and only once.

struct I1 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
  virtual ~I1() {}
};

struct I2 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct I3 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct I4 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X1 : I1 {
  INJECT(X1()) = default;
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X2 : I2 {
  // Taking an X1 here prevents binding compression.
  INJECT(X2(X1)) {}
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X3 : public I3 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X4 : public I4 {
  // Taking an X3 here prevents binding compression.
  X4(X3) {};
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X5 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};


struct X6 : public I1 {
  INJECT(X6()) = default;
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X7 : public I1 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
};

struct X8 : public I1 {
  std::shared_ptr<int> x = std::make_shared<int>(3);
  virtual ~X8() {}
};

Component<I1, I2, I3, I4, X5> getComponent() {
  static X5 x5;
  static std::unique_ptr<I1> x7(new X7());
  return fruit::createComponent()
      .bind<I1, X1>()
      .bind<I2, X2>()
      .bind<I3, X3>()
      .bind<I4, X4>()
      .registerProvider([]() { return X3(); })
      .registerProvider([](X3 x3) { return X4(x3); })
      .bindInstance(x5)
      .addMultibinding<I1, X6>()
      .addInstanceMultibinding(*x7)
      .addMultibindingProvider([]() { return (X1*) new X8(); });
}

int main() {
  // Create an injector without creating any instances.
  Injector<I1, I2, I3, I4, X5> injector1(getComponent());
  
  // And an injector where we do create the instances.
  Injector<I1, I2, I3, I4, X5> injector2(getComponent());
  
  injector2.get<I1*>();
  injector2.get<I2*>();
  injector2.get<I3*>();
  injector2.get<I4*>();
  injector2.get<X5>();
  
  injector2.getMultibindings<I1>();
  
  return 0;
}
