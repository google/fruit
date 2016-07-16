#!/usr/bin/python3
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

import random
import os
import shutil

from fruit_source_generator import *
from boost_di_source_generator import *
from generate_benchmark_args import *
from makefile_generator import *

def add_node(n, deps):
  with open('component%s.h' % n, 'w') as headerFile:
    headerFile.write(source_generator.generateComponentHeader(n))
  with open('component%s.cpp' % n, 'w') as sourceFile:
    sourceFile.write(source_generator.generateComponentSource(n, deps))

# This is a constant so that we always generate the same file (=> benchmark more repeatable).
random.seed(42);

if args.di_library == 'fruit':
  source_generator = FruitSourceGenerator()
  include_dirs = [args.fruit_build_dir + '/include', args.fruit_sources_dir + '/include']
elif args.di_library == 'boost_di':
  source_generator = BoostDiSourceGenerator()
  include_dirs = [args.boost_di_sources_dir + '/include']

os.makedirs(args.output_dir, exist_ok=True)
os.chdir(args.output_dir)

num_used_ids = 0
is_toplevel = [True for i in range(0, num_components_with_no_deps + num_components_with_deps)]
toplevel_components = set()
for i in range(0, num_components_with_no_deps):
  id = num_used_ids
  num_used_ids += 1
  add_node(id, [])
  toplevel_components |= set((id,))

# Then the rest have num_deps deps, chosen (pseudo-)randomly from the previous components with no
# deps, plus the previous component with deps (if any).
# However, the last few components depend on multiple components with >1 deps, so that the last
# component transitively depends on everything.
for i in range(0, num_components_with_deps):
  deps = set()
  
  if len(toplevel_components) > (num_components_with_deps - 1 - i)*(num_deps - 1):
    # We need at least 1 dep with deps, otherwise the last few components will not be enough
    # to tie together all components.
    num_deps_with_deps = len(toplevel_components) - (num_components_with_deps - 1 - i)*(num_deps - 1)
    deps |= set(random.sample(toplevel_components, num_deps_with_deps))
  
  if i != 0 and len(deps) < num_deps:
    # Pick one random component with deps.
    # If we picked num_deps random components here, the computation of the n-th component (during
    # the benchmark) would take time super-linear in n, and we don't want that (if most time was
    # spent constructing the component rather than constructing the injector and injecting objects,
    # the benchmark would be slow and not very meaningful).
    deps |= set((num_components_with_no_deps + random.randint(0, i - 1),))
  
  # Add other deps with no deps to get to the desired num_deps.
  deps |= set(random.sample(range(0, num_components_with_no_deps), num_deps - len(deps)))
  
  toplevel_components -= deps
  for dep in deps:
    is_toplevel[dep] = False
  
  component_id = num_used_ids
  toplevel_components |= set((component_id,))
  num_used_ids += 1
  deps_list = list(deps)
  random.shuffle(deps_list)
  add_node(component_id, deps_list)

assert len(toplevel_components) == 1, toplevel_components
toplevel_component = num_used_ids - 1
assert is_toplevel[toplevel_component]

with open("main.cpp", 'w') as mainFile:
  mainFile.write(source_generator.generateMain(toplevel_component))

include_flags = ' '.join(['-I%s' % include_dir for include_dir in include_dirs])
compile_command = '%s -std=c++14 -O2 -g -W -Wall -Werror -DNDEBUG -ftemplate-depth=1000 %s' % (args.compiler, include_flags)

sources = ['component%s' % i for i in range(0, num_used_ids)]
sources += ['main']
if args.di_library == 'fruit':
  library_srcs = source_generator.getLibrarySources()
  for library_src_file in library_srcs:
    shutil.copy('%s/src/%s.cpp' % (args.fruit_sources_dir, library_src_file), '.')
    sources += [library_src_file]

with open("Makefile", 'w') as makefile:
  makefile.write(generateMakefile(sources, 'main', compile_command))
