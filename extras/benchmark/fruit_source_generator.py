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


class FruitSourceGenerator:
    def __init__(self, use_fruit_2_x_syntax=False):
        self.use_fruit_2_x_syntax = use_fruit_2_x_syntax

    def _get_component_type(self, component_index):
        if self.use_fruit_2_x_syntax:
            return 'const fruit::Component<Interface{component_index}>&'.format(**locals())
        else:
            return 'fruit::Component<Interface{component_index}>'.format(**locals())

    def generate_component_header(self, component_index):
        component_type = self._get_component_type(component_index)
        template = """
#ifndef COMPONENT{component_index}_H
#define COMPONENT{component_index}_H

#include <fruit/fruit.h>

struct Interface{component_index} {{
  virtual ~Interface{component_index}() = default;
}};

{component_type} getComponent{component_index}();

#endif // COMPONENT{component_index}_H
"""
        return template.format(**locals())

    def generate_component_source(self, component_index, deps):
        include_directives = ''.join(['#include "component%s.h"\n' % index for index in deps + [component_index]])

        component_deps = ', '.join(['std::shared_ptr<Interface%s>' % dep for dep in deps])

        if self.use_fruit_2_x_syntax:
            install_expressions = ''.join(['        .install(getComponent%s())\n' % dep for dep in deps])
        else:
            install_expressions = ''.join(['        .install(getComponent%s)\n' % dep for dep in deps])

        component_type = self._get_component_type(component_index)

        template = """
{include_directives}

struct X{component_index} : public Interface{component_index} {{
INJECT(X{component_index}({component_deps})) {{}}

virtual ~X{component_index}() = default;
}};

"""

        if self.use_fruit_2_x_syntax:
            template += """
{component_type} getComponent{component_index}() {{
    static {component_type} comp = fruit::createComponent(){install_expressions}
        .bind<Interface{component_index}, X{component_index}>();
    return comp;
}}
"""
        else:
            template += """
{component_type} getComponent{component_index}() {{
    return fruit::createComponent(){install_expressions}
        .bind<Interface{component_index}, X{component_index}>();
}}
"""

        return template.format(**locals())

    def generate_main(self, toplevel_component):
        if self.use_fruit_2_x_syntax:
            return self.generate_main_with_fruit_2_x_syntax(toplevel_component)
        else:
            return self.generate_main_with_fruit_3_x_syntax(toplevel_component)

    def generate_main_with_fruit_2_x_syntax(self, toplevel_component):
        template = """
#include "component{toplevel_component}.h"

#include <ctime>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <chrono>

using namespace std;

int main(int argc, char* argv[]) {{
  if (argc != 2) {{
    std::cout << "Need to specify num_loops as argument." << std::endl;
    exit(1);
  }}
  size_t num_loops = std::atoi(argv[1]);
  double componentCreationTime = 0;
  double componentNormalizationTime = 0;
  std::chrono::high_resolution_clock::time_point start_time;
    
  for (size_t i = 0; i < 1 + num_loops/100; i++) {{
    start_time = std::chrono::high_resolution_clock::now();
    fruit::Component<Interface{toplevel_component}> component(getComponent{toplevel_component}());
    componentCreationTime += std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();
    start_time = std::chrono::high_resolution_clock::now();
    fruit::NormalizedComponent<Interface{toplevel_component}> normalizedComponent(std::move(component));
    componentNormalizationTime += std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();
  }}

  start_time = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < 1 + num_loops/100; i++) {{
    fruit::Injector<Interface{toplevel_component}> injector(getComponent{toplevel_component}());
    injector.get<std::shared_ptr<Interface{toplevel_component}>>();
  }}
  double fullInjectionTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();  

  // The cast to Component<Interface{toplevel_component}> is needed for Fruit<2.1.0, where the constructor of
  // NormalizedComponent only accepted a Component&&.
  fruit::NormalizedComponent<Interface{toplevel_component}> normalizedComponent{{fruit::Component<Interface{toplevel_component}>{{getComponent{toplevel_component}()}}}};
    
  start_time = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < num_loops; i++) {{
    fruit::Injector<Interface{toplevel_component}> injector(normalizedComponent, fruit::Component<>(fruit::createComponent()));
    injector.get<std::shared_ptr<Interface{toplevel_component}>>();
  }}
  double perRequestTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();

  std::cout << std::fixed;
  std::cout << std::setprecision(15);
  std::cout << "componentNormalizationTime = " << componentNormalizationTime * 100 / num_loops << std::endl;
  std::cout << "Total for setup            = " << (componentCreationTime + componentNormalizationTime) * 100 / num_loops << std::endl;
  std::cout << "Full injection time        = " << fullInjectionTime * 100 / num_loops << std::endl;
  std::cout << "Total per request          = " << perRequestTime / num_loops << std::endl;
  return 0;
}}
    """
        return template.format(**locals())

    def generate_main_with_fruit_3_x_syntax(self, toplevel_component):
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
  
  std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < 1 + num_loops/100; i++) {{
    fruit::NormalizedComponent<Interface{toplevel_component}> normalizedComponent(getComponent{toplevel_component});
    (void)normalizedComponent;
  }}
  double componentNormalizationTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();

  start_time = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < 1 + num_loops/100; i++) {{
    fruit::Injector<Interface{toplevel_component}> injector(getComponent{toplevel_component});
    injector.get<std::shared_ptr<Interface{toplevel_component}>>();
  }}
  double fullInjectionTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();  

  fruit::NormalizedComponent<Interface{toplevel_component}> normalizedComponent(getComponent{toplevel_component});
    
  start_time = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < num_loops; i++) {{
    fruit::Injector<Interface{toplevel_component}> injector(normalizedComponent, getEmptyComponent);
    injector.get<std::shared_ptr<Interface{toplevel_component}>>();
  }}
  double perRequestTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count();

  std::cout << std::fixed;
  std::cout << std::setprecision(15);
  std::cout << "componentNormalizationTime = " << componentNormalizationTime * 100 / num_loops << std::endl;
  std::cout << "Total for setup            = " << componentNormalizationTime * 100 / num_loops << std::endl;
  std::cout << "Full injection time        = " << fullInjectionTime * 100 / num_loops << std::endl;
  std::cout << "Total per request          = " << perRequestTime / num_loops << std::endl;
  return 0;
}}
    """
        return template.format(**locals())
