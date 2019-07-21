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
    
    struct Annotation {};
    
    using I1Annot = fruit::Annotated<Annotation, I1>;
    using I2Annot = fruit::Annotated<Annotation, I2>;
    using I3Annot = fruit::Annotated<Annotation, I3>;
    using I4Annot = fruit::Annotated<Annotation, I4>;
    
    using X1Annot = fruit::Annotated<Annotation, X1>;
    using X2Annot = fruit::Annotated<Annotation, X2>;
    using X3Annot = fruit::Annotated<Annotation, X3>;
    using X4Annot = fruit::Annotated<Annotation, X4>;
    using X5Annot = fruit::Annotated<Annotation, X5>;
    using X6Annot = fruit::Annotated<Annotation, X6>;
    using X7Annot = fruit::Annotated<Annotation, X7>;
    using X8Annot = fruit::Annotated<Annotation, X8>;
    
    using X1PtrAnnot = fruit::Annotated<Annotation, X1*>;
    '''

class TestClassDestruction(parameterized.TestCase):
    @parameterized.parameters([
        ('I1', 'I2', 'I3', 'I4', 'X1', 'X2', 'X3', 'X4', 'X5', 'X6', 'X7', 'X8', 'X1*', 'bindInstance(x5)', 'addInstanceMultibinding(*x7)'),
        ('I1Annot', 'I2Annot', 'I3Annot', 'I4Annot', 'X1Annot', 'X2Annot', 'X3Annot', 'X4Annot', 'X5Annot', 'X6Annot', 'X7Annot', 'X8Annot', 'X1PtrAnnot', 'bindInstance<X5Annot>(x5)', 'addInstanceMultibinding<X7Annot>(*x7)'),
    ])
    def test_injector_creation_no_injection(self,
            I1Annot, I2Annot, I3Annot, I4Annot, X1Annot, X2Annot, X3Annot, X4Annot, X5Annot, X6Annot, X7Annot, X8Annot, X1PtrAnnot, bindX5Instance, addX7InstanceMultibinding):
        source = '''
            fruit::Component<I1Annot, I2Annot, I3Annot, I4Annot, X5Annot> getComponent() {
              static X5 x5;
              static std::unique_ptr<X7> x7(new X7());
              return fruit::createComponent()
                  .bind<I1Annot, X1Annot>()
                  .bind<I2Annot, X2Annot>()
                  .bind<I3Annot, X3Annot>()
                  .bind<I4Annot, X4Annot>()
                  .registerProvider<X3Annot()>([]() { return X3(); })
                  .registerProvider<X4Annot(X3Annot)>([](X3 x3) { return X4(x3); })
                  .bindX5Instance
                  .addMultibinding<I1Annot, X6Annot>()
                  .addX7InstanceMultibinding
                  .addMultibindingProvider<X1PtrAnnot()>([]() { return (X1*) new X8(); });
            }
            
            int main() {
              fruit::Injector<I1Annot, I2Annot, I3Annot, I4Annot, X5Annot> injector(getComponent);
              (void)injector;
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('I1', 'I2', 'I3', 'I4', 'X1', 'X2', 'X3', 'X4', 'X5', 'X6', 'X7', 'X8', 'X1*', 'bindInstance(x5)', 'addInstanceMultibinding(*x7)'),
        ('I1Annot', 'I2Annot', 'I3Annot', 'I4Annot', 'X1Annot', 'X2Annot', 'X3Annot', 'X4Annot', 'X5Annot', 'X6Annot', 'X7Annot', 'X8Annot', 'X1PtrAnnot', 'bindInstance<X5Annot>(x5)', 'addInstanceMultibinding<X7Annot>(*x7)'),
    ])
    def test_injector_creation_and_injection(self,
            I1Annot, I2Annot, I3Annot, I4Annot, X1Annot, X2Annot, X3Annot, X4Annot, X5Annot, X6Annot, X7Annot, X8Annot, X1PtrAnnot, bindX5Instance, addX7InstanceMultibinding):
        source = '''
            fruit::Component<I1Annot, I2Annot, I3Annot, I4Annot, X5Annot> getComponent() {
              static X5 x5;
              static std::unique_ptr<X7> x7(new X7());
              return fruit::createComponent()
                  .bind<I1Annot, X1Annot>()
                  .bind<I2Annot, X2Annot>()
                  .bind<I3Annot, X3Annot>()
                  .bind<I4Annot, X4Annot>()
                  .registerProvider<X3Annot()>([]() { return X3(); })
                  .registerProvider<X4Annot(X3Annot)>([](X3 x3) { return X4(x3); })
                  .bindX5Instance
                  .addMultibinding<I1Annot, X6Annot>()
                  .addX7InstanceMultibinding
                  .addMultibindingProvider<X1PtrAnnot()>([]() { return (X1*) new X8(); });
            }
            
            int main() {
              fruit::Injector<I1Annot, I2Annot, I3Annot, I4Annot, X5Annot> injector(getComponent);
              
              injector.get<I1Annot>();
              injector.get<I2Annot>();
              injector.get<I3Annot>();
              injector.get<I4Annot>();
              injector.get<X5Annot>();
              
              injector.getMultibindings<I1Annot>();
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
