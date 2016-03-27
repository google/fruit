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
#include <map>
#include <random>
#include <cassert>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;


constexpr int num_types_per_component = 1;
constexpr int num_components_with_no_deps = 10;
constexpr int num_components_with_deps    = 90;
constexpr int num_deps = 10;

static_assert(num_components_with_no_deps >= num_deps, "Too few components with no deps.");

// This is a constant so that we always generate the same file (=> benchmark more repeatable).
unsigned seed = 42;
std::default_random_engine generator(seed);

string getHeaderName(int n) {
  ostringstream stream;
  stream << "component" << n << ".h";
  return stream.str();
}

string getSourceName(int n) {
  ostringstream stream;
  stream << "component" << n << ".cpp";
  return stream.str();
}

string getObjectName(int n) {
  ostringstream stream;
  stream << "component" << n << ".o";
  return stream.str();
}

void printComponentArgs(int id, ostream& str) {
  str << "<" << endl;
  for (int i = 0; i < num_types_per_component; ++i) {
    str << "Interface" << id << "_" << i;
    if (i == num_types_per_component - 1) {
      str << endl;
    } else {
      str << "," << endl;
    }
  }
  str << ">";
}

void add_node(int n, set<int> deps) {
  std::string headerName = getHeaderName(n);
  std::string sourceName = getSourceName(n);
  
  ofstream headerFile(headerName);
  headerFile << "#include <fruit/fruit.h>" << endl << endl;
  headerFile << "#ifndef COMPONENT" << n << "_H" << endl;
  headerFile << "#define COMPONENT" << n << "_H" << endl;
  for (int i = 0; i < num_types_per_component; ++i) {
    headerFile << "struct Interface" << n << "_" << i << " { virtual ~Interface" << n << "_" << i << "() = default; };" << endl;
  }
  headerFile << "fruit::Component" << endl;
  printComponentArgs(n, headerFile);
  headerFile << " getComponent" << n << "();" << endl;
  headerFile << "#endif // COMPONENT" << n << "_H" << endl;
  
  ofstream sourceFile(sourceName);
  sourceFile << "#include \"" << headerName << "\"" << endl << endl;
  for (auto dep : deps) {
    sourceFile << "#include \"" << getHeaderName(dep) << "\"" << endl;
  }
  for (int i = 0; i < num_types_per_component; ++i) {
    sourceFile << "struct X" << n << "_" << i << " : public Interface" << n << "_" << i << " { INJECT(X" << n << "_" << i << "(";
    for (auto dep = deps.begin(), dep_end = deps.end(); dep != dep_end; ++dep) {
      if (dep != deps.begin()) {
        sourceFile << ", ";
      }
      sourceFile << "Interface" << *dep << "_" << i << "*";
    }
    sourceFile << ")) {}" << endl;
    sourceFile << "virtual ~X" << n << "_" << i << "() = default;" << endl;
    sourceFile << "};" << endl;
  }
  sourceFile << "fruit::Component" << endl;
  printComponentArgs(n, sourceFile);
  sourceFile << " getComponent" << n << "() {" << endl;
  sourceFile << "  return fruit::createComponent()" << endl;
  for (auto dep : deps) {
    sourceFile << "      .install(getComponent" << dep << "())" << endl;
  }
  for (int i = 0; i < num_types_per_component; ++i) {
    sourceFile << "      .bind<Interface" << n << "_" << i << ", " << "X" << n << "_" << i << ">()" << endl;
  }
  sourceFile << ";" << endl;
  sourceFile << "}" << endl;
}

void cover(int i, vector<bool>& covered, const map<int, set<int>>& deps_map) {
  if (covered[i]) {
    return;
  }
  covered[i] = true;
  for (int dep : deps_map.at(i)) {
    cover(dep, covered, deps_map);
  }
}

int main(int argc, char* argv[]) {
  
  if (argc != 4) {
    cout << "Invalid invocation: " << argv[0];
    for (int i = 1; i < argc; i++) {
      cout << " " << argv[i];
    }
    cout << endl;
    cout << "Usage: " << argv[0] << " /path/to/compiler path/to/fruit/sources/root path/to/fruit/build/root" << endl;
    return 1;
  }
  
  int num_used_ids = 0;
  
  map<int, set<int>> deps_map;

  for (int i = 0; i < num_components_with_no_deps; i++) {
    int id = num_used_ids++;
    add_node(id, set<int>{});
    deps_map[id] = {};
  }
  
  // Then the rest have num_deps deps, chosen (pseudo-)randomly from the previous components with no deps, plus the previous
  // component with deps (if any).
  for (int i = 0; i < num_components_with_deps; i++) {
    int current_dep_id = num_used_ids++;
    set<int> deps;
    if (i != 0) {
      deps.insert(i - 1);
    }
    std::uniform_int_distribution<int> distribution(0, num_components_with_no_deps - 1);
    while (deps.size() != num_deps) {
      int dep = distribution(generator);
      deps.insert(dep);
    }
    add_node(current_dep_id, deps);
    deps_map[current_dep_id] = deps;
  }
  
  set<int> toplevel_types;
  vector<bool> covered(num_used_ids, false);
  for (int i = num_used_ids - 1; i >= 0; --i) {
    if (!covered[i]) {
      toplevel_types.insert(i);
      cover(i, covered, deps_map);
    }
  }
  
  int toplevel_component = num_used_ids++;
  add_node(toplevel_component, toplevel_types);
  
  ofstream mainFile("main.cpp");
  mainFile << "#include \"" << getHeaderName(toplevel_component) << "\"" << endl;
  mainFile << "#include <ctime>" << endl;
  mainFile << "#include <iostream>" << endl;
  mainFile << "#include <cstdlib>" << endl;
  mainFile << "#include <iomanip>" << endl;
  mainFile << "#include <chrono>" << endl;
  mainFile << "using namespace std;" << endl;

  mainFile << "int main(int argc, char* argv[]) {" << endl;
  mainFile << "if (argc != 2) {" << endl;
  mainFile << "  std::cout << \"Need to specify num_loops as argument.\" << std::endl;" << endl;  
  mainFile << "  exit(1);" << endl;  
  mainFile << "}" << endl;  
  mainFile << "size_t num_loops = std::atoi(argv[1]);" << endl;
  mainFile << "double componentCreationTime = 0;" << endl;
  mainFile << "double componentNormalizationTime = 0;" << endl;
  //mainFile << "double componentCopyTime = 0;" << endl;
  //mainFile << "double injectorCreationTime = 0;" << endl;
  //mainFile << "double injectionTime = 0;" << endl;
  //mainFile << "double destructionTime = 0;" << endl;
  mainFile << "double perRequestTime = 0;" << endl;
  mainFile << "std::chrono::high_resolution_clock::time_point start_time;" << endl;
  
  mainFile << "for (size_t i = 0; i < 1 + num_loops/100; i++) {" << endl;
  mainFile << "start_time = std::chrono::high_resolution_clock::now();" << endl;
  mainFile << "fruit::Component";
  printComponentArgs(toplevel_component, mainFile);
  mainFile << " component(getComponent" << toplevel_component << "());" << endl;
  mainFile << "componentCreationTime += 1000000*std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();" << endl;
  mainFile << "start_time = std::chrono::high_resolution_clock::now();" << endl;
  mainFile << "fruit::NormalizedComponent";
  printComponentArgs(toplevel_component, mainFile);
  mainFile << " normalizedComponent(std::move(component));" << endl;
  mainFile << "componentNormalizationTime += 1000000*std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();" << endl;
  mainFile << "}" << endl;


  mainFile << "fruit::Component";
  printComponentArgs(toplevel_component, mainFile);
  mainFile << " component(getComponent" << toplevel_component << "());" << endl;
  mainFile << "fruit::NormalizedComponent";
  printComponentArgs(toplevel_component, mainFile);
  mainFile << " normalizedComponent(std::move(component));" << endl;
  
  mainFile << "start_time = std::chrono::high_resolution_clock::now();" << endl;
  mainFile << "for (size_t i = 0; i < num_loops; i++) {" << endl;
  mainFile << "{" << endl;
  // mainFile << "start_time = std::chrono::high_resolution_clock::now();" << endl;
  mainFile << "fruit::Injector";
  printComponentArgs(toplevel_component, mainFile);
  mainFile << " injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));" << endl;
  // mainFile << "injectorCreationTime += 1000000*std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();" << endl;
  // mainFile << "start_time = std::chrono::high_resolution_clock::now();" << endl;
  for (int i = 0; i < num_types_per_component; ++i) {
    mainFile << "injector.get<Interface" << toplevel_component << "_" << i << "*>();" << endl;
  }
  // mainFile << "injectionTime += 1000000*std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();" << endl;
  // mainFile << "start_time = std::chrono::high_resolution_clock::now();" << endl;
  mainFile << "}" << endl;
  // mainFile << "destructionTime += 1000000*std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();" << endl;
  mainFile << "}" << endl;
  mainFile << "perRequestTime += 1000000*std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();" << endl;


  mainFile << "std::cout << std::fixed;" << endl;
  mainFile << "std::cout << std::setprecision(2);" << endl;
  mainFile << "std::cout << \"componentCreationTime      = \" << componentCreationTime * 100 / num_loops << std::endl;" << endl;
  mainFile << "std::cout << \"componentNormalizationTime = \" << componentNormalizationTime * 100 / num_loops << std::endl;" << endl;
  mainFile << "std::cout << \"Total for setup            = \" << (componentCreationTime + componentNormalizationTime) * 100 / num_loops << std::endl;" << endl;
  //mainFile << "std::cout << \"injectorCreationTime       = \" << injectorCreationTime / num_loops << std::endl;" << endl;
  //mainFile << "std::cout << \"injectionTime              = \" << injectionTime / num_loops << std::endl;" << endl;
  //mainFile << "std::cout << \"destructionTime            = \" << destructionTime / num_loops << std::endl;" << endl;
  mainFile << "std::cout << \"Total per request          = \" << perRequestTime / num_loops << std::endl;" << endl;
  mainFile << "return 0;" << endl;
  mainFile << "}" << endl;
  
  const string compiler = string(argv[1]) + " -std=c++11 -O2 -g -W -Wall -Werror -DNDEBUG -ftemplate-depth=1000 -I" + argv[2] + "/include -I" + argv[3] + "/include";
  vector<string> fruit_srcs = {"component_storage", "demangle_type_name", "injector_storage", "normalized_component_storage", "semistatic_map"};
  
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
