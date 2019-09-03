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

# "smoke tests" are run before other build matrix rows.
build_matrix_smoke_test_rows = []
build_matrix_rows = []


def determine_compiler_kind(compiler):
    if compiler.startswith('gcc'):
        return 'gcc'
    elif compiler.startswith('clang'):
        return 'clang'
    else:
        raise Exception('Unexpected compiler: %s' % compiler)


def determine_tests(asan, ubsan, smoke_tests, use_precompiled_headers_in_tests, exclude_tests,
                    include_only_tests):
    tests = []
    has_debug_build = False
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
    for smoke_test in smoke_tests:
        if smoke_test not in tests:
            tests += [smoke_test]
    excessive_excluded_tests = set(exclude_tests) - set(tests)
    if excessive_excluded_tests:
        raise Exception(
            'Some tests were excluded but were not going to run anyway: %s. '
            'Tests to run (ignoring the possible NoPch prefix): %s'
            % (excessive_excluded_tests, tests))
    if include_only_tests is not None:
        if exclude_tests != []:
            raise Exception('Using exclude_tests and include_only_tests together is not supported.')
        tests = include_only_tests
    else:
        tests = [test for test in tests if test not in exclude_tests]
    if not use_precompiled_headers_in_tests:
        tests = [test + 'NoPch' for test in tests]
    return tests


def generate_export_statements_for_env(env):
    return ' '.join(['export %s=\'%s\';' % (var_name, value) for (var_name, value) in sorted(env.items())])


def generate_env_string_for_env(env):
    return ' '.join(['%s=%s' % (var_name, value) for (var_name, value) in sorted(env.items())])


def add_ubuntu_tests(ubuntu_version, compiler, os='linux', stl=None, asan=True, ubsan=True,
                     use_precompiled_headers_in_tests=True, smoke_tests=[], exclude_tests=[], include_only_tests=None):
    env = {
        'UBUNTU': ubuntu_version,
        'COMPILER': compiler
    }
    if stl is not None:
        env['STL'] = stl
    compiler_kind = determine_compiler_kind(compiler)
    export_statements = 'export OS=' + os + '; ' + generate_export_statements_for_env(env=env)
    test_environment_template = {'os': 'linux', 'compiler': compiler_kind,
                                 'install': '%s extras/scripts/travis_ci_install_linux.sh' % export_statements}
    tests = determine_tests(asan, ubsan, smoke_tests,
                            use_precompiled_headers_in_tests=use_precompiled_headers_in_tests,
                            exclude_tests=exclude_tests,
                            include_only_tests=include_only_tests)
    for test in tests:
        test_environment = test_environment_template.copy()
        test_environment['script'] = '%s extras/scripts/postsubmit.sh %s' % (export_statements, test)
        # The TEST variable has no effect on the test run, but allows to see the test name in the Travis CI dashboard.
        test_environment['env'] = generate_env_string_for_env(env) + " TEST=%s" % test
        if test in smoke_tests:
            build_matrix_smoke_test_rows.append(test_environment)
        else:
            build_matrix_rows.append(test_environment)


def add_osx_tests(compiler, xcode_version=None, stl=None, asan=True, ubsan=True,
                  use_precompiled_headers_in_tests=True, smoke_tests=[], exclude_tests=[], include_only_tests=None):
    env = {'COMPILER': compiler}
    if stl is not None:
        env['STL'] = stl
    compiler_kind = determine_compiler_kind(compiler)
    export_statements = 'export OS=osx; ' + generate_export_statements_for_env(env=env)
    test_environment_template = {'os': 'osx', 'compiler': compiler_kind,
                                 'install': '%s extras/scripts/travis_ci_install_osx.sh' % export_statements}
    if xcode_version is not None:
        test_environment_template['osx_image'] = 'xcode%s' % xcode_version

    tests = determine_tests(asan, ubsan, smoke_tests,
                            use_precompiled_headers_in_tests=use_precompiled_headers_in_tests,
                            exclude_tests=exclude_tests, include_only_tests=include_only_tests)
    for test in tests:
        test_environment = test_environment_template.copy()
        test_environment['script'] = '%s extras/scripts/postsubmit.sh %s' % (export_statements, test)
        # The TEST variable has no effect on the test run, but allows to see the test name in the Travis CI dashboard.
        test_environment['env'] = generate_env_string_for_env(env) + " TEST=%s" % test
        if test in smoke_tests:
            build_matrix_smoke_test_rows.append(test_environment)
        else:
            build_matrix_rows.append(test_environment)


def add_bazel_tests(ubuntu_version, smoke_tests=[]):
    env = {
        'UBUNTU': ubuntu_version,
        'COMPILER': 'bazel',
    }
    test = 'DebugPlain'
    export_statements = 'export OS=linux; ' + generate_export_statements_for_env(env=env)
    test_environment = {'os': 'linux',
                        'compiler': 'gcc',
                        'env': generate_env_string_for_env(env),
                        'install': '%s extras/scripts/travis_ci_install_linux.sh' % export_statements,
                        'script': '%s extras/scripts/postsubmit.sh %s' % (export_statements, test)}
    if test in smoke_tests:
        build_matrix_smoke_test_rows.append(test_environment)
    else:
        build_matrix_rows.append(test_environment)


# TODO: re-enable ASan/UBSan once they work in Travis CI. ATM (as of 18 November 2017) they fail due to https://github.com/google/sanitizers/issues/837
add_ubuntu_tests(ubuntu_version='19.04', compiler='gcc-9', asan=False, ubsan=False,
                 smoke_tests=['DebugPlain', 'ReleasePlain'])
add_ubuntu_tests(ubuntu_version='19.04', compiler='clang-6.0', stl='libstdc++',
                 smoke_tests=['DebugPlain', 'DebugAsanUbsan', 'ReleasePlain'])
add_ubuntu_tests(ubuntu_version='19.04', compiler='clang-8.0', stl='libstdc++',
                 # Disabled due to https://bugs.llvm.org/show_bug.cgi?id=41625.
                 use_precompiled_headers_in_tests=False)
add_ubuntu_tests(ubuntu_version='19.04', compiler='clang-8.0', stl='libc++',
                 # Disabled due to https://bugs.llvm.org/show_bug.cgi?id=41625.
                 use_precompiled_headers_in_tests=False)

add_ubuntu_tests(ubuntu_version='18.10', compiler='gcc-8', asan=False, ubsan=False)
add_ubuntu_tests(ubuntu_version='18.10', compiler='clang-4.0', stl='libstdc++')
add_ubuntu_tests(ubuntu_version='18.10', compiler='clang-7.0', stl='libstdc++',
                 # Disabled due to https://bugs.llvm.org/show_bug.cgi?id=41625.
                 use_precompiled_headers_in_tests=False)

add_bazel_tests(ubuntu_version='18.04', smoke_tests=['DebugPlain'])
add_bazel_tests(ubuntu_version='16.04')

# ASan/UBSan are disabled for all these, the analysis on later versions is better anyway.
# Also, in some combinations they wouldn't work.
add_ubuntu_tests(ubuntu_version='16.04', compiler='gcc-5', asan=False, ubsan=False)
add_ubuntu_tests(ubuntu_version='16.04', compiler='clang-3.5', stl='libstdc++', asan=False, ubsan=False)
add_ubuntu_tests(ubuntu_version='16.04', compiler='clang-3.9', stl='libstdc++', asan=False, ubsan=False)

# Asan/Ubsan are disabled because it generates lots of warnings like:
#    warning: direct access in [...] to global weak symbol guard variable for [...] means the weak symbol cannot be
#    overridden at runtime. This was likely caused by different translation units being compiled with different
#    visibility settings.
# and the build eventually fails or times out.
add_osx_tests(compiler='gcc-6', xcode_version='11.4', asan=False, ubsan=False)
add_osx_tests(compiler='gcc-9', xcode_version='11.4', asan=False, ubsan=False, smoke_tests=['DebugPlain'])
add_osx_tests(compiler='clang-4.0', xcode_version='11.4', stl='libc++')
add_osx_tests(compiler='clang-8.0', xcode_version='11.4', stl='libc++', smoke_tests=['DebugPlain'],
              # Disabled due to https://bugs.llvm.org/show_bug.cgi?id=41625.
              use_precompiled_headers_in_tests=False)

# UBSan is disabled because AppleClang does not support -fsanitize=undefined.
add_osx_tests(compiler='clang-default', xcode_version='8.3', stl='libc++', ubsan=False)

add_osx_tests(compiler='clang-default', xcode_version='9.4', stl='libc++')
add_osx_tests(compiler='clang-default', xcode_version='10.3', stl='libc++', smoke_tests=['DebugPlain'])
add_osx_tests(compiler='clang-default', xcode_version='11.4', stl='libc++', smoke_tests=['DebugPlain'])

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
#   add_osx_tests('clang-3.5', stl='libc++', asan=False, ubsan=False)
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
    'services': ['docker'],
    'language': 'cpp',
    'branches': {
        'only': ['master'],
    },
    'matrix': {
        'fast_finish': True,
        'include': build_matrix_smoke_test_rows + build_matrix_rows,
    },
}


class CustomDumper(yaml.SafeDumper):
    def ignore_aliases(self, _data):
        return True


print('#')
print('# This file was auto-generated from extras/scripts/travis_yml_generator.py, DO NOT EDIT')
print('#')
print(yaml.dump(yaml_file, default_flow_style=False, Dumper=CustomDumper))
