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
    '''

class TestComponentFunctions(parameterized.TestCase):
    def test_component_function_success(self):
        source = '''
            struct X {
              int n;
              X(int n) : n(n) {}
            };
            
            struct Arg {
              Arg(int) {}
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = default;
            };
            
            bool operator==(const Arg&, const Arg&) {
              return true;
            }
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&) {
                  return 0;
                }
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg, Arg) {
              return fruit::createComponent()
                .registerProvider([]() { return X(5); });
            }
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), Arg{}, 15));
            }
    
            int main() {
              fruit::Injector<X> injector(getComponent);
              X x = injector.get<X>();
              Assert(x.n == 5);
            }
            '''
        expect_success(COMMON_DEFINITIONS, source)

    def test_component_function_no_args_success(self):
        source = '''
            struct X {
              int n;
              X(int n) : n(n) {}
            };
            
            fruit::Component<X> getParentComponent() {
              return fruit::createComponent()
                .registerProvider([]() { return X(5); });
            }
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent));
            }
    
            int main() {
              fruit::Injector<X> injector(getComponent);
              X x = injector.get<X>();
              Assert(x.n == 5);
            }
            '''
        expect_success(COMMON_DEFINITIONS, source)

    def test_component_function_one_arg_success(self):
        source = '''
            struct X {
              int n;
              X(int n) : n(n) {}
            };
            
            fruit::Component<X> getParentComponent(std::string) {
              return fruit::createComponent()
                .registerProvider([]() { return X(5); });
            }
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, std::string("Hello")));
            }
    
            int main() {
              fruit::Injector<X> injector(getComponent);
              X x = injector.get<X>();
              Assert(x.n == 5);
            }
            '''
        expect_success(COMMON_DEFINITIONS, source)

    def test_component_function_error_not_move_constructible(self):
        source = '''
            struct X {};
            
            struct Arg {
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = delete;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = default;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), Arg{}));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .Arg::Arg\(Arg&&\).'
            r'|error: call to deleted constructor of .Arg.'
            r'|.Arg::Arg\(Arg &&\).: cannot convert argument 1 from .std::_Tuple_val<Arg>. to .const Arg &.'
            r'|.Arg::Arg\(Arg &&\).: attempting to reference a deleted function',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_move_constructible_with_conversion(self):
        source = '''
            struct X {};
            
            struct Arg {
              Arg(int) {}
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = delete;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = default;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), 15));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .Arg::Arg\(Arg&&\).'
            r'|error: call to deleted constructor of .Arg.'
            r'|.Arg::Arg\(Arg &&\).: cannot convert argument 1 from .std::_Tuple_val<Arg>. to .int.'
            r'|error: copying parameter of type .Arg. invokes deleted constructor',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_copy_constructible(self):
        source = '''
            struct X {
              int n;
              X(int n) : n(n) {}
            };
            
            struct Arg {
              Arg() = default;
              Arg(const Arg&) = delete;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = default;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), Arg{}));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .Arg::Arg\(const Arg&\).'
            r'|error: call to deleted constructor of .Arg.'
            r'|error C2280: .Arg::Arg\(const Arg &\).: attempting to reference a deleted function'
            # This is the error printed by MSVC. It's not great but I couldn't find a way to have it print
            # a more useful error.
            r'|cannot convert argument 1 from .int. to .std::allocator_arg_t.',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_copy_constructible_with_conversion(self):
        source = '''
            struct X {
              int n;
              X(int n) : n(n) {}
            };
            
            struct Arg {
              Arg(int) {}
              Arg() = default;
              Arg(const Arg&) = delete;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = default;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), 15));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .Arg::Arg\(const Arg&\).'
            r'|error: call to deleted constructor of .Arg.'
            r'|error C2280: .Arg::Arg\(const Arg &\).: attempting to reference a deleted function'
            # This is the error printed by MSVC. It's not great but I couldn't find a way to have it print
            # a more useful error.
            r'|cannot convert argument 1 from .int. to .std::allocator_arg_t.',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_move_assignable(self):
        source = '''
            struct X {};
            
            struct Arg {
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = delete;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), Arg{}));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .Arg& Arg::operator=\(Arg&&\).'
            r'|error: overload resolution selected deleted operator .=.'
            r'|error C2280: .Arg &Arg::operator =\(Arg &&\).: attempting to reference a deleted function',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_move_assignable_with_conversion(self):
        source = '''
            struct X {};
            
            struct Arg {
              Arg(int) {}
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = delete;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), 15));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .Arg& Arg::operator=\(Arg&&\).'
            r'|error: overload resolution selected deleted operator .=.'
            r'|error C2280: .Arg &Arg::operator =\(Arg &&\).: attempting to reference a deleted function',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_copy_assignable(self):
        source = '''
            struct X {
              int n;
              X(int n) : n(n) {}
            };
            
            struct Arg {
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = delete;
              Arg& operator=(Arg&&) = default;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), Arg{}));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .Arg& Arg::operator=\(const Arg&\).'
            r'|error: overload resolution selected deleted operator .=.'
            r'|error C2280: .Arg &Arg::operator =\(const Arg &\).: attempting to reference a deleted function',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_copy_assignable_with_conversion(self):
        source = '''
            struct X {
              int n;
              X(int n) : n(n) {}
            };
            
            struct Arg {
              Arg(int) {}
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = delete;
              Arg& operator=(Arg&&) = default;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), 15));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .Arg& Arg::operator=\(const Arg&\).'
            r'|error: overload resolution selected deleted operator .=.'
            r'|error C2280: .Arg &Arg::operator =\(const Arg &\).: attempting to reference a deleted function',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_equality_comparable(self):
        source = '''
            struct X {
              int n;
              X(int n) : n(n) {}
            };
            
            struct Arg {
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = default;
            };
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), Arg{}));
            }
            '''
        expect_generic_compile_error(
            r'error: no match for .operator==. \(operand types are .const Arg. and .const Arg.\)'
            r'|error: invalid operands to binary expression \(.const Arg. and .const Arg.\)'
            r'|error C2676: binary .==.: .const Arg. does not define this operator',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_equality_comparable_with_conversion(self):
        source = '''
            struct X {
              int n;
              X(int n) : n(n) {}
            };
            
            struct Arg {
              Arg(int) {}
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = default;
            };
            
            namespace std {
              template <>
              struct hash<Arg> {
                size_t operator()(const Arg&);
              };
            }
    
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), 15));
            }
            '''
        expect_generic_compile_error(
            r'error: no match for .operator==. \(operand types are .const Arg. and .const Arg.\)'
            r'|error: invalid operands to binary expression \(.const Arg. and .const Arg.\)'
            r'|error C2676: binary .==.: .const Arg. does not define this operator',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_hashable(self):
        source = '''
            struct X {};
            
            struct Arg {
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = default;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), Arg{}));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .std::hash<Arg>::hash\(\).'
            r'|error: call to implicitly-deleted default constructor of .std::hash<Arg>.'
            r'|error: invalid use of incomplete type .struct std::hash<Arg>.'
            r'|error: implicit instantiation of undefined template .std::(__1::)?hash<Arg>.'
            r'|error C2338: The C\+\+ Standard doesn.t provide a hash for this type.'
            r'|error C2064: term does not evaluate to a function taking 1 arguments',
            COMMON_DEFINITIONS,
            source)

    def test_component_function_error_not_hashable_with_conversion(self):
        source = '''
            struct X {};
            
            struct Arg {
              Arg(int) {}
              Arg() = default;
              Arg(const Arg&) = default;
              Arg(Arg&&) = default;
              Arg& operator=(const Arg&) = default;
              Arg& operator=(Arg&&) = default;
            };
            
            bool operator==(const Arg&, const Arg&);
            
            fruit::Component<X> getParentComponent(int, std::string, Arg);
    
            fruit::Component<X> getComponent() {
              return fruit::createComponent()
                .installComponentFunctions(fruit::componentFunction(getParentComponent, 5, std::string("Hello"), 15));
            }
            '''
        expect_generic_compile_error(
            r'error: use of deleted function .std::hash<Arg>::hash\(\).'
            r'|error: call to implicitly-deleted default constructor of .std::hash<Arg>.'
            r'|error: invalid use of incomplete type .struct std::hash<Arg>.'
            r'|error: implicit instantiation of undefined template .std::(__1::)?hash<Arg>.'
            r'|error C2338: The C\+\+ Standard doesn.t provide a hash for this type.'
            r'|error C2064: term does not evaluate to a function taking 1 arguments',
            COMMON_DEFINITIONS,
            source)

if __name__ == '__main__':
    absltest.main()
