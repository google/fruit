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

COMMON_DEFINITIONS = '''
    #include "test_common.h"
    '''

def test_install_component_swap_optimization():
    source = '''
        fruit::Component<int, float, double, unsigned> getParentComponent() {
          static int x = 0;
          static float y = 0;
          static double z = 0;
          static unsigned u = 0;
          return fruit::createComponent()
            .bindInstance(x)
            .bindInstance(y)
            .bindInstance(z)
            .bindInstance(u);
        }
        
        fruit::Component<std::vector<int>, std::vector<float>, std::vector<double>, std::vector<unsigned>> getParentComponent2() {
          static std::vector<int> x;
          static std::vector<float> y;
          static std::vector<double> z;
          static std::vector<unsigned> u;
          return fruit::createComponent()
            .bindInstance(x)
            .bindInstance(y)
            .bindInstance(z)
            .bindInstance(u)
            .addInstanceMultibinding(x);
        }
        
        fruit::Component<int, float, double, unsigned> getComponent() {
          return fruit::createComponent()
            .install(getParentComponent())
            .install(getParentComponent2());
        }
        
        int main() {
          fruit::Injector<int, float, double, unsigned> injector(getComponent());
          injector.get<int>();
          
          return 0;
        }
    '''
    expect_success(COMMON_DEFINITIONS, source, ignore_deprecation_warnings=True)

if __name__== '__main__':
    main(__file__)
