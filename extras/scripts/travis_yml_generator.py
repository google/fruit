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

def add_ubuntu_tests(ubuntu_version, compiler, stl=None, asan=True, ubsan=True, valgrind=True):
  env = 'UBUNTU=%s COMPILER=%s' % (ubuntu_version, compiler)
  if stl is not None:
    env += ' STL=%s' % stl
  compiler_kind = determine_compiler_kind(compiler)
  test_environment_template = {'os': 'linux', 'compiler': compiler_kind, 'env': env,
                               'install': 'extras/scripts/travis_ci_install_linux.sh'}

  for test in determine_tests(asan, ubsan, valgrind):
    test_environment = test_environment_template.copy()
    test_environment['script'] = 'extras/scripts/postsubmit.sh %s' % test
    build_matrix_rows.append(test_environment)


def add_osx_tests(compiler, xcode_version=None, stl=None, asan=True, ubsan=True, valgrind=True):
  env = 'COMPILER=%s' % compiler
  if stl is not None:
    env += ' STL=%s' % stl
  compiler_kind = determine_compiler_kind(compiler)
  test_environment_template = {'os': 'osx', 'compiler': compiler_kind, 'env': env,
                               'install': 'extras/scripts/travis_ci_install_osx.sh'}
  if xcode_version is not None:
    test_environment_template['osx_image'] = 'xcode%s' % xcode_version

  for test in determine_tests(asan, ubsan, valgrind):
    test_environment = test_environment_template.copy()
    test_environment['script'] = 'extras/scripts/postsubmit.sh %s' % test
    build_matrix_rows.append(test_environment)


def add_bazel_tests(ubuntu_version):
  env = 'UBUNTU=%s COMPILER=bazel' % ubuntu_version
  test_environment = {'os': 'linux',
                      'compiler': 'gcc',
                      'env': env,
                      'install': 'extras/scripts/travis_ci_install_linux.sh',
                      'script': 'extras/scripts/postsubmit.sh DebugPlain'}
  build_matrix_rows.append(test_environment)


add_ubuntu_tests(ubuntu_version='16.04', compiler='gcc-6')
add_ubuntu_tests(ubuntu_version='16.04', compiler='gcc-5')
add_ubuntu_tests(ubuntu_version='16.04', compiler='clang-3.8', stl='libstdc++')

#add_ubuntu_tests(ubuntu_version='15.10', compiler='gcc-5', ubsan=False)
#add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.6', stl='libstdc++')
#add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.8', stl='libstdc++')
#add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.6', stl='libc++')
#add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.8', stl='libc++', ubsan=False)

add_bazel_tests(ubuntu_version='15.10')

#add_ubuntu_tests(ubuntu_version='14.04', compiler='gcc-4.8')
#add_ubuntu_tests(ubuntu_version='14.04', compiler='gcc-5')
#add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.5', stl='libstdc++')
#add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.8', stl='libstdc++', ubsan=False)
#add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.5', stl='libc++')
#add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.8', stl='libc++', ubsan=False)

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
