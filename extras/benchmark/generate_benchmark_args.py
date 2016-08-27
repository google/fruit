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

import argparse

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
parser.add_argument('--cxx-std', default='c++11', help='Version of the C++ standard to use. Typically one of \'c++11\' and \'c++14\'. (default: \'c++11\')')

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
  raise Exception('Unrecognized --di-library: \'%s\'. Allowed values are %s' % (args.di_library, supported_libraries))

num_components_with_deps = int(args.num_components_with_deps)
num_components_with_no_deps = int(args.num_components_with_no_deps)
num_deps = int(args.num_deps)

if num_components_with_no_deps < num_deps:
  raise Exception("Too few components with no deps. --num-components-with-no-deps=%s but --num-deps=%s." % (num_components_with_no_deps, num_deps))

if num_deps < 2:
  raise Exception("num_deps should be at least 2.")

if args.output_dir is None:
  raise Exception("output_dir must be specified.")
