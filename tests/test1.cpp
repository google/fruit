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

#include "fruit/fruit.h"
#include <map>
#include <iostream>

template <typename T>
class V {};

template <typename T>
class X {
private:
  X() {}
  
public:
  INJECT(X(ASSISTED(int))) {
  }
};

using XFactory = std::function<X<V<float>>(int)>;

fruit::Module<X<V<float>>> getXProvider2() {
  return fruit::createModule()
      .registerProvider<X<V<float>>()>([](){return X<V<float>>(1);});
}

struct AssistedMultiparamExample {
  INJECT(AssistedMultiparamExample(ASSISTED(std::map<int, float>))) {}
};

struct Implementation1 {
  bool constructed = true;
  
  Implementation1(V<int>&&, XFactory) {
    std::cout << "Called Implementation1() for object " << this << std::endl;
  }
  
  Implementation1() = delete;
  Implementation1(const Implementation1&) = delete;
  
  Implementation1& operator=(const Implementation1&) = delete;
  Implementation1& operator=(Implementation1&&) = delete;
  
  Implementation1(Implementation1&&) {
    std::cout << "Moving an Implementation1 into object" << this << std::endl;
  }
  
  ~Implementation1() {
    std::cout << "Called ~Implementation1() for object " << this << std::endl;
    constructed = 0;
  }
  
  int x;
};

struct Interface2 {
  virtual void f() = 0;
};

struct Implementation2 : public Interface2 {
  INJECT(Implementation2(std::function<Implementation1(int)>)) {
    std::cout << "Called Implementation2()" << std::endl;
  }
  
  virtual ~Implementation2() {}
  
  virtual void f() {};
};

fruit::Module<Interface2, XFactory, std::function<Implementation1(int)>> getParentModule() {
  using fruit::Module;
  using fruit::Assisted;
  using fruit::createModule;
  return createModule()
      .registerFactory<Implementation1(Assisted<int>, XFactory)>(
        [](int, XFactory xFactory) {
          return Implementation1(V<int>(), xFactory);
        })
      .bind<Interface2, Implementation2>();
}

//*************************************

struct Interface3 {
  virtual void f() = 0;
};

struct Implementation3 : public Interface3 {
  INJECT(Implementation3(Implementation2*, fruit::Injector<Implementation2> injector)) {
    (void) injector.get<Implementation2*>();
    std::cout << "Called Implementation2()" << std::endl;
  }
  
  virtual ~Implementation3() {}
  
  virtual void f() {};
};

fruit::Module<Interface3, std::function<Implementation1(int)>> getMyModule() {
  using fruit::Module;
  using fruit::Assisted;
  using fruit::createModule;
  return createModule()
      // Must fail at runtime.
      // .install(getXProvider2())
      .bind<Interface3, Implementation3>()
      .install(getParentModule());
};

using fruit::Module;
using fruit::Injector;
using fruit::createModule;

int main() {
  Module<Interface3> m = getMyModule();
      
  Injector<
    Interface3,
    // XFactory,
    std::function<Implementation1(int)>
    > injector = getMyModule();
  
  std::cout << "Constructing an Interface3" << std::endl;
  Interface3* interface3(injector);
  std::cout << std::endl;
  (void) interface3;
  
  std::cout << "Constructing another Interface3" << std::endl;
  Interface3* interface3_obj2 = injector.get<Interface3*>();
  std::cout << std::endl;
  (void) interface3_obj2;
  
  std::function<Implementation1(int)> implementation1Factory(injector);
  {
     std::cout << "Constructing another Implementation1" << std::endl;
     Implementation1 implementation1 = implementation1Factory(12);
     (void) implementation1;
  }
  std::cout << "Destroying injector" << std::endl;
  
  Module<std::function<AssistedMultiparamExample(std::map<int, float>)>> assistedMultiparamExampleModule =
    createModule();
  Injector<std::function<AssistedMultiparamExample(std::map<int, float>)>> assistedMultiparamExampleInjector =
    assistedMultiparamExampleModule;
  
  return 0;
}

