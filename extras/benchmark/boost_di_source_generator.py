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


class BoostDiSourceGenerator:
    def generate_component_header(self, component_index):
        template = """
#ifndef COMPONENT{component_index}_H
#define COMPONENT{component_index}_H

#include <boost/di.hpp>
#include <memory>

namespace di = boost::di;

struct Interface{component_index} {{
    virtual ~Interface{component_index}() = default;
}};
di::injector<std::shared_ptr<Interface{component_index}>> getComponent{component_index}();

#endif // COMPONENT{component_index}_H
"""
        return template.format(**locals())

    def generate_component_source(self, component_index, deps):
        include_directives = ''.join(['#include "component%s.h"\n' % index for index in deps + [component_index]])

        component_deps = ''.join([', std::shared_ptr<Interface%s>' % dep for dep in deps])

        make_injector_params = ','.join(['\n        getComponent%s()' % dep for dep in deps]
                                        + ['\n        di::bind<Interface%s>().in(di::singleton).to<X%s>()' % (component_index, component_index)])

        template = """
{include_directives}

struct X{component_index} : public Interface{component_index} {{
    BOOST_DI_INJECT(X{component_index}{component_deps}) {{}}
    
    virtual ~X{component_index}() = default;
}};

di::injector<std::shared_ptr<Interface{component_index}>> getComponent{component_index}() {{
    return di::make_injector({make_injector_params});
}}
"""
        return template.format(**locals())

    def generate_main(self, toplevel_component):
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
  double perRequestTime = 0;
  std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < num_loops; i++) {{
    di::injector<std::shared_ptr<Interface{toplevel_component}>> injector(getComponent{toplevel_component}());
    injector.create<std::shared_ptr<Interface{toplevel_component}>>();
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
        return template.format(**locals())
