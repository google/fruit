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

import yaml

build_matrix_rows = []

def determine_compiler_kind(compiler):
  if compiler.startswith('gcc'):
    return 'gcc'
  elif compiler.startswith('clang'):
    return 'clang'
  else:
    raise Exception('Unexpected compiler: %s' % compiler)


def determine_tests(asan, ubsan, valgrind):
  tests = []
  has_debug_build = False
  if valgrind:
    tests += ['DebugValgrind', 'ReleaseValgrind']
    has_debug_build = True
  else:
    tests += ['ReleasePlain']
  if asan:
    has_debug_build = True
    if ubsan:
      tests += ['DebugAsanUbsan']
    else:
      tests += ['DebugAsan']
  if ubsan and not asan:
    raise Exception('Enabling UBSan but not ASan is not currently supported.')
  if not has_debug_build:
    tests += ['DebugPlain']
  return tests

def generate_export_statements_for_env(env):
  return ' '.join(['export %s=\'%s\';' % (var_name, value) for (var_name, value) in sorted(env.items())])

def generate_env_string_for_env(env):
  return ' '.join(['%s=%s' % (var_name, value) for (var_name, value) in sorted(env.items())])

def add_ubuntu_tests(ubuntu_version, compiler, stl=None, asan=True, ubsan=True, valgrind=True):
  env = {
    'UBUNTU': ubuntu_version,
    'COMPILER': compiler
  }
  if stl is not None:
    env['STL'] = stl
  compiler_kind = determine_compiler_kind(compiler)
  export_statements = generate_export_statements_for_env(env=env)
  test_environment_template = {'os': 'linux', 'compiler': compiler_kind,
                               'install': '%s extras/scripts/travis_ci_install_linux.sh' % export_statements}
  for test in determine_tests(asan, ubsan, valgrind):
    test_environment = test_environment_template.copy()
    test_environment['script'] = '%s extras/scripts/postsubmit.sh %s' % (export_statements, test)
    # The TEST variable has no effect on the test run, but allows to see the test name in the Travis CI dashboard.
    test_environment['env'] = generate_env_string_for_env(env) + " TEST=%s" % test
    build_matrix_rows.append(test_environment)


def add_osx_tests(compiler, xcode_version=None, stl=None, asan=True, ubsan=True, valgrind=True):
  env = {'COMPILER': compiler}
  if stl is not None:
    env['STL'] = stl
  compiler_kind = determine_compiler_kind(compiler)
  export_statements = generate_export_statements_for_env(env=env)
  test_environment_template = {'os': 'osx', 'compiler': compiler_kind,
                               'install': '%s extras/scripts/travis_ci_install_osx.sh' % export_statements}
  if xcode_version is not None:
    test_environment_template['osx_image'] = 'xcode%s' % xcode_version

  for test in determine_tests(asan, ubsan, valgrind):
    test_environment = test_environment_template.copy()
    test_environment['script'] = '%s extras/scripts/postsubmit.sh %s' % (export_statements, test)
    # The TEST variable has no effect on the test run, but allows to see the test name in the Travis CI dashboard.
    test_environment['env'] = generate_env_string_for_env(env) + " TEST=%s" % test
    build_matrix_rows.append(test_environment)


def add_bazel_tests(ubuntu_version):
  env = {
    'UBUNTU': ubuntu_version,
    'COMPILER': 'bazel',
  }
  export_statements = generate_export_statements_for_env(env=env)
  test_environment = {'os': 'linux',
                      'compiler': 'gcc',
                      'env': generate_env_string_for_env(env),
                      'install': '%s extras/scripts/travis_ci_install_linux.sh' % export_statements,
                      'script': '%s extras/scripts/postsubmit.sh DebugPlain' % export_statements}
  build_matrix_rows.append(test_environment)


add_ubuntu_tests(ubuntu_version='16.04', compiler='gcc-6')
add_ubuntu_tests(ubuntu_version='16.04', compiler='gcc-5')
add_ubuntu_tests(ubuntu_version='16.04', compiler='clang-3.8', stl='libstdc++')

add_ubuntu_tests(ubuntu_version='15.10', compiler='gcc-5', ubsan=False)
add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.6', stl='libstdc++')
add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.8', stl='libstdc++')
add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.6', stl='libc++')
add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.8', stl='libc++', ubsan=False)

add_bazel_tests(ubuntu_version='15.10')

add_ubuntu_tests(ubuntu_version='14.04', compiler='gcc-4.8')
add_ubuntu_tests(ubuntu_version='14.04', compiler='gcc-5')
add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.5', stl='libstdc++')
add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.8', stl='libstdc++', ubsan=False)
add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.5', stl='libc++')
add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.8', stl='libc++', ubsan=False)

add_osx_tests(compiler='gcc-4.8', asan=False, ubsan=False)
add_osx_tests(compiler='gcc-5')
add_osx_tests(compiler='clang-3.6', stl='libc++', asan=False, ubsan=False)
add_osx_tests(compiler='clang-3.7', stl='libc++', asan=False, ubsan=False)
add_osx_tests(compiler='clang-3.8', stl='libc++')
add_osx_tests(compiler='clang-default', xcode_version='7.1', stl='libc++', ubsan=False)
add_osx_tests(compiler='clang-default', xcode_version='7.3', stl='libc++', ubsan=False)
add_osx_tests(compiler='clang-default', xcode_version='8', stl='libc++', ubsan=False)

yaml_file = {
  'sudo': 'required',
  'dist': 'trusty',
  'services' : ['docker'],
  'language': 'cpp',
  'branches': {
    'only': ['master'],
  },
  'matrix': {
    'fast_finish': True,
    'include': build_matrix_rows,
  },
}

class CustomDumper(yaml.SafeDumper):
   def ignore_aliases(self, _data):
       return True

print(yaml.dump(yaml_file, default_flow_style=False, Dumper=CustomDumper))
