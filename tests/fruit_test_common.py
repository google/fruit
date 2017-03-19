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

import itertools

import subprocess
from fruit_test_config import *

class CommandFailedException(Exception):
    def __init__(self, command, stdout, stderr, error_code):
        self.command = command
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
        ''').format(command=self.command, error_code=self.error_code, stdout=self.stdout, stderr=self.stderr)

def run_command(executable, args=[]):
    command = [executable] + args
    try:
        p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        (stdout, stderr) = p.communicate()
    except Exception as e:
        raise Exception("While executing: %s" % command)
    if p.returncode != 0:
        raise CommandFailedException(command, stdout, stderr, p.returncode)
    return (stdout, stderr)

class CompilationFailedException(Exception):
    def __init__(self, command, error_message):
        self.command = command
        self.error_message = error_message

    def __str__(self):
        return textwrap.dedent('''\
        Ran command: {command}
        Error message:
        {error_message}
        ''').format(command=self.command, error_message=self.error_message)

class PosixCompiler:
    def __init__(self):
        self.executable = CXX
        self.name = CXX_COMPILER_NAME

    def compile_discarding_output(self, source, include_dirs, args=[]):
        try:
            self._compile(
                include_dirs,
                args = (
                    args
                    + ['-c', source]
                    + ['-o', os.path.devnull]
                ))
        except CommandFailedException as e:
            raise CompilationFailedException(e.command, e.stderr)

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
            + ['-g0']
            + args
        )
        run_command(self.executable, args)

class MsvcCompiler:
    def __init__(self):
        self.executable = CXX
        self.name = CXX_COMPILER_NAME

    def compile_discarding_output(self, source, include_dirs, args=[]):
        try:
            self._compile(
                include_dirs,
                args = (
                    args
                    + ['/c', source]
                ))
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
            + args
        )
        run_command(self.executable, args)

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

fruit_tests_include_dirs=[
    PATH_TO_FRUIT_TEST_HEADERS,
    PATH_TO_FRUIT_STATIC_HEADERS,
    PATH_TO_FRUIT_GENERATED_HEADERS,
]

_assert_helper = unittest.TestCase()

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

def expect_compile_error(expected_fruit_error_regex, expected_fruit_error_desc_regex, setup_source_code, source_code, test_params={}):
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
    if '\n' in expected_fruit_error_regex:
        raise Exception('expected_fruit_error_regex should not contain newlines')
    if '\n' in expected_fruit_error_desc_regex:
        raise Exception('expected_fruit_error_desc_regex should not contain newlines')

    expected_fruit_error_regex = _replace_using_test_params(expected_fruit_error_regex, test_params)
    expected_fruit_error_regex = expected_fruit_error_regex.replace(' ', '')
    source_code = _construct_final_source_code(setup_source_code, source_code, test_params)

    source_file_name = _create_temporary_file(source_code, file_name_suffix='.cpp')

    try:
        compiler.compile_discarding_output(source=source_file_name, include_dirs=fruit_tests_include_dirs)
        raise Exception('The test should have failed to compile, but it compiled successfully')
    except CompilationFailedException as e1:
        e = e1

    error_message = e.error_message
    error_message_lines = error_message.splitlines()
    # Different compilers output a different number of spaces when pretty-printing types.
    # When using libc++, sometimes std::foo identifiers are reported as std::__1::foo.
    normalized_error_message = error_message.replace(' ', '').replace('std::__1::', 'std::')
    normalized_error_message_lines = normalized_error_message.splitlines()
    error_message_head = _cap_to_lines(error_message, 40)

    for line_number, line in enumerate(normalized_error_message_lines):
        match = re.search('fruit::impl::(.*Error<.*>)', line)
        if match:
            actual_fruit_error_line_number = line_number
            actual_fruit_error = match.groups()[0]
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
                try:
                    replacement_lines = []
                    if normalized_error_message_lines[line_number + 1].strip() == 'with':
                        for line in itertools.islice(normalized_error_message_lines, line_number + 3, None):
                            line = line.strip()
                            if line == ']':
                                break
                            if line.endswith(','):
                                line = line[:-1]
                            replacement_lines.append(line)
                    for replacement_line in replacement_lines:
                        match = re.search('([A-Za-z0-9_-]*)=(.*)', replacement_line)
                        if not match:
                            raise Exception('Failed to parse replacement line: %s' % replacement_line) from e
                        (type_variable, type_expression) = match.groups()
                        actual_fruit_error = re.sub(r'\b' + type_variable + r'\b', type_expression, actual_fruit_error)
                except Exception:
                    raise Exception('Failed to parse MSVC template type arguments')
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

    # 6 is just a constant that works for both g++ (<=4.8.3) and clang++ (<=3.5.0). It might need to be changed.
    if actual_fruit_error_line_number > 6 or actual_static_assert_error_line_number > 6:
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

    # Note that we don't delete the temporary file if the test failed. This is intentional, keeping the file around helps debugging the failure.
    os.remove(source_file_name)


def expect_runtime_error(expected_error_regex, setup_source_code, source_code, test_params={}):
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
    if '\n' in expected_error_regex:
        raise Exception('expected_error_regex should not contain newlines')

    expected_error_regex = _replace_using_test_params(expected_error_regex, test_params)
    source_code = _construct_final_source_code(setup_source_code, source_code, test_params)

    source_file_name = _create_temporary_file(source_code, file_name_suffix='.cpp')
    executable_suffix = {'posix': '', 'nt': '.exe'}[os.name]
    output_file_name = _create_temporary_file('', executable_suffix)
    compiler.compile_and_link(
        source=source_file_name, include_dirs=fruit_tests_include_dirs, output_file_name=output_file_name,
        args=fruit_tests_linker_flags)

    try:
        run_command(output_file_name)
        raise Exception('The test should have failed at runtime, but it ran successfully')
    except CommandFailedException as e1:
        e = e1

    stderr = e.stderr
    stderr_head = _cap_to_lines(stderr, 40)

    try:
        regex_search_result = re.search(expected_error_regex, stderr)
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
    os.remove(source_file_name)
    os.remove(output_file_name)


def expect_success(setup_source_code, source_code, test_params={}):
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

    compiler.compile_and_link(
        source=source_file_name, include_dirs=fruit_tests_include_dirs, output_file_name=output_file_name,
        args=fruit_tests_linker_flags)

    if RUN_TESTS_UNDER_VALGRIND.lower() in ('false', 'off', 'no', '0', ''):
        run_command(output_file_name)
    else:
        args = VALGRIND_FLAGS.split() + [output_file_name]
        run_command('valgrind', args)

    # Note that we don't delete the temporary files if the test failed. This is intentional, keeping them around helps debugging the failure.
    os.remove(source_file_name)
    os.remove(output_file_name)

# E.g.
# @params_cartesian_product(
#   (
#     ("prefix name1", A1, B1),
#     ("prefix name2", A2, B2),
#   ),
#   (
#     ("suffix name 1", C1),
#     ("suffix name 2", C2),
#     ("suffix name 3", C3),
#   ))
# Executes the following combinations:
#     ("prefix name1, suffix name 1", A1, B1, C1),
#     ("prefix name2, suffix name 1", A2, B2, C1),
#     ("prefix name1, suffix name 2", A1, B1, C2),
#     ("prefix name2, suffix name 2", A2, B2, C2),
#     ("prefix name1, suffix name 3", A1, B1, C3),
#     ("prefix name2, suffix name 3", A2, B2, C3),
def params_cartesian_product(*param_tuples):
    def decorator(func):
        results = []
        for combination_tuples in itertools.product(*param_tuples):
            combination_name = ', '.join(combination_tuple[0] for combination_tuple in combination_tuples)
            combination_params = (param for combination_tuple in combination_tuples for param in combination_tuple[1:])
            results.append((combination_name,) + tuple(combination_params))
        func.paramList = tuple(results)
        return func
    return decorator
