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
  export_statements = 'export OS=linux; ' + generate_export_statements_for_env(env=env)
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
  export_statements = 'export OS=osx; ' + generate_export_statements_for_env(env=env)
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
  export_statements = 'export OS=linux; ' + generate_export_statements_for_env(env=env)
  test_environment = {'os': 'linux',
                      'compiler': 'gcc',
                      'env': generate_env_string_for_env(env),
                      'install': '%s extras/scripts/travis_ci_install_linux.sh' % export_statements,
                      'script': '%s extras/scripts/postsubmit.sh DebugPlain' % export_statements}
  build_matrix_rows.append(test_environment)


add_ubuntu_tests(ubuntu_version='16.04', compiler='gcc-6')
add_ubuntu_tests(ubuntu_version='16.04', compiler='gcc-5')
add_ubuntu_tests(ubuntu_version='16.04', compiler='clang-3.8', stl='libstdc++')

# UBSan is disabled because it would fail with an error like:
# runtime error: member call on null pointer of type 'const struct __lambda26'
# This issue is fixed in the version of GCC shipped in Ubuntu 16.04.
add_ubuntu_tests(ubuntu_version='15.10', compiler='gcc-5', ubsan=False)
add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.6', stl='libstdc++')
add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.8', stl='libstdc++')
# UBSan is disabled because it would fail with an error like:
# /usr/include/c++/v1/ostream:236:9: runtime error: cast to virtual base of address 0x000000807408
# which does not point to an object of type 'std::__1::basic_ostream<char, std::__1::char_traits<char> >'
# 0x000000807408: note: object is of type 'std::__1::basic_ostream<char, std::__1::char_traits<char> >'
add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.6', stl='libc++', ubsan=False)
# UBSan is disabled because it would fail with an error like:
# /usr/include/c++/v1/memory:1554:35: runtime error: null pointer passed as argument 2, which is declared to never be null
add_ubuntu_tests(ubuntu_version='15.10', compiler='clang-3.8', stl='libc++', ubsan=False)

add_bazel_tests(ubuntu_version='15.10')

# UBSan (aka '-fsanitize=undefined') is not supported in GCC 4.8.
add_ubuntu_tests(ubuntu_version='14.04', compiler='gcc-4.8', ubsan=False)
# ASan/UBSan are disabled because they would fail with the error:
# /usr/bin/ld: unrecognized option '--push-state'
add_ubuntu_tests(ubuntu_version='14.04', compiler='gcc-5', asan=False, ubsan=False)
add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.5', stl='libstdc++')
add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.8', stl='libstdc++')
# UBSan is disabled because it would fail with an error like:
# /usr/include/c++/v1/memory:4273:18: runtime error: member call on address 0x60300000efe0
# which does not point to an object of type 'std::__1::__shared_ptr_emplace<int, std::__1::allocator<int> >'
# 0x60300000efe0: note: object is of type 'std::__1::__shared_ptr_emplace<int, std::__1::allocator<int> >'
add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.5', stl='libc++', ubsan=False)
# UBSan is disabled because Ubuntu Trusty uses libc++ 1.x that doesn't work
# with UBSan, it fails with this error:
# /usr/include/c++/v1/memory:1550:35: runtime error: null pointer passed as argument 2, which is declared to never be null
add_ubuntu_tests(ubuntu_version='14.04', compiler='clang-3.8', stl='libc++', ubsan=False)

# UBSan (aka '-fsanitize=undefined') is not supported in GCC 4.8.
# ASan (aka '-fsanitize=address') doesn't work, due to https://llvm.org/bugs/show_bug.cgi?id=27310.
add_osx_tests(compiler='gcc-4.8', asan=False, ubsan=False)
add_osx_tests(compiler='gcc-5')
# ASan/UBSan are disabled because it would hit errors like:
# ld: file not found: [...]/libclang_rt.asan_osx_dynamic.dylib
# ld: file not found: [...]/libclang_rt.ubsan_osx.a
# Not sure if that's a limitation of Clang 3.6 on OS X or just of the brew-provided binaries.
add_osx_tests(compiler='clang-3.6', stl='libc++', asan=False, ubsan=False)
# ASan/UBSan are disabled because it would hit errors like:
# ld: file not found: [...]/libclang_rt.asan_osx_dynamic.dylib
# ld: file not found: [...]/libclang_rt.ubsan_osx.a
# Not sure if that's a limitation of Clang 3.6 on OS X or just of the brew-provided binaries.
add_osx_tests(compiler='clang-3.7', stl='libc++', asan=False, ubsan=False)
add_osx_tests(compiler='clang-3.8', stl='libc++')

# UBSan is disabled because AppleClang does not support -fsanitize=undefined.
add_osx_tests(compiler='clang-default', xcode_version='7.1', stl='libc++', ubsan=False)
# UBSan is disabled because AppleClang does not support -fsanitize=undefined.
add_osx_tests(compiler='clang-default', xcode_version='7.3', stl='libc++', ubsan=False)
# UBSan is disabled because AppleClang does not support -fsanitize=undefined.
add_osx_tests(compiler='clang-default', xcode_version='8', stl='libc++', ubsan=False)

# ** Disabled combinations **
#
# These fail with "'type_traits' file not found" (the <type_traits> header is missing).
#
#   add_osx_tests('gcc-default', stl='libstdc++')
#   add_osx_tests('clang-default', stl='libstdc++')
#   add_osx_tests('clang-3.5', stl='libstdc++')
#   add_osx_tests('clang-3.6', stl='libstdc++')
#
#
# The compiler complains that the 2-argument constructor of std::pair is ambiguous, even after
# adding explicit casts to the exact types of the expected overload.
#
#   add_osx_tests('clang-default', stl='libc++')
#
#
# This triggers an assert error in the compiler, with the message:
# "expected to get called on an inlined function!" [...] function isMSExternInline, file Decl.cpp, line 2647.
#
#   add_osx_tests('clang-3.5', stl='libc++', asan=False, ubsan=False, valgrind=False)
#
#
# This fails with this error:
# /usr/include/c++/v1/string:1938:44: error: 'basic_string<_CharT, _Traits, _Allocator>' is missing
# exception specification 'noexcept(is_nothrow_copy_constructible<allocator_type>::value)'
# TODO: Try again every once in a while (to re-enable these once the bug in libc++ is fixed).
#
#   add_ubuntu_tests(ubuntu_version='16.04', compiler='clang-3.8', stl='libc++', asan=False, ubsan=False)
#


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

print('#')
print('# This file was auto-generated from extras/scripts/travis_yml_generator.py, DO NOT EDIT')
print('#')
print(yaml.dump(yaml_file, default_flow_style=False, Dumper=CustomDumper))
