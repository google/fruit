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

    struct X;

    struct Annotation1 {};
    using XAnnot1 = fruit::Annotated<Annotation1, X>;
    '''

class TestRequiredTypes(parameterized.TestCase):

    def test_required_success(self):
        source = '''
            struct X {
                virtual void foo() = 0;
                virtual ~X() = default;
            };
            using XFactory = std::function<std::unique_ptr<X>()>;
            struct Y {
                XFactory xFactory;
    
                INJECT(Y(XFactory xFactory))
                    : xFactory(xFactory) {
                }
    
                void doStuff() {
                    xFactory()->foo();
                }
            };
            fruit::Component<fruit::Required<XFactory>, Y> getYComponent() {
                return fruit::createComponent();
            }
            struct XImpl : public X {
                INJECT(XImpl()) = default;
                void foo() override {}
            };
            fruit::Component<XFactory> getXFactoryComponent() {
                return fruit::createComponent()
                    .bind<X, XImpl>();
            }
            fruit::Component<Y> getComponent() {
                return fruit::createComponent()
                    .install(getYComponent)
                    .install(getXFactoryComponent);
            }
            int main() {
                fruit::Injector<Y> injector(getComponent);
                Y* y(injector);
                y->doStuff();
            }
            '''
        expect_success(COMMON_DEFINITIONS, source)

    def test_required_annotated_success(self):
        source = '''
            struct X {
                virtual void foo() = 0;
                virtual ~X() = default;
            };
            using XFactory = std::function<std::unique_ptr<X>()>;
            using XFactoryAnnot = fruit::Annotated<Annotation1, XFactory>;
            struct Y {
                XFactory xFactory;
    
                INJECT(Y(ANNOTATED(Annotation1, XFactory) xFactory))
                    : xFactory(xFactory) {
                }
    
                void doStuff() {
                    xFactory()->foo();
                }
            };
            fruit::Component<fruit::Required<XFactoryAnnot>, Y> getYComponent() {
                return fruit::createComponent();
            }
            struct XImpl : public X {
                INJECT(XImpl()) = default;
                void foo() override {}
            };
            fruit::Component<XFactoryAnnot> getXFactoryComponent() {
                return fruit::createComponent()
                    .bind<fruit::Annotated<Annotation1, X>, fruit::Annotated<Annotation1, XImpl>>();
            }
            fruit::Component<Y> getComponent() {
                return fruit::createComponent()
                    .install(getYComponent)
                    .install(getXFactoryComponent);
            }
            int main() {
                fruit::Injector<Y> injector(getComponent);
                Y* y(injector);
                y->doStuff();
            }
            '''
        expect_success(COMMON_DEFINITIONS, source)

    def test_required_forward_declared_success(self):
        source = '''
            struct X;
            using XFactory = std::function<std::unique_ptr<X>()>;
            struct Y {
                XFactory xFactory;
    
                INJECT(Y(XFactory xFactory))
                    : xFactory(xFactory) {
                }
    
                void doStuff();
            };
            fruit::Component<fruit::Required<XFactory>, Y> getYComponent() {
                return fruit::createComponent();
            }
            fruit::Component<XFactory> getXFactoryComponent();
            fruit::Component<Y> getComponent() {
                return fruit::createComponent()
                    .install(getYComponent)
                    .install(getXFactoryComponent);
            }
            int main() {
                fruit::Injector<Y> injector(getComponent);
                Y* y(injector);
                y->doStuff();
            }
    
            // We define X as late as possible, to make sure that all the above compiles even if X is only forward-declared.
            struct X {
                virtual void foo() = 0;
                virtual ~X() = default;
            };
            void Y::doStuff() {
                xFactory()->foo();
            }
            struct XImpl : public X {
                INJECT(XImpl()) = default;
                void foo() override {}
            };
            fruit::Component<XFactory> getXFactoryComponent() {
                return fruit::createComponent()
                    .bind<X, XImpl>();
            }
            '''
        expect_success(COMMON_DEFINITIONS, source)

    def test_required_annotated_forward_declared_success(self):
        source = '''
            struct X;
            using XFactory = std::function<std::unique_ptr<X>()>;
            using XFactoryAnnot = fruit::Annotated<Annotation1, XFactory>;
            struct Y {
                XFactory xFactory;
    
                INJECT(Y(ANNOTATED(Annotation1, XFactory) xFactory))
                    : xFactory(xFactory) {
                }
    
                void doStuff();
            };
            fruit::Component<fruit::Required<XFactoryAnnot>, Y> getYComponent() {
                return fruit::createComponent();
            }
            fruit::Component<XFactoryAnnot> getXFactoryComponent();
            fruit::Component<Y> getComponent() {
                return fruit::createComponent()
                    .install(getYComponent)
                    .install(getXFactoryComponent);
            }
            int main() {
                fruit::Injector<Y> injector(getComponent);
                Y* y(injector);
                y->doStuff();
            }
    
            // We define X as late as possible, to make sure that all the above compiles even if X is only forward-declared.
            struct X {
                virtual void foo() = 0;
                virtual ~X() = default;
            };
            void Y::doStuff() {
                xFactory()->foo();
            }
            struct XImpl : public X {
                INJECT(XImpl()) = default;
                void foo() override {}
            };
            fruit::Component<XFactoryAnnot> getXFactoryComponent() {
                return fruit::createComponent()
                    .bind<fruit::Annotated<Annotation1, X>, fruit::Annotated<Annotation1, XImpl>>();
            }
            '''
        expect_success(COMMON_DEFINITIONS, source)

    def test_required_const_forward_declared_success(self):
        source = '''
            struct X;
            using XFactory = std::function<std::unique_ptr<X>()>;
            struct Y {
                XFactory xFactory;
    
                INJECT(Y(XFactory xFactory))
                    : xFactory(xFactory) {
                }
    
                void doStuff();
            };
            fruit::Component<fruit::Required<const XFactory>, Y> getYComponent() {
                return fruit::createComponent();
            }
            fruit::Component<const XFactory> getXFactoryComponent();
            fruit::Component<Y> getComponent() {
                return fruit::createComponent()
                    .install(getYComponent)
                    .install(getXFactoryComponent);
            }
            int main() {
                fruit::Injector<Y> injector(getComponent);
                Y* y(injector);
                y->doStuff();
            }
    
            // We define X as late as possible, to make sure that all the above compiles even if X is only forward-declared.
            struct X {
                virtual void foo() = 0;
                virtual ~X() = default;
            };
            void Y::doStuff() {
                xFactory()->foo();
            }
            struct XImpl : public X {
                INJECT(XImpl()) = default;
                void foo() override {}
            };
            fruit::Component<const XFactory> getXFactoryComponent() {
                return fruit::createComponent()
                    .bind<X, XImpl>();
            }
            '''
        expect_success(COMMON_DEFINITIONS, source)

    def test_required_const_annotated_forward_declared_success(self):
        source = '''
            struct X;
            using XFactory = std::function<std::unique_ptr<X>()>;
            using ConstXFactoryAnnot = fruit::Annotated<Annotation1, const XFactory>;
            struct Y {
                XFactory xFactory;
    
                INJECT(Y(ANNOTATED(Annotation1, XFactory) xFactory))
                    : xFactory(xFactory) {
                }
    
                void doStuff();
            };
            fruit::Component<fruit::Required<ConstXFactoryAnnot>, Y> getYComponent() {
                return fruit::createComponent();
            }
            fruit::Component<ConstXFactoryAnnot> getXFactoryComponent();
            fruit::Component<Y> getComponent() {
                return fruit::createComponent()
                    .install(getYComponent)
                    .install(getXFactoryComponent);
            }
            int main() {
                fruit::Injector<Y> injector(getComponent);
                Y* y(injector);
                y->doStuff();
            }
    
            // We define X as late as possible, to make sure that all the above compiles even if X is only forward-declared.
            struct X {
                virtual void foo() = 0;
                virtual ~X() = default;
            };
            void Y::doStuff() {
                xFactory()->foo();
            }
            struct XImpl : public X {
                INJECT(XImpl()) = default;
                void foo() override {}
            };
            fruit::Component<ConstXFactoryAnnot> getXFactoryComponent() {
                return fruit::createComponent()
                    .bind<fruit::Annotated<Annotation1, X>, fruit::Annotated<Annotation1, XImpl>>();
            }
            '''
        expect_success(COMMON_DEFINITIONS, source)

if __name__ == '__main__':
    absltest.main()
