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

#include <iostream>
#include <set>
#include <random>
#include <cassert>
#include <chrono>

using namespace std;

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator(seed);

void add_node(int n, set<int> deps) {
  cout << "struct X" << n << " { INJECT(X" << n << "(";
  for (auto i = deps.begin(), i_end = deps.end(); i != i_end; ++i) {
    if (i != deps.begin()) {
      cout << ", ";
    }
    cout << "X" << *i;
  }
  cout << ")) {} };" << endl;
}

void print_type_list(int n) {
  for (int i = 0; i < n; i++) {
    if (i != 0) {
      cout << ", ";
    }
    cout << endl;
    cout << "X" << i;
  }
  cout << endl;
}

set<int> get_random_set(int N, int desired_size) {
  assert(desired_size <= N);
  set<int> result;
  std::uniform_int_distribution<int> distribution(0, N - 1);
  while (result.size() != desired_size) {
    result.insert(distribution(generator));
  }
  return result;
}

int main() {
  constexpr int num_types_with_no_deps = 100;
  constexpr int num_types_with_deps = 100;
  constexpr int num_deps = 10;
  constexpr int num_loops = 1000;
  static_assert(num_types_with_no_deps >= num_deps, "Not enough types with no deps");

  cout << "#include <fruit/fruit.h>" << endl << endl;;
  
  for (int i = 0; i < num_types_with_no_deps; i++) {
    add_node(i, {});
  }
  
  for (int i = 0; i < num_types_with_deps; i++) {
    add_node(i + num_types_with_no_deps, get_random_set(num_types_with_no_deps + i, num_deps));
  }

  cout << "fruit::Component<";
  print_type_list(num_types_with_no_deps + num_types_with_deps);
  cout << "> getComponent() { return fruit::createComponent(); }" << endl;
  cout << "int main() {" << endl;
  cout << "for (int i = 0; i < " << num_loops << "; i++) {" << endl;
  cout << "fruit::Injector<" << endl;
  print_type_list(num_types_with_no_deps + num_types_with_deps);
  cout << "> injector(getComponent());" << endl;
  for (int i = 0; i < num_types_with_no_deps + num_types_with_deps; i++) {
    cout << "injector.get<X" << i << "*>();" << endl;
  }
  cout << "}" << endl;
  cout << "return 0;" << endl;
  cout << "}" << endl;
  
  return 0;
}
