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
from typing import List

import networkx as nx


def generate_files(injection_graph: nx.DiGraph, generate_runtime_bench_code: bool, use_normalized_component: bool=False):
    if use_normalized_component:
        assert not generate_runtime_bench_code

    file_content_by_name = dict()

    for node_id in injection_graph.nodes:
        file_content_by_name['component%s.h' % node_id] = _generate_component_header(node_id)
        file_content_by_name['component%s.cpp' % node_id] = _generate_component_source(node_id, list(injection_graph.successors(node_id)))

    [toplevel_node] = [node_id
                       for node_id in injection_graph.nodes
                       if not any(True for p in injection_graph.predecessors(node_id))]
    file_content_by_name['main.cpp'] = _generate_main(toplevel_node, generate_runtime_bench_code)

    return file_content_by_name

def _get_component_type(component_index: int):
    return 'fruit::Component<Interface{component_index}>'.format(**locals())

def _generate_component_header(component_index: int):
    component_type = _get_component_type(component_index)
    template = """
#ifndef COMPONENT{component_index}_H
#define COMPONENT{component_index}_H

#include <fruit/fruit.h>

// Example include that the code might use
#include <vector>

struct Interface{component_index} {{
  virtual ~Interface{component_index}() = default;
}};

{component_type} getComponent{component_index}();

#endif // COMPONENT{component_index}_H
"""
    return template.format(**locals())

def _generate_component_source(component_index: int, deps: List[int]):
    include_directives = ''.join(['#include "component%s.h"\n' % index for index in deps + [component_index]])

    fields = ''.join(['Interface%s& x%s;\n' % (dep, dep)
                      for dep in deps])

    component_deps = ', '.join(['Interface%s& x%s' % (dep, dep)
                                for dep in deps])
    param_initializers = ', '.join('x%s(x%s)' % (dep, dep)
                                   for dep in deps)
    if param_initializers:
        param_initializers = ': ' + param_initializers

    install_expressions = ''.join(['        .install(getComponent%s)\n' % dep for dep in deps])

    component_type = _get_component_type(component_index)

    template = """
{include_directives}

namespace {{
struct X{component_index} : public Interface{component_index} {{
  {fields}

  INJECT(X{component_index}({component_deps})) {param_initializers} {{}}

  ~X{component_index}() override = default;
}};
}}

"""

    template += """
{component_type} getComponent{component_index}() {{
    return fruit::createComponent(){install_expressions}
        .bind<Interface{component_index}, X{component_index}>();
}}
"""

    return template.format(**locals())

def _generate_main(toplevel_component: int, generate_runtime_bench_code: bool):
    if generate_runtime_bench_code:
        template = """
#include "component{toplevel_component}.h"

#include <ctime>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <chrono>

using namespace std;

fruit::Component<> getEmptyComponent() {{
  return fruit::createComponent();
}}

int main(int argc, char* argv[]) {{
  if (argc != 2) {{
    std::cout << "Need to specify num_loops as argument." << std::endl;
    exit(1);
  }}
  size_t num_loops = std::atoi(argv[1]);
  
  fruit::NormalizedComponent<Interface{toplevel_component}> normalizedComponent(getComponent{toplevel_component});
    
  std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < num_loops; i++) {{
    fruit::Injector<Interface{toplevel_component}> injector(normalizedComponent, getEmptyComponent);
    injector.get<std::shared_ptr<Interface{toplevel_component}>>();
  }}
  double perRequestTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();

  std::cout << std::fixed;
  std::cout << std::setprecision(15);
  std::cout << "Total per request          = " << perRequestTime / num_loops << std::endl;
  return 0;
}}
    """
    else:
        template = """
#include "component{toplevel_component}.h"

#include <iostream>

fruit::Component<> getEmptyComponent() {{
  return fruit::createComponent();
}}

int main(void) {{
  fruit::NormalizedComponent<Interface{toplevel_component}> normalizedComponent(getComponent{toplevel_component});
  fruit::Injector<Interface{toplevel_component}> injector(normalizedComponent, getEmptyComponent);
  injector.get<std::shared_ptr<Interface{toplevel_component}>>();
  std::cout << "Hello, world" << std::endl;
  return 0;
}}
    """

    return template.format(**locals())
