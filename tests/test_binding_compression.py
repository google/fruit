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

    struct Annotation1 {};
    struct Annotation2 {};

    template <typename T>
    using WithNoAnnot = T;

    template <typename T>
    using WithAnnot1 = fruit::Annotated<Annotation1, T>;

    template <typename T>
    using WithAnnot2 = fruit::Annotated<Annotation2, T>;
    '''

class TestBindingCompression(parameterized.TestCase):
    @parameterized.parameters([
        ('I', 'X', 'WithNoAnnot'),
        ('fruit::Annotated<Annotation1, I>', 'fruit::Annotated<Annotation2, X>', 'WithAnnot1'),
    ])
    def test_provider_returning_value_success_with_annotation(self, IAnnot, XAnnot, WithAnnot):
        source = '''
            struct I {
              int value = 5;
            };
    
            struct X : public I, ConstructionTracker<X> {
            };
    
            fruit::Component<IAnnot> getComponent() {
              return fruit::createComponent()
                .registerProvider<XAnnot()>([](){return X();})
                .bind<IAnnot, XAnnot>();
            }
    
            int main() {
              fruit::Injector<IAnnot> injector(getComponent);
              Assert((injector.get<WithAnnot<I                 >>() .value == 5));
              Assert((injector.get<WithAnnot<I*                >>()->value == 5));
              Assert((injector.get<WithAnnot<I&                >>() .value == 5));
              Assert((injector.get<WithAnnot<const I           >>() .value == 5));
              Assert((injector.get<WithAnnot<const I*          >>()->value == 5));
              Assert((injector.get<WithAnnot<const I&          >>() .value == 5));
              Assert((injector.get<WithAnnot<std::shared_ptr<I>>>()->value == 5));
              Assert(fruit::impl::InjectorAccessorForTests::unsafeGet<WithAnnot<X>>(injector) == nullptr);
    
              Assert(X::num_objects_constructed == 1);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    @parameterized.parameters([
        ('I', 'X', 'X*', 'WithNoAnnot'),
        ('fruit::Annotated<Annotation1, I>', 'fruit::Annotated<Annotation2, X>', 'fruit::Annotated<Annotation2, X*>', 'WithAnnot1'),
    ])
    def test_provider_returning_pointer_success_with_annotation(self, IAnnot, XAnnot, XPtrAnnot, WithAnnot):
        source = '''
            struct I {
              int value = 5;
            };
    
            struct X : public I, ConstructionTracker<X> {
            };
    
            fruit::Component<IAnnot> getComponent() {
              return fruit::createComponent()
                .registerProvider<XPtrAnnot()>([](){return new X();})
                .bind<IAnnot, XAnnot>();
            }
    
            int main() {
              fruit::Injector<IAnnot> injector(getComponent);
              Assert((injector.get<WithAnnot<I                 >>() .value == 5));
              Assert((injector.get<WithAnnot<I*                >>()->value == 5));
              Assert((injector.get<WithAnnot<I&                >>() .value == 5));
              Assert((injector.get<WithAnnot<const I           >>() .value == 5));
              Assert((injector.get<WithAnnot<const I*          >>()->value == 5));
              Assert((injector.get<WithAnnot<const I&          >>() .value == 5));
              Assert((injector.get<WithAnnot<std::shared_ptr<I>>>()->value == 5));
              Assert(fruit::impl::InjectorAccessorForTests::unsafeGet<WithAnnot<X>>(injector) == nullptr);
              Assert(X::num_objects_constructed == 1);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_compression_undone(self):
        source = '''
            struct I1 {};
            struct C1 : public I1, ConstructionTracker<C1> {
              INJECT(C1()) = default;
            };
    
            struct I2 {};
            struct C2 : public I2 {
              INJECT(C2(I1*)) {}
            };
    
            fruit::Component<I1> getI1Component() {
              return fruit::createComponent()
                  .bind<I1, C1>();
            }
    
            fruit::Component<I2> getI2Component() {
              return fruit::createComponent()
                  .install(getI1Component)
                  .bind<I2, C2>();
            }
    
            struct X {
              // Intentionally C1 and not I1. This prevents binding compression for the I1->C1 edge.
              INJECT(X(C1*)) {}
            };
    
            fruit::Component<X> getXComponent() {
              return fruit::createComponent();
            }
    
            int main() {
              // Here the binding C2->I1->C1 is compressed into C2->C1.
              fruit::NormalizedComponent<I2> normalizedComponent(getI2Component);
    
              // However the binding X->C1 prevents binding compression on I1->C1, the binding compression must be undone.
              fruit::Injector<I2, X> injector(normalizedComponent, getXComponent);
    
              Assert(C1::num_objects_constructed == 0);
              injector.get<I2*>();
              injector.get<X*>();
              Assert(fruit::impl::InjectorAccessorForTests::unsafeGet<C1>(injector) != nullptr);
              Assert(C1::num_objects_constructed == 1);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source)

if __name__ == '__main__':
    absltest.main()
