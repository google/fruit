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

#include <fruit/fruit.h>
#include "test_macros.h"
#include <vector>

using fruit::Component;
using fruit::Injector;
using namespace std;

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

fruit::Component<vector<int>, vector<float>, vector<double>, vector<unsigned>> getParentComponent2() {
  static vector<int> x;
  static vector<float> y;
  static vector<double> z;
  static vector<unsigned> u;
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
  
  Injector<int, float, double, unsigned> injector(getComponent());
  
  injector.get<int>();
  
  return 0;
}
