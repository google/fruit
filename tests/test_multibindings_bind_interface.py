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
from fruit_test_config import CXX_COMPILER_NAME
import unittest
import re

def test_error_not_base():
    expect_compile_error(
    'NotABaseClassOfError<X,int>',
    'I is not a base class of C.',
    '''
struct X {};

Component<int> getComponent() {
  return fruit::createComponent()
    .addMultibinding<X, int>();
}
''')

def test_error_not_base_with_annotation():
    expect_compile_error(
    'NotABaseClassOfError<X,int>',
    'I is not a base class of C.',
    '''
struct Annotation {};

struct X {};

Component<int> getComponent() {
  return fruit::createComponent()
    .addMultibinding<fruit::Annotated<Annotation, X>, fruit::Annotated<Annotation, int>>();
}
''')

def test_error_abstract_class():
    expect_compile_error(
    'NoBindingFoundForAbstractClassError<ScalerImpl>',
    'No explicit binding was found for C, and C is an abstract class',
    '''
struct Annotation {};

class Scaler {
public:
  virtual double scale(double x) = 0;
};

struct ScalerImpl : public Scaler {
  // Note: here we "forgot" to implement scale() (on purpose, for this test) so ScalerImpl is an abstract class.
};

Component<> getComponent() {
  return fruit::createComponent()
    .addMultibinding<fruit::Annotated<Annotation, Scaler>, fruit::Annotated<Annotation, ScalerImpl>>();
}
''')

@unittest.skipUnless(
    re.search('Clang', CXX_COMPILER_NAME) is not None,
    'This is Clang-only because GCC >=4.9 refuses to even mention the type C() when C is an abstract class, '
    'while Clang allows to mention the type (but of course there can be no functions with this type)')
def test_error_abstract_class_clang():
    expect_compile_error(
    'CannotConstructAbstractClassError<ScalerImpl>',
    'The specified class can.t be constructed because it.s an abstract class.',
    '''
struct Annotation {};

class Scaler {
public:
  virtual double scale(double x) = 0;
};

struct ScalerImpl : public Scaler {
  INJECT(ScalerImpl()) = default;

  // Note: here we "forgot" to implement scale() (on purpose, for this test) so ScalerImpl is an abstract class.
};

Component<> getComponent() {
  return fruit::createComponent()
    .addMultibinding<fruit::Annotated<Annotation, Scaler>, fruit::Annotated<Annotation, ScalerImpl>>();
}
''')

if __name__ == '__main__':
    import nose2
    nose2.main()
