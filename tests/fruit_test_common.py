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
import sh
from fruit_test_config import *

_assert_helper = unittest.TestCase()

cxx_compile_only_command = sh.Command(CXX).bake(CXXFLAGS.split(), _tty_out=False)
cxx_compile_command = cxx_compile_only_command.bake(LDFLAGS.split())

def _str_or_bytes_to_str(x):
    if x is None:
        return ''
    elif isinstance(x, str):
        return x
    else:
        return x.decode()

def _rethrow_sh_exception(e):
    """Rethrows a sh.ErrorReturnCode exception, printing the entire stdout/stderr.

    If we didn't re-throw the exception this way, sh will clip the command+stdout+stderr message at 750 chars, which
    sometimes makes it hard to understand what the error was.
    """

    # We use str(..., 'utf-8') to convert both str and bytes objects to str.
    stdout = _str_or_bytes_to_str(e.stdout)
    stderr = _str_or_bytes_to_str(e.stderr)

    message = textwrap.dedent('''\
        Ran command: {command}
        Stdout:
        {stdout}

        Stderr:
        {stderr}
        ''').format(command=e.full_cmd, stdout=stdout, stderr=stderr)
    # The "from None" here prevents the original exception from being shown if this function is called in an except
    # clause.
    raise Exception(message) from None

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
        cxx_compile_only_command('-c', source_file_name, o='/dev/null')
        raise Exception('The test should have failed to compile, but it compiled successfully')
    except sh.ErrorReturnCode as e1:
        e = e1

    stderr = _str_or_bytes_to_str(e.stderr)
    stderr_lines = stderr.splitlines()
    # Different compilers output a different number of spaces when pretty-printing types.
    # When using libc++, sometimes std::foo identifiers are reported as std::__1::foo.
    normalized_stderr = stderr.replace(' ', '').replace('std::__1::', 'std::')
    normalized_stderr_lines = normalized_stderr.splitlines()
    stderr_head = _cap_to_lines(stderr, 40)

    for line_number, line in enumerate(normalized_stderr_lines):
        match = re.search('fruit::impl::(.*Error<.*>)', line)
        if match:
            actual_fruit_error_line_number = line_number
            actual_fruit_error = match.groups()[0]
            break
    else:
        raise Exception(textwrap.dedent('''\
            Expected error {expected_error} but the compiler output did not contain user-facing Fruit errors.
            Compiler command line: {compiler_command}
            Stderr was:
            {stderr}
            ''').format(expected_error = expected_fruit_error_regex, compiler_command = e.full_cmd, stderr = stderr_head))

    for line_number, line in enumerate(stderr_lines):
        match = re.search('static.assert(.*)', line)
        if match:
            actual_static_assert_error_line_number = line_number
            actual_static_assert_error = match.groups()[0]
            break
    else:
        raise Exception(textwrap.dedent('''\
            Expected error {expected_error} but the compiler output did not contain static_assert errors.
            Compiler command line: {compiler_command}
            Stderr was:
            {stderr}
            ''').format(expected_error = expected_fruit_error_regex, compiler_command=e.full_cmd, stderr = stderr_head))

    if not re.search(expected_fruit_error_regex, actual_fruit_error):
        raise Exception(textwrap.dedent('''\
            The compilation failed as expected, but with a different error type.
            Expected Fruit error type:    {expected_fruit_error_regex}
            Error type was:               {actual_fruit_error}
            Expected static assert error: {expected_fruit_error_desc_regex}
            Static assert was:            {actual_static_assert_error}
            Stderr:
            {stderr}
            '''.format(
            expected_fruit_error_regex = expected_fruit_error_regex,
            actual_fruit_error = actual_fruit_error,
            expected_fruit_error_desc_regex = expected_fruit_error_desc_regex,
            actual_static_assert_error = actual_static_assert_error,
            stderr = stderr_head)))
    if not re.search(expected_fruit_error_desc_regex, actual_static_assert_error):
        raise Exception(textwrap.dedent('''\
            The compilation failed as expected, but with a different error message.
            Expected Fruit error type:    {expected_fruit_error_regex}
            Error type was:               {actual_fruit_error}
            Expected static assert error: {expected_fruit_error_desc_regex}
            Static assert was:            {actual_static_assert_error}
            Stderr:
            {stderr}
            '''.format(
            expected_fruit_error_regex = expected_fruit_error_regex,
            actual_fruit_error = actual_fruit_error,
            expected_fruit_error_desc_regex = expected_fruit_error_desc_regex,
            actual_static_assert_error = actual_static_assert_error,
            stderr = stderr_head)))

    # 6 is just a constant that works for both g++ (<=4.8.3) and clang++ (<=3.5.0). It might need to be changed.
    if actual_fruit_error_line_number > 6 or actual_static_assert_error_line_number > 6:
        raise Exception(textwrap.dedent('''\
            The compilation failed with the expected message, but the error message contained too many lines before the relevant ones.
            The error type was reported on line {actual_fruit_error_line_number} of the message (should be <=6).
            The static assert was reported on line {actual_static_assert_error_line_number} of the message (should be <=6).
            Stderr:
            {stderr}
            '''.format(
            actual_fruit_error_line_number = actual_fruit_error_line_number,
            actual_static_assert_error_line_number = actual_static_assert_error_line_number,
            stderr = stderr_head)))

    for line in stderr_lines[:max(actual_fruit_error_line_number, actual_static_assert_error_line_number)]:
        if re.search('fruit::impl::meta', line):
            raise Exception(
                'The compilation failed with the expected message, but the error message contained some metaprogramming types in the output (besides Error). Stderr:\n%s' + stderr_head)

    # Note that we don't delete the temporary file if the test failed. This is intentional, keeping the file around helps debugging the failure.
    sh.rm('-f', source_file_name, _tty_out=False)


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
    output_file_name = _create_temporary_file('')
    try:
        cxx_compile_command(source_file_name, '-lfruit', '-lstdc++', o=output_file_name)
    except sh.ErrorReturnCode as e:
        _rethrow_sh_exception(e)

    try:
        sh.Command(output_file_name)(_tty_out=False)
        raise Exception('The test should have failed at runtime, but it ran successfully')
    except sh.ErrorReturnCode as e1:
        e = e1

    stderr = e.stderr.decode()
    stderr_head = _cap_to_lines(stderr, 40)

    if not re.search(expected_error_regex, stderr):
        raise Exception(textwrap.dedent('''\
            The test failed as expected, but with a different message.
            Expected: {expected_error_regex}
            Was:
            {stderr}
            '''.format(expected_error_regex = expected_error_regex, stderr = stderr_head)))

    # Note that we don't delete the temporary files if the test failed. This is intentional, keeping them around helps debugging the failure.
    sh.rm('-f', source_file_name, _tty_out=False)
    sh.rm('-f', output_file_name, _tty_out=False)


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
    output_file_name = _create_temporary_file('')

    try:
        cxx_compile_command(source_file_name, '-lfruit', o=output_file_name)

        if RUN_TESTS_UNDER_VALGRIND.lower() in ('false', 'off', 'no', '0', ''):
            sh.Command(output_file_name)(_tty_out=False)
        else:
            sh.Command('valgrind')(*(VALGRIND_FLAGS.split() + [output_file_name]), _tty_out=False)
    except sh.ErrorReturnCode as e:
        _rethrow_sh_exception(e)

    # Note that we don't delete the temporary files if the test failed. This is intentional, keeping them around helps debugging the failure.
    sh.rm('-f', source_file_name, _tty_out=False)
    sh.rm('-f', output_file_name, _tty_out=False)

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
            results.append((combination_name, *combination_params))
        func.paramList = tuple(results)
        return func
    return decorator
