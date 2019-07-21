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

import os
import tempfile
import unittest
import textwrap
import re
import sys
import shlex

import itertools

import subprocess

from absl.testing import parameterized

from fruit_test_config import *

from absl.testing import absltest

run_under_valgrind = RUN_TESTS_UNDER_VALGRIND.lower() not in ('false', 'off', 'no', '0', '')

def pretty_print_command(command, env):
    return 'cd %s; env -i %s %s' % (
        shlex.quote(env['PWD']),
        ' '.join('%s=%s' % (var_name, shlex.quote(value)) for var_name, value in env.items() if var_name != 'PWD'),
        ' '.join(shlex.quote(x) for x in command))

def multiple_parameters(*param_lists):
    param_lists = [[params if isinstance(params, tuple) else (params,)
                    for params in param_list]
                   for param_list in param_lists]
    result = param_lists[0]
    for param_list in param_lists[1:]:
        result = [(*args1, *args2)
                  for args1 in result
                  for args2 in param_list]
    return parameterized.parameters(*result)

def multiple_named_parameters(*param_lists):
    result = param_lists[0]
    for param_list in param_lists[1:]:
        result = [(name1 + ', ' + name2, *args1, *args2)
                  for name1, *args1 in result
                  for name2, *args2 in param_list]
    return parameterized.named_parameters(*result)

class CommandFailedException(Exception):
    def __init__(self, command, env, stdout, stderr, error_code):
        self.command = command
        self.env = env
        self.stdout = stdout
        self.stderr = stderr
        self.error_code = error_code

    def __str__(self):
        return textwrap.dedent('''\
        Ran command: {command}
        Exit code {error_code}
        Stdout:
        {stdout}

        Stderr:
        {stderr}
        ''').format(command=pretty_print_command(self.command, self.env), error_code=self.error_code, stdout=self.stdout, stderr=self.stderr)

def run_command(executable, args=[], modify_env=lambda env: env):
    command = [executable] + args
    modified_env = modify_env(os.environ)
    print('Executing command:', pretty_print_command(command, modified_env))
    try:
        p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, env=modified_env)
        (stdout, stderr) = p.communicate()
    except Exception as e:
        raise Exception("While executing: %s" % command)
    if p.returncode != 0:
        raise CommandFailedException(command, modified_env, stdout, stderr, p.returncode)
    print('Execution successful.')
    print('stdout:')
    print(stdout)
    print('')
    print('stderr:')
    print(stderr)
    print('')
    return (stdout, stderr)

def run_compiled_executable(executable):
    if run_under_valgrind:
        args = VALGRIND_FLAGS.split() + [executable]
        run_command('valgrind', args = args, modify_env = modify_env_for_compiled_executables)
    else:
        run_command(executable, modify_env = modify_env_for_compiled_executables)

class CompilationFailedException(Exception):
    def __init__(self, command, env, error_message):
        self.command = command
        self.env = env
        self.error_message = error_message

    def __str__(self):
        return textwrap.dedent('''\
        Ran command: {command}
        Error message:
        {error_message}
        ''').format(command=pretty_print_command(self.command, self.env), error_message=self.error_message)

class PosixCompiler:
    def __init__(self):
        self.executable = CXX
        self.name = CXX_COMPILER_NAME

    def compile_discarding_output(self, source, include_dirs, args=[]):
        try:
            args = args + ['-c', source, '-o', os.path.devnull]
            self._compile(include_dirs, args=args)
        except CommandFailedException as e:
            raise CompilationFailedException(e.command, e.env, e.stderr)

    def compile_and_link(self, source, include_dirs, output_file_name, args=[]):
        self._compile(
            include_dirs,
            args = (
                [source]
                + ADDITIONAL_LINKER_FLAGS.split()
                + args
                + ['-o', output_file_name]
            ))

    def _compile(self, include_dirs, args):
        include_flags = ['-I%s' % include_dir for include_dir in include_dirs]
        args = (
            FRUIT_COMPILE_FLAGS.split()
            + include_flags
            + ['-g0', '-Werror']
            + args
        )
        run_command(self.executable, args)

    def get_disable_deprecation_warning_flags(self):
        return ['-Wno-deprecated-declarations']

    def get_disable_all_warnings_flags(self):
        return ['-Wno-error']

class MsvcCompiler:
    def __init__(self):
        self.executable = CXX
        self.name = CXX_COMPILER_NAME

    def compile_discarding_output(self, source, include_dirs, args=[]):
        try:
            args = args + ['/c', source]
            self._compile(include_dirs, args = args)
        except CommandFailedException as e:
            # Note that we use stdout here, unlike above. MSVC reports compilation warnings and errors on stdout.
            raise CompilationFailedException(e.command, e.stdout)

    def compile_and_link(self, source, include_dirs, output_file_name, args=[]):
        self._compile(
            include_dirs,
            args = (
                [source]
                + ADDITIONAL_LINKER_FLAGS.split()
                + args
                + ['/Fe' + output_file_name]
            ))

    def _compile(self, include_dirs, args):
        include_flags = ['-I%s' % include_dir for include_dir in include_dirs]
        args = (
            FRUIT_COMPILE_FLAGS.split()
            + include_flags
            + ['/WX']
            + args
        )
        run_command(self.executable, args)

    def get_disable_deprecation_warning_flags(self):
        return ['/wd4996']

    def get_disable_all_warnings_flags(self):
        return ['/WX:NO']

if CXX_COMPILER_NAME == 'MSVC':
    compiler = MsvcCompiler()
    if PATH_TO_COMPILED_FRUIT_LIB.endswith('.dll'):
        path_to_fruit_lib = PATH_TO_COMPILED_FRUIT_LIB[:-4] + '.lib'
    else:
        path_to_fruit_lib = PATH_TO_COMPILED_FRUIT_LIB
    fruit_tests_linker_flags = [path_to_fruit_lib]
    fruit_error_message_extraction_regex = 'error C2338: (.*)'
else:
    compiler = PosixCompiler()
    fruit_tests_linker_flags = [
        '-lfruit',
        '-L' + PATH_TO_COMPILED_FRUIT,
        '-Wl,-rpath,' + PATH_TO_COMPILED_FRUIT,
    ]
    fruit_error_message_extraction_regex = 'static.assert(.*)'

fruit_tests_include_dirs = ADDITIONAL_INCLUDE_DIRS.splitlines() + [
    PATH_TO_FRUIT_TEST_HEADERS,
    PATH_TO_FRUIT_STATIC_HEADERS,
    PATH_TO_FRUIT_GENERATED_HEADERS,
]

_assert_helper = unittest.TestCase()

def modify_env_for_compiled_executables(env):
    env = env.copy()
    path_to_fruit_lib_dir = os.path.dirname(PATH_TO_COMPILED_FRUIT_LIB)
    print('PATH_TO_COMPILED_FRUIT_LIB:', PATH_TO_COMPILED_FRUIT_LIB)
    print('Adding directory to PATH:', path_to_fruit_lib_dir)
    env["PATH"] += os.pathsep + path_to_fruit_lib_dir
    return env

def _create_temporary_file(file_content, file_name_suffix=''):
    file_descriptor, file_name = tempfile.mkstemp(text=True, suffix=file_name_suffix)
    file = os.fdopen(file_descriptor, mode='w')
    file.write(file_content)
    file.close()
    return file_name

def _cap_to_lines(s, n):
    lines = s.splitlines()
    if len(lines) <= n:
        return s
    else:
        return '\n'.join(lines[0:n] + ['...'])

def _replace_using_test_params(s, test_params):
    for var_name, value in test_params.items():
        if isinstance(value, str):
            s = re.sub(r'\b%s\b' % var_name, value, s)
    return s

def _construct_final_source_code(setup_source_code, source_code, test_params):
    setup_source_code = textwrap.dedent(setup_source_code)
    source_code = textwrap.dedent(source_code)
    source_code = _replace_using_test_params(source_code, test_params)
    return setup_source_code + source_code

def try_remove_temporary_file(filename):
    try:
        os.remove(filename)
    except:
        # When running Fruit tests on Windows using Appveyor, the remove command fails for temporary files sometimes.
        # This shouldn't cause the tests to fail, so we ignore the exception and go ahead.
        pass

def normalize_error_message_lines(lines):
    # Different compilers output a different number of spaces when pretty-printing types.
    # When using libc++, sometimes std::foo identifiers are reported as std::__1::foo.
    return [line.replace(' ', '').replace('std::__1::', 'std::') for line in lines]

def expect_compile_error_helper(
        check_error_fun,
        setup_source_code,
        source_code,
        test_params={},
        ignore_deprecation_warnings=False,
        ignore_warnings=False):
    source_code = _construct_final_source_code(setup_source_code, source_code, test_params)

    source_file_name = _create_temporary_file(source_code, file_name_suffix='.cpp')

    try:
        args = []
        if ignore_deprecation_warnings:
            args += compiler.get_disable_deprecation_warning_flags()
        if ignore_warnings:
            args += compiler.get_disable_all_warnings_flags()
        if ENABLE_COVERAGE:
            # When collecting coverage these arguments are enabled by default; however we must disable them in tests
            # expected to fail at compile-time because GCC would otherwise fail with an error like:
            # /tmp/tmp4m22cey7.cpp:1:0: error: cannot open /dev/null.gcno
            args += ['-fno-profile-arcs', '-fno-test-coverage']
        compiler.compile_discarding_output(
            source=source_file_name,
            include_dirs=fruit_tests_include_dirs,
            args=args)
        raise Exception('The test should have failed to compile, but it compiled successfully')
    except CompilationFailedException as e1:
        e = e1

    error_message = e.error_message
    error_message_lines = error_message.splitlines()
    error_message_lines = error_message.splitlines()
    error_message_head = _cap_to_lines(error_message, 40)

    check_error_fun(e, error_message_lines, error_message_head)

    try_remove_temporary_file(source_file_name)

def apply_any_error_context_replacements(error_string, following_lines):
    if CXX_COMPILER_NAME == 'MSVC':
        # MSVC errors are of the form:
        #
        # C:\Path\To\header\foo.h(59): note: see reference to class template instantiation 'fruit::impl::NoBindingFoundError<fruit::Annotated<Annotation,U>>' being compiled
        #         with
        #         [
        #              Annotation=Annotation1,
        #              U=std::function<std::unique_ptr<ScalerImpl,std::default_delete<ScalerImpl>> (double)>
        #         ]
        #
        # So we need to parse the following few lines and use them to replace the placeholder types in the Fruit error type.
        replacement_lines = []
        if len(following_lines) >= 4 and following_lines[0].strip() == 'with':
            assert following_lines[1].strip() == '[', 'Line was: ' + following_lines[1]
            for line in itertools.islice(following_lines, 2, None):
                line = line.strip()
                if line == ']':
                    break
                if line.endswith(','):
                    line = line[:-1]
                replacement_lines.append(line)

        for replacement_line in replacement_lines:
            match = re.search('([A-Za-z0-9_-]*)=(.*)', replacement_line)
            if not match:
                raise Exception('Failed to parse replacement line: %s' % replacement_line)
            (type_variable, type_expression) = match.groups()
            error_string = re.sub(r'\b' + type_variable + r'\b', type_expression, error_string)
    return error_string

def expect_generic_compile_error(expected_error_regex, setup_source_code, source_code, test_params={}):
    """
    Tests that the given source produces the expected error during compilation.

    :param expected_fruit_error_regex: A regex used to match the Fruit error type,
           e.g. 'NoBindingFoundForAbstractClassError<ScalerImpl>'.
           Any identifiers contained in the regex will be replaced using test_params (where a replacement is defined).
    :param expected_fruit_error_desc_regex: A regex used to match the Fruit error description,
           e.g. 'No explicit binding was found for C, and C is an abstract class'.
    :param setup_source_code: The first part of the source code. This is dedented separately from source_code and it's
           *not* subject to test_params, unlike source_code.
    :param source_code: The second part of the source code. Any identifiers will be replaced using test_params
           (where a replacement is defined). This will be dedented.
    :param test_params: A dict containing the definition of some identifiers. Each identifier in
           expected_fruit_error_regex and source_code will be replaced (textually) with its definition (if a definition
           was provided).
    """

    expected_error_regex = _replace_using_test_params(expected_error_regex, test_params)
    expected_error_regex = expected_error_regex.replace(' ', '')

    def check_error(e, error_message_lines, error_message_head):
        error_message_lines_with_replacements = [
            apply_any_error_context_replacements(line, error_message_lines[line_number + 1:])
            for line_number, line in enumerate(error_message_lines)]

        normalized_error_message_lines = normalize_error_message_lines(error_message_lines_with_replacements)

        for line in normalized_error_message_lines:
            if re.search(expected_error_regex, line):
                return
        raise Exception(textwrap.dedent('''\
            Expected error {expected_error} but the compiler output did not contain that.
            Compiler command line: {compiler_command}
            Error message was:
            {error_message}
            ''').format(expected_error = expected_error_regex, compiler_command=e.command, error_message = error_message_head))

    expect_compile_error_helper(check_error, setup_source_code, source_code, test_params)

def expect_compile_error(
        expected_fruit_error_regex,
        expected_fruit_error_desc_regex,
        setup_source_code,
        source_code,
        test_params={},
        ignore_deprecation_warnings=False,
        ignore_warnings=False,
        disable_error_line_number_check=False):
    """
    Tests that the given source produces the expected error during compilation.

    :param expected_fruit_error_regex: A regex used to match the Fruit error type,
           e.g. 'NoBindingFoundForAbstractClassError<ScalerImpl>'.
           Any identifiers contained in the regex will be replaced using test_params (where a replacement is defined).
    :param expected_fruit_error_desc_regex: A regex used to match the Fruit error description,
           e.g. 'No explicit binding was found for C, and C is an abstract class'.
    :param setup_source_code: The first part of the source code. This is dedented separately from source_code and it's
           *not* subject to test_params, unlike source_code.
    :param source_code: The second part of the source code. Any identifiers will be replaced using test_params
           (where a replacement is defined). This will be dedented.
    :param test_params: A dict containing the definition of some identifiers. Each identifier in
           expected_fruit_error_regex and source_code will be replaced (textually) with its definition (if a definition
           was provided).
    :param ignore_deprecation_warnings: A boolean. If True, deprecation warnings will be ignored.
    :param ignore_warnings: A boolean. If True, all warnings will be ignored.
    :param disable_error_line_number_check: A boolean. If True, the test will not fail if there are other diagnostic
           lines before the expected error.
    """
    if '\n' in expected_fruit_error_regex:
        raise Exception('expected_fruit_error_regex should not contain newlines')
    if '\n' in expected_fruit_error_desc_regex:
        raise Exception('expected_fruit_error_desc_regex should not contain newlines')

    expected_fruit_error_regex = _replace_using_test_params(expected_fruit_error_regex, test_params)
    expected_fruit_error_regex = expected_fruit_error_regex.replace(' ', '')

    def check_error(e, error_message_lines, error_message_head):
        normalized_error_message_lines = normalize_error_message_lines(error_message_lines)

        for line_number, line in enumerate(normalized_error_message_lines):
            match = re.search('fruit::impl::(.*Error<.*>)', line)
            if match:
                actual_fruit_error_line_number = line_number
                actual_fruit_error = match.groups()[0]
                actual_fruit_error = apply_any_error_context_replacements(actual_fruit_error, normalized_error_message_lines[line_number + 1:])
                break
        else:
            raise Exception(textwrap.dedent('''\
                Expected error {expected_error} but the compiler output did not contain user-facing Fruit errors.
                Compiler command line: {compiler_command}
                Error message was:
                {error_message}
                ''').format(expected_error = expected_fruit_error_regex, compiler_command = e.command, error_message = error_message_head))

        for line_number, line in enumerate(error_message_lines):
            match = re.search(fruit_error_message_extraction_regex, line)
            if match:
                actual_static_assert_error_line_number = line_number
                actual_static_assert_error = match.groups()[0]
                break
        else:
            raise Exception(textwrap.dedent('''\
                Expected error {expected_error} but the compiler output did not contain static_assert errors.
                Compiler command line: {compiler_command}
                Error message was:
                {error_message}
                ''').format(expected_error = expected_fruit_error_regex, compiler_command=e.command, error_message = error_message_head))

        try:
            regex_search_result = re.search(expected_fruit_error_regex, actual_fruit_error)
        except Exception as e:
            raise Exception('re.search() failed for regex \'%s\'' % expected_fruit_error_regex) from e
        if not regex_search_result:
            raise Exception(textwrap.dedent('''\
                The compilation failed as expected, but with a different error type.
                Expected Fruit error type:    {expected_fruit_error_regex}
                Error type was:               {actual_fruit_error}
                Expected static assert error: {expected_fruit_error_desc_regex}
                Static assert was:            {actual_static_assert_error}
                Error message was:
                {error_message}
                '''.format(
                expected_fruit_error_regex = expected_fruit_error_regex,
                actual_fruit_error = actual_fruit_error,
                expected_fruit_error_desc_regex = expected_fruit_error_desc_regex,
                actual_static_assert_error = actual_static_assert_error,
                error_message = error_message_head)))
        try:
            regex_search_result = re.search(expected_fruit_error_desc_regex, actual_static_assert_error)
        except Exception as e:
            raise Exception('re.search() failed for regex \'%s\'' % expected_fruit_error_desc_regex) from e
        if not regex_search_result:
            raise Exception(textwrap.dedent('''\
                The compilation failed as expected, but with a different error message.
                Expected Fruit error type:    {expected_fruit_error_regex}
                Error type was:               {actual_fruit_error}
                Expected static assert error: {expected_fruit_error_desc_regex}
                Static assert was:            {actual_static_assert_error}
                Error message:
                {error_message}
                '''.format(
                expected_fruit_error_regex = expected_fruit_error_regex,
                actual_fruit_error = actual_fruit_error,
                expected_fruit_error_desc_regex = expected_fruit_error_desc_regex,
                actual_static_assert_error = actual_static_assert_error,
                error_message = error_message_head)))

        # 6 is just a constant that works for both g++ (<=6.0.0 at least) and clang++ (<=4.0.0 at least).
        # It might need to be changed.
        if not disable_error_line_number_check and (actual_fruit_error_line_number > 6 or actual_static_assert_error_line_number > 6):
            raise Exception(textwrap.dedent('''\
                The compilation failed with the expected message, but the error message contained too many lines before the relevant ones.
                The error type was reported on line {actual_fruit_error_line_number} of the message (should be <=6).
                The static assert was reported on line {actual_static_assert_error_line_number} of the message (should be <=6).
                Error message:
                {error_message}
                '''.format(
                actual_fruit_error_line_number = actual_fruit_error_line_number,
                actual_static_assert_error_line_number = actual_static_assert_error_line_number,
                error_message = error_message_head)))

        for line in error_message_lines[:max(actual_fruit_error_line_number, actual_static_assert_error_line_number)]:
            if re.search('fruit::impl::meta', line):
                raise Exception(
                    'The compilation failed with the expected message, but the error message contained some metaprogramming types in the output (besides Error). Error message:\n%s' + error_message_head)

    expect_compile_error_helper(check_error, setup_source_code, source_code, test_params, ignore_deprecation_warnings, ignore_warnings)


def expect_runtime_error(
        expected_error_regex,
        setup_source_code,
        source_code,
        test_params={},
        ignore_deprecation_warnings=False):
    """
    Tests that the given source (compiles successfully and) produces the expected error at runtime.

    :param expected_error_regex: A regex used to match the content of stderr.
           Any identifiers contained in the regex will be replaced using test_params (where a replacement is defined).
    :param setup_source_code: The first part of the source code. This is dedented separately from source_code and it's
           *not* subject to test_params, unlike source_code.
    :param source_code: The second part of the source code. Any identifiers will be replaced using test_params
           (where a replacement is defined). This will be dedented.
    :param test_params: A dict containing the definition of some identifiers. Each identifier in
           expected_error_regex and source_code will be replaced (textually) with its definition (if a definition
           was provided).
    """
    expected_error_regex = _replace_using_test_params(expected_error_regex, test_params)
    source_code = _construct_final_source_code(setup_source_code, source_code, test_params)

    source_file_name = _create_temporary_file(source_code, file_name_suffix='.cpp')
    executable_suffix = {'posix': '', 'nt': '.exe'}[os.name]
    output_file_name = _create_temporary_file('', executable_suffix)

    args = fruit_tests_linker_flags.copy()
    if ignore_deprecation_warnings:
        args += compiler.get_disable_deprecation_warning_flags()
    compiler.compile_and_link(
        source=source_file_name,
        include_dirs=fruit_tests_include_dirs,
        output_file_name=output_file_name,
        args=args)

    try:
        run_compiled_executable(output_file_name)
        raise Exception('The test should have failed at runtime, but it ran successfully')
    except CommandFailedException as e1:
        e = e1

    stderr = e.stderr
    stderr_head = _cap_to_lines(stderr, 40)

    if '\n' in expected_error_regex:
        regex_flags = re.MULTILINE
    else:
        regex_flags = 0

    try:
        regex_search_result = re.search(expected_error_regex, stderr, flags=regex_flags)
    except Exception as e:
        raise Exception('re.search() failed for regex \'%s\'' % expected_error_regex) from e
    if not regex_search_result:
        raise Exception(textwrap.dedent('''\
            The test failed as expected, but with a different message.
            Expected: {expected_error_regex}
            Was:
            {stderr}
            '''.format(expected_error_regex = expected_error_regex, stderr = stderr_head)))

    # Note that we don't delete the temporary files if the test failed. This is intentional, keeping them around helps debugging the failure.
    if not ENABLE_COVERAGE:
        try_remove_temporary_file(source_file_name)
        try_remove_temporary_file(output_file_name)


def expect_success(setup_source_code, source_code, test_params={}, ignore_deprecation_warnings=False):
    """
    Tests that the given source compiles and runs successfully.

    :param setup_source_code: The first part of the source code. This is dedented separately from source_code and it's
           *not* subject to test_params, unlike source_code.
    :param source_code: The second part of the source code. Any identifiers will be replaced using test_params
           (where a replacement is defined). This will be dedented.
    :param test_params: A dict containing the definition of some identifiers. Each identifier in
           source_code will be replaced (textually) with its definition (if a definition was provided).
    """
    source_code = _construct_final_source_code(setup_source_code, source_code, test_params)

    if 'main(' not in source_code:
        source_code += textwrap.dedent('''
            int main() {
            }
            ''')

    source_file_name = _create_temporary_file(source_code, file_name_suffix='.cpp')
    executable_suffix = {'posix': '', 'nt': '.exe'}[os.name]
    output_file_name = _create_temporary_file('', executable_suffix)

    args = fruit_tests_linker_flags.copy()
    if ignore_deprecation_warnings:
        args += compiler.get_disable_deprecation_warning_flags()
    compiler.compile_and_link(
        source=source_file_name,
        include_dirs=fruit_tests_include_dirs,
        output_file_name=output_file_name,
        args=args)

    run_compiled_executable(output_file_name)

    # Note that we don't delete the temporary files if the test failed. This is intentional, keeping them around helps debugging the failure.
    if not ENABLE_COVERAGE:
        try_remove_temporary_file(source_file_name)
        try_remove_temporary_file(output_file_name)


# Note: this is not the main function of this file, it's meant to be used as main function from test_*.py files.
def main():
    absltest.main(*sys.argv)
