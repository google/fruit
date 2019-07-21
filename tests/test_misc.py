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

from absl.testing import parameterized
from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #include "test_common.h"
    
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
    '''

class TestMisc(parameterized.TestCase):

    def test_misc(self):
        source = '''
            fruit::Component<X<V<float>>> getXProvider2() {
              return fruit::createComponent()
                  .registerProvider([](){return X<V<float>>(1);});
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
            
            fruit::Component<Interface2, XFactory, std::function<Implementation1(int)>> getParentComponent() {
              return fruit::createComponent()
                  .registerFactory<Implementation1(fruit::Assisted<int>, XFactory)>(
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
              INJECT(Implementation3(Implementation2*, fruit::Provider<Implementation2> provider)) {
                (void) provider.get();
                std::cout << "Called Implementation2()" << std::endl;
              }
              
              virtual ~Implementation3() {}
              
              virtual void f() {};
            };
            
            fruit::Component<Interface3, std::function<Implementation1(int)>> getMyComponent() {
              return fruit::createComponent()
                  // Must fail at runtime.
                  // .install(getXProvider2)
                  .bind<Interface3, Implementation3>()
                  .install(getParentComponent);
            }
            
            fruit::Component<std::function<AssistedMultiparamExample(std::map<int, float>)>> getAssistedMultiparamExampleComponent() {
              return fruit::createComponent();
            }
            
            int main() {
              fruit::Injector<
                Interface3,
                // XFactory,
                std::function<Implementation1(int)>
                > oldInjector(getMyComponent);
            
              // The move is completely unnecessary, it's just to check that it works.
              fruit::Injector<
                Interface3,
                // XFactory,
                std::function<Implementation1(int)>
                > injector(std::move(oldInjector));
              
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
              
              fruit::Injector<std::function<AssistedMultiparamExample(std::map<int, float>)>> assistedMultiparamExampleInjector(
                getAssistedMultiparamExampleComponent);
              
              return 0;
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
