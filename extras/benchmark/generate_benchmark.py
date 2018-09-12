#!/usr/bin/env python3
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

import random
import os

import fruit_source_generator
import boost_di_source_generator
import no_di_library_source_generator
from makefile_generator import generate_makefile
import argparse
import networkx as nx


def generate_injection_graph(num_components_with_no_deps,
                             num_components_with_deps,
                             num_deps):
    injection_graph = nx.DiGraph()

    num_used_ids = 0
    is_toplevel = [True for i in range(0, num_components_with_no_deps + num_components_with_deps)]
    toplevel_components = set()
    for i in range(0, num_components_with_no_deps):
        id = num_used_ids
        num_used_ids += 1
        toplevel_components.add(id)

    # Then the rest have num_deps deps, chosen (pseudo-)randomly from the previous components.
    # The last few components depend more components with >1 deps, so that the last component transitively depends on
    # everything.
    for i in range(0, num_components_with_deps):
        deps = set()

        if len(toplevel_components) > (num_components_with_deps - 1 - i) * (num_deps - 1):
            # We need at least 1 dep with deps, otherwise the last few components will not be enough
            # to tie together all components.
            num_deps_with_deps = len(toplevel_components) - (num_components_with_deps - 1 - i) * (num_deps - 1)
            deps |= set(random.sample(toplevel_components, num_deps_with_deps))

        # Add other deps to get to the desired num_deps.
        deps |= set(random.sample(range(0, num_components_with_no_deps + i), num_deps - len(deps)))

        toplevel_components -= deps
        for dep in deps:
            is_toplevel[dep] = False

        component_id = num_used_ids
        toplevel_components |= {component_id}
        num_used_ids += 1
        deps_list = list(deps)
        random.shuffle(deps_list)
        for dep in deps_list:
            injection_graph.add_edge(component_id, dep)

    assert len(toplevel_components) == 1, toplevel_components
    toplevel_component = num_used_ids - 1
    assert is_toplevel[toplevel_component]

    return injection_graph

def generate_benchmark(
        di_library,
        compiler,
        cxx_std,
        output_dir,
        num_components_with_no_deps,
        num_components_with_deps,
        num_deps,
        generate_runtime_bench_code,
        use_exceptions=True,
        use_rtti=True,
        fruit_build_dir=None,
        fruit_sources_dir=None,
        boost_di_sources_dir=None,
        generate_debuginfo=False,
        use_new_delete=False,
        use_interfaces=False,
        use_normalized_component=False):
    """Generates a sample codebase using the specified DI library, meant for benchmarking.

    :param boost_di_sources_dir: this is only used if di_library=='boost_di', it can be None otherwise.
    """

    if num_components_with_no_deps < num_deps:
        raise Exception(
            "Too few components with no deps. num_components_with_no_deps=%s but num_deps=%s." % (num_components_with_no_deps, num_deps))
    if num_deps < 2:
        raise Exception("num_deps should be at least 2.")

    # This is a constant so that we always generate the same file (=> benchmark more repeatable).
    random.seed(42)

    injection_graph = generate_injection_graph(num_components_with_no_deps=num_components_with_no_deps,
                                               num_components_with_deps=num_components_with_deps,
                                               num_deps=num_deps)

    if di_library == 'fruit':
        file_content_by_name = fruit_source_generator.generate_files(injection_graph, generate_runtime_bench_code)
        include_dirs = [fruit_build_dir + '/include', fruit_sources_dir + '/include']
        library_dirs = [fruit_build_dir + '/src']
        link_libraries = ['fruit']
    elif di_library == 'boost_di':
        file_content_by_name = boost_di_source_generator.generate_files(injection_graph, generate_runtime_bench_code)
        include_dirs = [boost_di_sources_dir + '/include', boost_di_sources_dir + '/extension/include']
        library_dirs = []
        link_libraries = []
    elif di_library == 'none':
        file_content_by_name = no_di_library_source_generator.generate_files(injection_graph, use_new_delete, use_interfaces, generate_runtime_bench_code)
        include_dirs = []
        library_dirs = []
        link_libraries = []
    else:
        raise Exception('Unrecognized di_library: %s' % di_library)

    include_flags = ' '.join(['-I%s' % include_dir for include_dir in include_dirs])
    library_dirs_flags = ' '.join(['-L%s' % library_dir for library_dir in library_dirs])
    rpath_flags = ' '.join(['-Wl,-rpath,%s' % library_dir for library_dir in library_dirs])
    link_libraries_flags = ' '.join(['-l%s' % library for library in link_libraries])
    other_compile_flags = []
    if generate_debuginfo:
        other_compile_flags.append('-g')
    if not use_exceptions:
        other_compile_flags.append('-fno-exceptions')
    if not use_rtti:
        other_compile_flags.append('-fno-rtti')
    compile_command = '%s -std=%s -MMD -MP -O2 -W -Wall -Werror -DNDEBUG -ftemplate-depth=10000 %s %s' % (compiler, cxx_std, include_flags, ' '.join(other_compile_flags))
    link_command = '%s -std=%s -O2 -W -Wall -Werror %s %s' % (compiler, cxx_std, rpath_flags, library_dirs_flags)
    # GCC requires passing the -lfruit flag *after* all object files to be linked for some reason.
    link_command_suffix = link_libraries_flags

    cpp_files = [file_name
                 for file_name in file_content_by_name.keys()
                 if file_name.endswith('.cpp')]

    file_content_by_name['Makefile'] = generate_makefile(cpp_files, 'main', compile_command, link_command, link_command_suffix)

    os.makedirs(output_dir, exist_ok=True)
    for file_name, file_content in file_content_by_name.items():
        with open('%s/%s' % (output_dir, file_name), 'w') as file:
            file.write(file_content)

    return file_content_by_name.keys()

def main():
    parser = argparse.ArgumentParser(description='Generates source files and a build script for benchmarks.')
    parser.add_argument('--di-library', default='fruit', help='DI library to use. One of {fruit, boost_di, none}. (default: fruit)')
    parser.add_argument('--compiler', help='Compiler to use')
    parser.add_argument('--fruit-sources-dir', help='Path to the fruit sources (only used when di_library==\'fruit\')')
    parser.add_argument('--fruit-build-dir', help='Path to the fruit build dir (only used with --di_library=\'fruit\')')
    parser.add_argument('--boost-di-sources-dir', help='Path to the Boost.DI sources (only used with --di-library==\'boost_di\')')
    parser.add_argument('--num-components-with-no-deps', default=10, help='Number of components with no deps that will be generated')
    parser.add_argument('--num-components-with-deps', default=90, help='Number of components with deps that will be generated')
    parser.add_argument('--num-deps', default=10, help='Number of deps in each component with deps that will be generated')
    parser.add_argument('--output-dir', help='Output directory for generated files')
    parser.add_argument('--cxx-std', default='c++11',
                        help='Version of the C++ standard to use. Typically one of \'c++11\' and \'c++14\'. (default: \'c++11\')')
    parser.add_argument('--use-new-delete', default='false', help='Set this to \'true\' to use new/delete. Only relevant when --di_library=none.')
    parser.add_argument('--use-interfaces', default='false', help='Set this to \'true\' to use interfaces. Only relevant when --di_library=none.')
    parser.add_argument('--use-normalized-component', default='false', help='Set this to \'true\' to create a NormalizedComponent and create the injector from that. Only relevant when --di_library=fruit and --generate-runtime-bench-code=false.')
    parser.add_argument('--generate-runtime-bench-code', default='true', help='Set this to \'false\' for compile benchmarks.')
    parser.add_argument('--generate-debuginfo', default='false', help='Set this to \'true\' to generate debugging information (-g).')
    parser.add_argument('--use-exceptions', default='true', help='Set this to \'false\' to disable exceptions.')
    parser.add_argument('--use-rtti', default='true', help='Set this to \'false\' to disable RTTI.')

    args = parser.parse_args()

    if args.compiler is None:
        raise Exception('--compiler is required.')

    if args.di_library == 'fruit':
        if args.fruit_sources_dir is None:
            raise Exception('--fruit-sources-dir is required with --di-library=\'fruit\'.')
        if args.fruit_build_dir is None:
            raise Exception('--fruit-build-dir is required with --di-library=\'fruit\'.')
    elif args.di_library == 'boost_di':
        if args.boost_di_sources_dir is None:
            raise Exception('--boost-di-sources-dir is required with --di-library=\'boost_di\'.')
    elif args.di_library == 'none':
        pass
    else:
        raise Exception('Unrecognized --di-library: \'%s\'. Allowed values are %s' % (args.di_library, {'fruit', 'boost_di', 'none'}))

    num_components_with_deps = int(args.num_components_with_deps)
    num_components_with_no_deps = int(args.num_components_with_no_deps)
    num_deps = int(args.num_deps)

    if args.output_dir is None:
        raise Exception("output_dir must be specified.")

    generate_benchmark(
        di_library=args.di_library,
        fruit_sources_dir=args.fruit_sources_dir,
        boost_di_sources_dir=args.boost_di_sources_dir,
        output_dir=args.output_dir,
        compiler=args.compiler,
        cxx_std=args.cxx_std,
        num_components_with_deps=num_components_with_deps,
        num_components_with_no_deps=num_components_with_no_deps,
        fruit_build_dir=args.fruit_build_dir,
        num_deps=num_deps,
        generate_debuginfo=(args.generate_debuginfo == 'true'),
        use_new_delete=(args.use_new_delete == 'true'),
        use_interfaces=(args.use_interfaces == 'true'),
        use_normalized_component=(args.use_normalized_component == 'true'),
        generate_runtime_bench_code=(args.generate_runtime_bench_code == 'true'),
        use_exceptions=(args.use_exceptions == 'true'),
        use_rtti=(args.use_rtti == 'true'))


if __name__ == "__main__":
    main()
