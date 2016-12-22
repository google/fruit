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
from nose2.tools import params

from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #include "test_common.h"

    struct X;

    struct Annotation1 {};
    using XAnnot1 = fruit::Annotated<Annotation1, X>;

    struct Annotation2 {};
    using XAnnot2 = fruit::Annotated<Annotation2, X>;
    '''

@params(
    ('X', 'X', 'Y'),
    ('fruit::Annotated<Annotation1, X>', 'ANNOTATED(Annotation1, X)', 'fruit::Annotated<Annotation2, Y>'))
def test_success_normalized_component_provides_unused(XAnnot, X_ANNOT, YAnnot):
    source = '''
        struct X {};

        struct Y {
          INJECT(Y(X_ANNOT)) {};
        };

        fruit::Component<fruit::Required<XAnnot>, YAnnot> getComponent() {
          return fruit::createComponent();
        }

        fruit::Component<XAnnot> getXComponent(X& x) {
          return fruit::createComponent()
            .bindInstance<XAnnot, X>(x);
        }

        int main() {
          fruit::NormalizedComponent<fruit::Required<XAnnot>, YAnnot> normalizedComponent(getComponent());

          X x{};

          fruit::Injector<XAnnot> injector(normalizedComponent, getXComponent(x));
          injector.get<XAnnot>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'X', 'Y'),
    ('fruit::Annotated<Annotation1, X>', 'ANNOTATED(Annotation1, X)', 'fruit::Annotated<Annotation2, Y>'))
def test_success(XAnnot, X_ANNOT, YAnnot):
    source = '''
        struct X {};

        struct Y {
          INJECT(Y(X_ANNOT)) {};
        };

        fruit::Component<fruit::Required<XAnnot>, YAnnot> getComponent() {
          return fruit::createComponent();
        }

        fruit::Component<XAnnot> getXComponent(X& x) {
          return fruit::createComponent()
            .bindInstance<XAnnot, X>(x);
        }

        int main() {
          fruit::NormalizedComponent<fruit::Required<XAnnot>, YAnnot> normalizedComponent(getComponent());

          X x{};

          fruit::Injector<YAnnot> injector(normalizedComponent, getXComponent(x));
          injector.get<YAnnot>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params(
    ('X', 'X', 'Y'),
    ('fruit::Annotated<Annotation1, X>', 'ANNOTATED(Annotation1, X)', 'fruit::Annotated<Annotation2, Y>'))
def test_success_inline_component(XAnnot, X_ANNOT, YAnnot):
    source = '''
        struct X {};

        struct Y {
          INJECT(Y(X_ANNOT)) {};
        };

        fruit::Component<fruit::Required<XAnnot>, YAnnot> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::NormalizedComponent<fruit::Required<XAnnot>, YAnnot> normalizedComponent(getComponent());

          X x{};

          fruit::Injector<YAnnot> injector(normalizedComponent,
                                           fruit::Component<XAnnot>(fruit::createComponent().bindInstance<XAnnot, X>(x)));
          injector.get<YAnnot>();
        }
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_unsatisfied_requirements(XAnnot):
    source = '''
        struct X {
          INJECT(X());
        };

        fruit::Component<fruit::Required<XAnnot>> getComponent() {
          return fruit::createComponent();
        }

        int main() {
          fruit::NormalizedComponent<fruit::Required<XAnnot>> normalizedComponent(getComponent());
          fruit::Injector<> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));
        }
        '''
    expect_compile_error(
        'UnsatisfiedRequirementsInNormalizedComponentError<XAnnot>',
        'The requirements in UnsatisfiedRequirements are required by the NormalizedComponent but are not provided by the Component',
        COMMON_DEFINITIONS,
        source,
        locals())

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_repeated_type(XAnnot):
    source = '''
        struct X {};

        InstantiateType(fruit::NormalizedComponent<XAnnot, XAnnot>)
        '''
    expect_compile_error(
        'RepeatedTypesError<XAnnot,XAnnot>',
        'A type was specified more than once.',
        COMMON_DEFINITIONS,
        source,
        locals())

def test_error_repeated_type_with_different_annotation_ok():
    source = '''
        struct X {};

        InstantiateType(fruit::NormalizedComponent<XAnnot1, XAnnot2>)
        '''
    expect_success(
        COMMON_DEFINITIONS,
        source)

@params('X', 'fruit::Annotated<Annotation1, X>')
def test_error_type_required_and_provided(XAnnot):
    source = '''
        struct X {};

        InstantiateType(fruit::NormalizedComponent<fruit::Required<XAnnot>, XAnnot>)
        '''
    expect_compile_error(
        'RepeatedTypesError<XAnnot, XAnnot>',
        'A type was specified more than once.',
        COMMON_DEFINITIONS,
        source,
        locals())

if __name__ == '__main__':
    import nose2
    nose2.main()
