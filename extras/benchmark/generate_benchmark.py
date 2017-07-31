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

from fruit_source_generator import FruitSourceGenerator
from boost_di_source_generator import BoostDiSourceGenerator
from makefile_generator import generate_makefile
import argparse


def add_node(n, deps, source_generator, output_dir):
    with open('%s/component%s.h' % (output_dir, n), 'w') as headerFile:
        headerFile.write(source_generator.generate_component_header(n))
    with open('%s/component%s.cpp' % (output_dir, n), 'w') as sourceFile:
        sourceFile.write(source_generator.generate_component_source(n, deps))


def generate_benchmark(
        di_library,
        compiler,
        cxx_std,
        fruit_build_dir,
        fruit_sources_dir,
        output_dir,
        num_components_with_no_deps,
        num_components_with_deps,
        num_deps,
        boost_di_sources_dir=None,
        use_fruit_2_x_syntax=False,
        generate_debuginfo=False):
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

    if di_library == 'fruit':
        source_generator = FruitSourceGenerator(
            use_fruit_2_x_syntax = use_fruit_2_x_syntax)
        include_dirs = [fruit_build_dir + '/include', fruit_sources_dir + '/include']
        library_dirs = [fruit_build_dir + '/src']
        link_libraries = ['fruit']
    elif di_library == 'boost_di':
        source_generator = BoostDiSourceGenerator()
        include_dirs = [boost_di_sources_dir + '/include']
        library_dirs = []
        link_libraries = []
    else:
        raise Exception('Unrecognized di_library: %s' % di_library)

    os.makedirs(output_dir, exist_ok=True)

    num_used_ids = 0
    is_toplevel = [True for i in range(0, num_components_with_no_deps + num_components_with_deps)]
    toplevel_components = set()
    for i in range(0, num_components_with_no_deps):
        id = num_used_ids
        num_used_ids += 1
        add_node(id, [], source_generator=source_generator, output_dir=output_dir)
        toplevel_components |= {id}

    # Then the rest have num_deps deps, chosen (pseudo-)randomly from the previous components with no
    # deps, plus the previous component with deps (if any).
    # However, the last few components depend on multiple components with >1 deps, so that the last
    # component transitively depends on everything.
    for i in range(0, num_components_with_deps):
        deps = set()

        if len(toplevel_components) > (num_components_with_deps - 1 - i) * (num_deps - 1):
            # We need at least 1 dep with deps, otherwise the last few components will not be enough
            # to tie together all components.
            num_deps_with_deps = len(toplevel_components) - (num_components_with_deps - 1 - i) * (num_deps - 1)
            deps |= set(random.sample(toplevel_components, num_deps_with_deps))

        if i != 0 and len(deps) < num_deps:
            # Pick one random component with deps.
            # If we picked num_deps random components here, the computation of the n-th component (during
            # the benchmark) would take time super-linear in n, and we don't want that (if most time was
            # spent constructing the component rather than constructing the injector and injecting objects,
            # the benchmark would be slow and not very meaningful).
            deps |= {num_components_with_no_deps + random.randint(0, i - 1)}

        # Add other deps with no deps to get to the desired num_deps.
        deps |= set(random.sample(range(0, num_components_with_no_deps), num_deps - len(deps)))

        toplevel_components -= deps
        for dep in deps:
            is_toplevel[dep] = False

        component_id = num_used_ids
        toplevel_components |= {component_id}
        num_used_ids += 1
        deps_list = list(deps)
        random.shuffle(deps_list)
        add_node(component_id, deps_list, source_generator, output_dir=output_dir)

    assert len(toplevel_components) == 1, toplevel_components
    toplevel_component = num_used_ids - 1
    assert is_toplevel[toplevel_component]

    with open("%s/main.cpp" % output_dir, 'w') as mainFile:
        mainFile.write(source_generator.generate_main(toplevel_component))

    include_flags = ' '.join(['-I%s' % include_dir for include_dir in include_dirs])
    library_dirs_flags = ' '.join(['-L%s' % library_dir for library_dir in library_dirs])
    rpath_flags = ' '.join(['-Wl,-rpath,%s' % library_dir for library_dir in library_dirs])
    link_libraries_flags = ' '.join(['-l%s' % library for library in link_libraries])
    other_compile_flags = []
    if use_fruit_2_x_syntax:
        other_compile_flags.append('-Wno-deprecated-declarations')
    if generate_debuginfo:
        other_compile_flags.append('-g')
    compile_command = '%s -std=%s -O2 -W -Wall -Werror -DNDEBUG -ftemplate-depth=1000 %s %s' % (compiler, cxx_std, include_flags, ' '.join(other_compile_flags))
    link_command = '%s -std=%s -O2 -W -Wall -Werror %s %s' % (compiler, cxx_std, rpath_flags, library_dirs_flags)
    # GCC requires passing the -lfruit flag *after* all object files to be linked for some reason.
    link_command_suffix = link_libraries_flags

    sources = ['component%s' % i for i in range(0, num_used_ids)]
    sources += ['main']

    with open("%s/Makefile" % output_dir, 'w') as makefile:
        makefile.write(generate_makefile(sources, 'main', compile_command, link_command, link_command_suffix))


def main():
    parser = argparse.ArgumentParser(description='Generates source files and a build script for benchmarks.')
    parser.add_argument('--di-library', default='fruit', help='DI library to use. One of {fruit, boost_di}. (default: fruit)')
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
    parser.add_argument('--use-fruit-2-x-syntax', default=False, help='Set this to \'true\' to generate source files compatible with Fruit 2.x (instead of 3.x).')
    parser.add_argument('--generate-debuginfo', default=False, help='Set this to \'true\' to generate debugging information (-g).')

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
    else:
        raise Exception('Unrecognized --di-library: \'%s\'. Allowed values are %s' % (args.di_library, {'fruit', 'boost_di'}))

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
        use_fruit_2_x_syntax=(args.use_fruit_2_x_syntax == 'true'),
        generate_debuginfo=(args.generate_debuginfo == 'true'))


if __name__ == "__main__":
    main()
