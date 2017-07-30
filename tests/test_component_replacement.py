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
import pytest

from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #include "test_common.h"

    '''

@pytest.mark.parametrize('ReplacedComponentParamTypes,ReplacedComponentInstallation', [
    ('', 'getReplacedComponent'),
    ('double', 'getReplacedComponent, 1.0'),
])
@pytest.mark.parametrize('ReplacementComponentParamTypes,ReplacementComponentInstallation', [
    ('', 'getReplacementComponent'),
    ('std::string', 'getReplacementComponent, std::string("Hello, world")'),
])
def test_replace_component_success(
        ReplacedComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentParamTypes, ReplacementComponentInstallation):
    source = '''
        fruit::Component<int> getReplacedComponent(ReplacedComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getRootComponent() {
          return fruit::createComponent()
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .install(ReplacedComponentInstallation);
        }
        
        int main() {
          fruit::Injector<int> injector(getRootComponent());
          int n = injector.get<int>();
          Assert(n == 20);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ComponentParamTypes,ReplacedComponentInstallation,ReplacementComponentInstallation,ReplacementReplacementComponentInstallation', [
    ('', 'getReplacedComponent', 'getReplacementComponent', 'getReplacementReplacementComponent'),
    ('double', 'getReplacedComponent, 1.0', 'getReplacementComponent, 1.0', 'getReplacementReplacementComponent, 1.0'),
])
def test_replace_component_chain_success(
        ComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentInstallation, ReplacementReplacementComponentInstallation):
    source = '''
        fruit::Component<int> getReplacedComponent(ComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getReplacementComponent(ComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getReplacementReplacementComponent(ComponentParamTypes) {
          static int n = 30;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getRootComponent() {
          return fruit::createComponent()
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .replace(ReplacementComponentInstallation).with(ReplacementReplacementComponentInstallation)
              .install(ReplacedComponentInstallation);
        }
        
        int main() {
          fruit::Injector<int> injector(getRootComponent());
          int n = injector.get<int>();
          Assert(n == 30);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ComponentParamTypes,ReplacedComponentInstallation,ReplacementComponentInstallation,ReplacementReplacementComponentInstallation', [
    ('', 'getReplacedComponent', 'getReplacementComponent', 'getReplacementReplacementComponent'),
    ('double', 'getReplacedComponent, 1.0', 'getReplacementComponent, 1.0', 'getReplacementReplacementComponent, 1.0'),
])
def test_replace_component_chain_other_order_success(
        ComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentInstallation, ReplacementReplacementComponentInstallation):
    source = '''
        fruit::Component<int> getReplacedComponent(ComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getReplacementComponent(ComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getReplacementReplacementComponent(ComponentParamTypes) {
          static int n = 30;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getRootComponent() {
          return fruit::createComponent()
              .replace(ReplacementComponentInstallation).with(ReplacementReplacementComponentInstallation)
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .install(ReplacedComponentInstallation);
        }
        
        int main() {
          fruit::Injector<int> injector(getRootComponent());
          int n = injector.get<int>();
          Assert(n == 30);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_replace_component_different_type_error():
    source = '''
        fruit::Component<int> getReplacedComponent() {
          static int n = 10;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<double> getReplacementComponent() {
          static const double n = 20.0;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
              .replace(getReplacedComponent).with(getReplacementComponent);
        }
        '''
    expect_generic_compile_error(
        # Clang
        'candidate template ignored: could not match .Component<int>. against .Component<double>.'
        # GCC
        '|mismatched types .int. and .double.'
        # MSVC
        '|could not deduce template argument for .fruit::Component<int> \(__cdecl \*\)\(ReplacementFunArgs...\). from .fruit::Component<double> \(void\).',
        COMMON_DEFINITIONS,
        source)

@pytest.mark.parametrize('ReplacedComponentParamTypes,ReplacedComponentInstallation', [
    ('', 'getReplacedComponent'),
    ('double', 'getReplacedComponent, 1.0'),
])
@pytest.mark.parametrize('ReplacementComponentParamTypes,ReplacementComponentInstallation', [
    ('', 'getReplacementComponent'),
    ('std::string', 'getReplacementComponent, std::string("Hello, world")'),
])
def test_replace_component_already_replaced_consistent_ok(
        ReplacedComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentParamTypes, ReplacementComponentInstallation):
    source = '''
        fruit::Component<int> getReplacedComponent(ReplacedComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getRootComponent() {
          return fruit::createComponent()
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .install(ReplacedComponentInstallation);
        }
        
        int main() {
          fruit::Injector<int> injector(getRootComponent());
          int n = injector.get<int>();
          Assert(n == 20);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ReplacedComponentParamTypes,ReplacedComponentInstallation', [
    ('', 'getReplacedComponent'),
    ('double', 'getReplacedComponent, 1.0'),
])
@pytest.mark.parametrize('ReplacementComponentParamTypes,ReplacementComponentInstallation,OtherReplacementComponentInstallation', [
    ('', 'getReplacementComponent', 'getOtherReplacementComponent'),
    ('std::string', 'getReplacementComponent, std::string("Hello, world")', 'getOtherReplacementComponent, std::string("Hello, world")'),
])
def test_replace_component_already_replaced_inconsistent_error(
        ReplacedComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentParamTypes, ReplacementComponentInstallation, OtherReplacementComponentInstallation):
    source = '''
        fruit::Component<int> getReplacedComponent(ReplacedComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getOtherReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 30;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .replace(ReplacedComponentInstallation).with(OtherReplacementComponentInstallation);
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent());
          (void) injector;
        }
        '''
    expect_runtime_error(
        'Fatal injection error: the component function at (0x)?[0-9a-fA-F]* with signature '
            + '(class )?fruit::Component<int> \((__cdecl)?\*\)\((void)?ReplacedComponentParamTypes\) was replaced '
            + '\(using .replace\(...\).with\(...\)\) with both the component function at (0x)?[0-9a-fA-F]* with signature '
            + '(class )?fruit::Component<int> \((__cdecl)?\*\)\(.*\) and the component function at '
            + '(0x)?[0-9a-fA-F]* with signature (class )?fruit::Component<int> \((__cdecl)?\*\)\(.*\) .',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ReplacedComponentParamTypes,ReplacedComponentInstallation', [
    ('', 'getReplacedComponent'),
    ('double', 'getReplacedComponent, 1.0'),
])
@pytest.mark.parametrize('ReplacementComponentParamTypes,ReplacementComponentInstallation', [
    ('', 'getReplacementComponent'),
    ('std::string', 'getReplacementComponent, std::string("Hello, world")'),
])
def test_replace_component_after_install_error(
        ReplacedComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentParamTypes, ReplacementComponentInstallation):
    source = '''
        fruit::Component<int> getReplacedComponent(ReplacedComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .bindInstance(n);
        }
        
        fruit::Component<int> getRootComponent() {
          return fruit::createComponent()
              .install(ReplacedComponentInstallation)
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation);
        }
        
        int main() {
          fruit::Injector<int> injector(getRootComponent());
          (void) injector;
        }
        '''
    expect_runtime_error(
        'Fatal injection error: unable to replace \(using .replace\(...\).with\(...\)\) the component function at '
            + '(0x)?[0-9a-fA-F]* with signature (class )?fruit::Component<int> \((__cdecl)?\*\)\((void)?ReplacedComponentParamTypes\) with the '
            + 'component function at (0x)?[0-9a-fA-F]* with signature '
            + '(class )?fruit::Component<int> \((__cdecl)?\*\)\(.*\) because the former component function '
            + 'was installed before the .replace\(...\).with\(...\).',
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ReplacedComponentParamTypes,ReplacedComponentInstallation,ReplacementComponentParamTypes,ReplacementComponentInstallation', [
    ('', 'getReplacedComponent', '', 'getReplacementComponent'),
    ('double', 'getReplacedComponent, 1.0', 'std::string', 'getReplacementComponent, std::string("Hello, world")'),
])
def test_replace_component_unused_ok(
        ReplacedComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentParamTypes, ReplacementComponentInstallation):
    source = '''
        fruit::Component<> getReplacedComponent(ReplacedComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation);
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 0);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ReplacedComponentParamTypes,ReplacedComponentInstallation', [
    ('', 'getReplacedComponent'),
    ('double', 'getReplacedComponent, 1.0'),
])
@pytest.mark.parametrize('ReplacementComponentParamTypes,ReplacementComponentInstallation', [
    ('', 'getReplacementComponent'),
    ('std::string', 'getReplacementComponent, std::string("Hello, world")'),
])
def test_replace_component_used_multiple_times_ok(
        ReplacedComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentParamTypes, ReplacementComponentInstallation):
    source = '''
        fruit::Component<> getReplacedComponent(ReplacedComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .install(ReplacedComponentInstallation)
              .install(ReplacedComponentInstallation);
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 1);
          Assert(*(multibindings[0]) == 20);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ReplacedComponentParamTypes,ReplacedComponentInstallation', [
    ('', 'getReplacedComponent'),
    ('double', 'getReplacedComponent, 1.0'),
])
@pytest.mark.parametrize('ReplacementComponentParamTypes,ReplacementComponentInstallation', [
    ('', 'getReplacementComponent'),
    ('std::string', 'getReplacementComponent, std::string("Hello, world")'),
])
def test_replace_component_also_installed_directly_before_ok(
        ReplacedComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentParamTypes, ReplacementComponentInstallation):
    source = '''
        fruit::Component<> getReplacedComponent(ReplacedComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
              .install(ReplacementComponentInstallation)
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .install(ReplacedComponentInstallation);
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 1);
          Assert(*(multibindings[0]) == 20);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ReplacedComponentParamTypes,ReplacedComponentInstallation', [
    ('', 'getReplacedComponent'),
    ('double', 'getReplacedComponent, 1.0'),
])
@pytest.mark.parametrize('ReplacementComponentParamTypes,ReplacementComponentInstallation', [
    ('', 'getReplacementComponent'),
    ('std::string', 'getReplacementComponent, std::string("Hello, world")'),
])
def test_replace_component_also_installed_directly_after_ok(
        ReplacedComponentParamTypes, ReplacedComponentInstallation, ReplacementComponentParamTypes, ReplacementComponentInstallation):
    source = '''
        fruit::Component<> getReplacedComponent(ReplacedComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .install(ReplacedComponentInstallation)
              .install(ReplacementComponentInstallation);
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 1);
          Assert(*(multibindings[0]) == 20);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@pytest.mark.parametrize('ReplacedComponentParamTypes,ReplacedComponentInstallation,OtherReplacedComponentInstallation', [
    ('', 'getReplacedComponent', 'getOtherReplacementComponent'),
    ('double', 'getReplacedComponent, 1.0', 'getOtherReplacementComponent, 1.0'),
])
@pytest.mark.parametrize('ReplacementComponentParamTypes,ReplacementComponentInstallation', [
    ('', 'getReplacementComponent'),
    ('std::string', 'getReplacementComponent, std::string("Hello, world")'),
])
def test_replace_multiple_components_with_same(
        ReplacedComponentParamTypes, ReplacedComponentInstallation, OtherReplacedComponentInstallation, ReplacementComponentParamTypes, ReplacementComponentInstallation):
    source = '''
        fruit::Component<> getReplacedComponent(ReplacedComponentParamTypes) {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getOtherReplacementComponent(ReplacedComponentParamTypes) {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent(ReplacementComponentParamTypes) {
          static int n = 30;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
              .replace(ReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .replace(OtherReplacedComponentInstallation).with(ReplacementComponentInstallation)
              .install(ReplacedComponentInstallation)
              .install(OtherReplacedComponentInstallation);
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 1);
          Assert(*(multibindings[0]) == 30);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_replace_component_one_set_of_args_only():
    source = '''
        fruit::Component<> getReplacedComponent(double) {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent() {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
              .replace(getReplacedComponent, 1.0).with(getReplacementComponent)
              .install(getReplacedComponent, 1.0)
              .install(getReplacedComponent, 5.0);
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 2);
          Assert(*(multibindings[0]) == 20);
          Assert(*(multibindings[1]) == 10);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_replace_component_already_replaced_with_different_args():
    source = '''
        fruit::Component<> getReplacedComponent(double) {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent() {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getOtherReplacementComponent() {
          static int n = 30;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent() {
          return fruit::createComponent()
              .replace(getReplacedComponent, 1.0).with(getReplacementComponent)
              .replace(getReplacedComponent, 5.0).with(getOtherReplacementComponent)
              .install(getReplacedComponent, 1.0)
              .install(getReplacedComponent, 5.0);
        }
        
        int main() {
          fruit::Injector<> injector(getRootComponent());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 2);
          Assert(*(multibindings[0]) == 20);
          Assert(*(multibindings[1]) == 30);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_replacement_in_normalized_component_no_effect_on_other_component():
    source = '''
        fruit::Component<> getReplacedComponent() {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent() {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent1() {
          return fruit::createComponent()
              .replace(getReplacedComponent).with(getReplacementComponent);
        }
        
        fruit::Component<> getRootComponent2() {
          return fruit::createComponent()
              .install(getReplacedComponent);
        }
        
        int main() {
          fruit::NormalizedComponent<> normalized_component(getRootComponent1());
          fruit::Injector<> injector(normalized_component, getRootComponent2());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 1);
          Assert(*(multibindings[0]) == 10);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_replacement_in_other_component_no_effect_on_normalized_component():
    source = '''
        fruit::Component<> getReplacedComponent() {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent() {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent1() {
          return fruit::createComponent()
              .install(getReplacedComponent);
        }
        
        fruit::Component<> getRootComponent2() {
          return fruit::createComponent()
              .replace(getReplacedComponent).with(getReplacementComponent);
        }
        
        int main() {
          fruit::NormalizedComponent<> normalized_component(getRootComponent1());
          fruit::Injector<> injector(normalized_component, getRootComponent2());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 1);
          Assert(*(multibindings[0]) == 10);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())


def test_replace_component_different_type_in_normalized_component_and_other_component_ok():
    source = '''
        fruit::Component<> getReplacedComponent() {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent() {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getOtherReplacementComponent() {
          static int n = 30;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent1() {
          return fruit::createComponent()
              .replace(getReplacedComponent).with(getReplacementComponent)
              .install(getReplacedComponent);
        }
        
        fruit::Component<> getRootComponent2() {
          return fruit::createComponent()
              .replace(getReplacedComponent).with(getOtherReplacementComponent)
              .install(getReplacedComponent);
        }
        
        int main() {
          fruit::NormalizedComponent<> normalized_component(getRootComponent1());
          fruit::Injector<> injector(normalized_component, getRootComponent2());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 2);
          Assert(*(multibindings[0]) == 20);
          Assert(*(multibindings[1]) == 30);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_replace_component_in_normalized_component_also_installed_directly_in_other_component():
    source = '''
        fruit::Component<> getReplacedComponent() {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent() {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent1() {
          return fruit::createComponent()
              .replace(getReplacedComponent).with(getReplacementComponent)
              .install(getReplacedComponent);
        }
        
        fruit::Component<> getRootComponent2() {
          return fruit::createComponent()
              .install(getReplacedComponent);
        }
        
        int main() {
          fruit::NormalizedComponent<> normalized_component(getRootComponent1());
          fruit::Injector<> injector(normalized_component, getRootComponent2());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 2);
          Assert(*(multibindings[0]) == 20);
          Assert(*(multibindings[1]) == 10);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

def test_replace_component_in_other_component_also_installed_directly_in_normalized_component():
    source = '''
        fruit::Component<> getReplacedComponent() {
          static int n = 10;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getReplacementComponent() {
          static int n = 20;
          return fruit::createComponent()
              .addInstanceMultibinding(n);
        }
        
        fruit::Component<> getRootComponent1() {
          return fruit::createComponent()
              .install(getReplacedComponent);
        }
        
        fruit::Component<> getRootComponent2() {
          return fruit::createComponent()
              .replace(getReplacedComponent).with(getReplacementComponent)
              .install(getReplacedComponent);
        }
        
        int main() {
          fruit::NormalizedComponent<> normalized_component(getRootComponent1());
          fruit::Injector<> injector(normalized_component, getRootComponent2());
          
          std::vector<int*> multibindings = injector.getMultibindings<int>();
          Assert(multibindings.size() == 2);
          Assert(*(multibindings[0]) == 10);
          Assert(*(multibindings[1]) == 20);
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

if __name__== '__main__':
    main(__file__)
