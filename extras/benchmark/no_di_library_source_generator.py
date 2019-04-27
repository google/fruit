# Copyright 2016 Google Inc. All Rights Reserved.
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
import itertools
import networkx as nx

def generate_files(injection_graph, use_new_delete, use_interfaces, generate_runtime_bench_code):
    file_content_by_name = dict()

    for node_id in injection_graph.nodes_iter():
        deps = injection_graph.successors(node_id)
        if use_interfaces:
            file_content_by_name['class%s_interface.h' % node_id] = _generate_class_interface_header(node_id)
            file_content_by_name['class%s.h' % node_id] = _generate_class_header_with_interfaces(node_id, deps)
            file_content_by_name['class%s.cpp' % node_id] = _generate_class_cpp_file_with_interfaces(node_id, deps)
        else:
            file_content_by_name['class%s.h' % node_id] = _generate_class_header_without_interfaces(node_id, deps)
            file_content_by_name['class%s.cpp' % node_id] = _generate_class_cpp_file_without_interfaces(node_id, deps)

    file_content_by_name['main.cpp'] = _generate_main(injection_graph, use_interfaces, use_new_delete, generate_runtime_bench_code)

    return file_content_by_name

def _generate_class_interface_header(class_index):
    template = """
#ifndef CLASS{class_index}_INTERFACE_H
#define CLASS{class_index}_INTERFACE_H

// Example include that the code might use
#include <vector>

struct Interface{class_index} {{
  virtual void foo() = 0;
  virtual ~Interface{class_index}();
}};

#endif // CLASS{class_index}_INTERFACE_H
"""
    return template.format(**locals())

def _generate_class_header_with_interfaces(class_index, deps):
    include_directives = ''.join('#include "class%s_interface.h"\n' % index
                                 for index in itertools.chain(deps, (class_index,)))
    fields = ''.join('Interface%s& x%s;\n' % (index, index)
                     for index in deps)
    constructor_params = ', '.join('Interface%s& x%s' % (index, index)
                                   for index in deps)

    template = """
#ifndef CLASS{class_index}_H
#define CLASS{class_index}_H

{include_directives}

struct Class{class_index} : public Interface{class_index} {{
  {fields}
  Class{class_index}({constructor_params});

  virtual void foo() override;

  virtual ~Class{class_index}();
}};

#endif // CLASS{class_index}_H
"""
    return template.format(**locals())

def _generate_class_header_without_interfaces(class_index, deps):
    include_directives = ''.join('#include "class%s.h"\n' % index
                                 for index in deps)
    fields = ''.join('Class%s& x%s;\n' % (index, index)
                     for index in deps)
    constructor_params = ', '.join('Class%s& x%s' % (index, index)
                                   for index in deps)

    template = """
#ifndef CLASS{class_index}_H
#define CLASS{class_index}_H

// Example include that the code might use
#include <vector>

{include_directives}

struct Class{class_index} {{
  {fields}
  Class{class_index}({constructor_params});
}};

#endif // CLASS{class_index}_H
"""
    return template.format(**locals())

def _generate_class_cpp_file_with_interfaces(class_index, deps):
    constructor_params = ', '.join('Interface%s& x%s' % (index, index)
                                   for index in deps)
    field_initializers = ', '.join('x%s(x%s)' % (index, index)
                                   for index in deps)
    if field_initializers:
        field_initializers = ': ' + field_initializers

    template = """
#include "class{class_index}.h"

Interface{class_index}::~Interface{class_index}() {{
}}

Class{class_index}::Class{class_index}({constructor_params})
  {field_initializers} {{
}}

void Class{class_index}::foo() {{
}}

Class{class_index}::~Class{class_index}() {{
}}
"""
    return template.format(**locals())

def _generate_class_cpp_file_without_interfaces(class_index, deps):
    constructor_params = ', '.join('Class%s& x%s' % (index, index)
                                   for index in deps)
    field_initializers = ', '.join('x%s(x%s)' % (index, index)
                                   for index in deps)
    if field_initializers:
        field_initializers = ': ' + field_initializers

    template = """
#include "class{class_index}.h"

Class{class_index}::Class{class_index}({constructor_params})
  {field_initializers} {{
}}
"""
    return template.format(**locals())


def _generate_main(injection_graph, use_interfaces, use_new_delete, generate_runtime_bench_code):
    [toplevel_class_index] = [node_id
                              for node_id in injection_graph.nodes_iter()
                              if not injection_graph.predecessors(node_id)]

    if use_interfaces:
        include_directives = ''.join('#include "class%s.h"\n' % index
                                     for index in injection_graph.nodes_iter())
    else:
        include_directives = '#include "class%s.h"\n' % toplevel_class_index

    if use_new_delete:
        instance_creations = ''.join('std::unique_ptr<Class%s> x%s(new Class%s(%s));\n' % (class_index,
                                                                                           class_index,
                                                                                           class_index,
                                                                                           ', '.join('*x%s' % dep_index
                                                                                                     for dep_index in injection_graph.successors(class_index)))
                                     for class_index in reversed(list(nx.topological_sort(injection_graph))))
    else:
        instance_creations = ''.join('Class%s x%s{%s};\n' % (class_index,
                                                             class_index,
                                                             ', '.join('x%s' % dep_index
                                                                       for dep_index in injection_graph.successors(class_index)))
                                     for class_index in reversed(list(nx.topological_sort(injection_graph))))

    void_casts = ''.join('(void) x%s;\n' % index
                         for index in injection_graph.nodes_iter())

    if generate_runtime_bench_code:
        template = """
{include_directives}

#include <ctime>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <chrono>

using namespace std;

void do_injection() {{
  {instance_creations}
  {void_casts}
}}

int main(int argc, char* argv[]) {{
  if (argc != 2) {{
    std::cout << "Need to specify num_loops as argument." << std::endl;
    exit(1);
  }}
  size_t num_loops = std::atoi(argv[1]);
  std::chrono::high_resolution_clock::time_point start_time;

  start_time = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < 1 + num_loops/100; i++) {{
    do_injection();
  }}
  double fullInjectionTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();  

  std::cout << std::fixed;
  std::cout << std::setprecision(15);
  std::cout << "Total per request          = " << fullInjectionTime * 100 / num_loops << std::endl;
  return 0;
}}
"""
    else:
        template = """
{include_directives}

#include <memory>
#include <iostream>

int main() {{
  {instance_creations}
  {void_casts}
  std::cout << "Hello, world" << std::endl;
  return 0;
}}
"""
    return template.format(**locals())
