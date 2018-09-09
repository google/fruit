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

def generate_files(injection_graph, generate_runtime_bench_code):
    file_content_by_name = dict()

    for node_id in injection_graph.nodes_iter():
        deps = injection_graph.successors(node_id)
        file_content_by_name['component%s.h' % node_id] = _generate_component_header(node_id, deps)
        file_content_by_name['component%s.cpp' % node_id] = _generate_component_source(node_id, deps)

    [toplevel_node] = [node_id
                       for node_id in injection_graph.nodes_iter()
                       if not injection_graph.predecessors(node_id)]
    file_content_by_name['main.cpp'] = _generate_main(injection_graph, toplevel_node, generate_runtime_bench_code)

    return file_content_by_name

def _generate_component_header(component_index, deps):
    fields = ''.join(['std::shared_ptr<Interface%s> x%s;\n' % (dep, dep)
                      for dep in deps])
    component_deps = ''.join([', std::shared_ptr<Interface%s>' % dep for dep in deps])

    include_directives = ''.join(['#include "component%s.h"\n' % index for index in deps])

    template = """
#ifndef COMPONENT{component_index}_H
#define COMPONENT{component_index}_H

#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped_scope.hpp>
#include <memory>

// Example include that the code might use
#include <vector>

namespace di = boost::di;

{include_directives}

struct Interface{component_index} {{
    virtual ~Interface{component_index}() = default;
}};

struct X{component_index} : public Interface{component_index} {{
    {fields}
    
    BOOST_DI_INJECT(X{component_index}{component_deps});
    
    virtual ~X{component_index}() = default;
}};

auto x{component_index}Component = [] {{
    return di::make_injector(di::bind<Interface{component_index}>().to<X{component_index}>().in(di::extension::scoped));
}};

#endif // COMPONENT{component_index}_H
"""
    return template.format(**locals())

def _generate_component_source(component_index, deps):
    param_initializers = ', '.join('x%s(x%s)' % (dep, dep)
                                   for dep in deps)
    if param_initializers:
        param_initializers = ': ' + param_initializers
    component_deps = ', '.join('std::shared_ptr<Interface%s> x%s' % (dep, dep)
                               for dep in deps)

    template = """
#include "component{component_index}.h"

X{component_index}::X{component_index}({component_deps}) 
    {param_initializers} {{
}}
"""
    return template.format(**locals())

def _generate_main(injection_graph, toplevel_component, generate_runtime_bench_code):
    include_directives = ''.join('#include "component%s.h"\n' % index
                                 for index in injection_graph.nodes_iter())

    injector_params = ', '.join('x%sComponent()' % index
                                for index in injection_graph.nodes_iter())

    if generate_runtime_bench_code:
        template = """
{include_directives}

#include "component{toplevel_component}.h"
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <chrono>

using namespace std;

void f() {{
  auto injector = di::make_injector({injector_params});
  injector.create<std::shared_ptr<Interface{toplevel_component}>>();
}}

int main(int argc, char* argv[]) {{
  if (argc != 2) {{
    std::cout << "Need to specify num_loops as argument." << std::endl;
    exit(1);
  }}
  size_t num_loops = std::atoi(argv[1]);
  double perRequestTime = 0;
  std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < num_loops; i++) {{
    f();
  }}
  perRequestTime += std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();
  std::cout << std::fixed;
  std::cout << std::setprecision(15);
  std::cout << "Total for setup            = " << 0 << std::endl;
  std::cout << "Full injection time        = " << perRequestTime / num_loops << std::endl;
  std::cout << "Total per request          = " << perRequestTime / num_loops << std::endl;
  return 0;
}}
"""
    else:
        template = """
{include_directives}

#include "component{toplevel_component}.h"

#include <iostream>

int main() {{
  auto injector = di::make_injector({injector_params});
  injector.create<std::shared_ptr<Interface{toplevel_component}>>();
  std::cout << "Hello, world" << std::endl;
  return 0;
}}
"""
    return template.format(**locals())
