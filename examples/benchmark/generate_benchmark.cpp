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
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

// This is a constant so that we always generate the same file (=> benchmark more repeatable).
unsigned seed = 42;
std::default_random_engine generator(seed);

string getHeaderName(int n) {
  ostringstream stream;
  stream << "X" << n << ".h";
  return stream.str();
}

string getSourceName(int n) {
  ostringstream stream;
  stream << "X" << n << ".cpp";
  return stream.str();
}

string getObjectName(int n) {
  ostringstream stream;
  stream << "X" << n << ".o";
  return stream.str();
}

void add_node(int n, set<int> deps) {
  std::string headerName = getHeaderName(n);
  std::string sourceName = getSourceName(n);
  
  ofstream headerFile(headerName);
  headerFile << "#include <fruit/fruit.h>" << endl << endl;
  headerFile << "#ifndef X" << n << "_H" << endl;
  headerFile << "#define X" << n << "_H" << endl;
  for (auto dep : deps) {
    headerFile << "#include \"" << getHeaderName(dep) << "\"" << endl;
  }
  headerFile << "struct X" << n << " { INJECT(X" << n << "(";
  for (auto i = deps.begin(), i_end = deps.end(); i != i_end; ++i) {
    if (i != deps.begin()) {
      headerFile << ", ";
    }
    headerFile << "const X" << *i << "&";
  }
  headerFile << ")) {} };" << endl;
  headerFile << "fruit::Component<X" << n << "> getX" << n << "Component();" << endl;
  headerFile << "#endif // X" << n << "_H" << endl;
  
  ofstream sourceFile(sourceName);
  sourceFile << "#include \"" << headerName << "\"" << endl << endl;
  sourceFile << "fruit::Component<X" << n << "> getX" << n << "Component() {" << endl;
  sourceFile << "  return fruit::createComponent()" << endl;
  for (auto dep : deps) {
    sourceFile << "      .install(getX" << dep << "Component())" << endl;
  }
  sourceFile << "  ;" << endl;
  sourceFile << "}" << endl;
}

set<int> get_random_set(set<int>& pool, size_t desired_size) {
  assert(desired_size <= pool.size());
  set<int> result;
  while (result.size() != desired_size) {
    std::uniform_int_distribution<int> distribution(0, pool.size() - 1);
    int i = distribution(generator);
    auto itr = pool.begin();
    std::advance(itr, i);
    result.insert(*itr);
    pool.erase(itr);
  }
  return result;
}

int main(int argc, char* argv[]) {
  
  if (argc != 3) {
    cout << "Invalid invocation: " << argv[0];
    for (int i = 1; i < argc; i++) {
      cout << " " << argv[i];
    }
    cout << endl;
    cout << "Usage: " << argv[0] << " /path/to/compiler path/to/fruit/sources/root" << endl;
    return 1;
  }
  
  constexpr int num_types_with_no_deps = 250;
  constexpr int num_types_with_deps = 30;
  constexpr int num_deps = 8;
  constexpr int num_loops = 20;
  static_assert(num_types_with_no_deps >= num_types_with_deps * num_deps + 1, "Not enough types with no deps");
  
  int num_used_ids = 0;

  set<int> toplevel_types;
  
  for (int i = 0; i < num_types_with_no_deps; i++) {
    int id = num_used_ids++;
    add_node(id, {});
    toplevel_types.insert(id);
  }
  
  for (int i = 0; i < num_types_with_deps; i++) {
    int current_dep_id = num_used_ids++;
    auto deps = get_random_set(toplevel_types, num_deps);
    toplevel_types.insert(current_dep_id);
    add_node(current_dep_id, deps);
  }
  
  int toplevel_component = num_used_ids++;
  add_node(toplevel_component, toplevel_types);
  
  ofstream mainFile("main.cpp");
  mainFile << "#include \"" << getHeaderName(toplevel_component) << "\"" << endl;
  mainFile << "#include <ctime>" << endl;
  mainFile << "#include <iostream>" << endl;
  mainFile << "using namespace std;" << endl;

  mainFile << "int main() {" << endl;
  mainFile << "clock_t start_time = clock();" << endl;
  mainFile << "for (int i = 0; i < " << num_loops << "; i++) {" << endl;
  mainFile << "fruit::Injector<X" << toplevel_component << "> injector(getX" << toplevel_component << "Component());" << endl;
  mainFile << "injector.get<X" << toplevel_component << "*>();" << endl;
  mainFile << "}" << endl;
  mainFile << "clock_t end_time = clock();" << endl;
  mainFile << "cout << (end_time - start_time) / " << num_loops << " << endl;" << endl;
  mainFile << "return 0;" << endl;
  mainFile << "}" << endl;
  
  const string compiler = string(argv[1]) + " -std=c++11 -O2 -g -W -Wall -Werror -DNDEBUG -ftemplate-depth=1000 -I" + argv[2] + "/include";
  vector<string> fruit_srcs = {"component_storage", "demangle_type_name", "injector_storage"};
  
  ofstream buildFile("build.sh");
  buildFile << "#!/bin/bash" << endl;
  for (int i = 0; i < num_used_ids; i++) {
    buildFile << compiler << " -c " << getSourceName(i) << " -o " << getObjectName(i) << " &" << endl;
    if (i % 20 == 0) {
      // Avoids having too many parallel processes.
      buildFile << "wait || exit 1" << endl;
    }
  }
  buildFile << compiler << " -c main.cpp -o main.o &" << endl;
  for (string s : fruit_srcs) {
    buildFile << compiler << " -c " << argv[2] << "/src/" << s << ".cpp -o " << s << ".o &" << endl;
  }
  buildFile << "wait" << endl;
  buildFile << compiler << " main.o ";
  for (string s : fruit_srcs) {
    buildFile << s << ".o ";
  }
  for (int i = 0; i < num_used_ids; i++) {
    buildFile << getObjectName(i) << " ";
  }
  buildFile << " -o main" << endl;
  
  return 0;
}
