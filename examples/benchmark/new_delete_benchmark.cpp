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

#include <ctime>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

constexpr size_t num_allocations = 100;

struct I {
  virtual ~I() = default;
};

struct C : public I {
  virtual ~C() = default;
};

int main() {
  size_t num_loops;
  cin >> num_loops;
  
  //size_t newTime = 0;
  //size_t deleteTime = 0;
  
  clock_t start_time;
  
  vector<C*> v(num_allocations, nullptr);
  
  start_time = clock();
  for (size_t i = 0; i < num_loops; i++) {
    for (size_t j = 0; j < num_allocations; ++j) {
      v[j] = new C();
    }
    for (size_t j = num_allocations; j > 0; --j) {
      delete v[j-1];
    }
  }
  size_t totalTime = clock() - start_time;
  
  std::cout << std::fixed;
  std::cout << std::setprecision(2);
  //std::cout << "Time for new    = " << newTime * 1.0 / num_loops << std::endl;
  //std::cout << "Time for delete = " << deleteTime * 1.0 / num_loops << std::endl;
  std::cout << "Total           = " << totalTime * 1.0 / num_loops << std::endl;
  
  return 0;
}
