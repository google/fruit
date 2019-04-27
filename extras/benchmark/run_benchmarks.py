#!/usr/bin/env python3
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
import re
import textwrap
from collections import defaultdict
from timeit import default_timer as timer
import tempfile
import os
import shutil
import itertools
import numpy
import subprocess
import yaml
from numpy import floor, log10
import scipy
import multiprocessing
import sh
import json
import statsmodels.stats.api as stats
from generate_benchmark import generate_benchmark
import git
from functools import lru_cache as memoize

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

def run_command(executable, args=[], cwd=None, env=None):
    args = [str(arg) for arg in args]
    command = [executable] + args
    try:
        p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, cwd=cwd,
                             env=env)
        (stdout, stderr) = p.communicate()
    except Exception as e:
        raise Exception("While executing: %s" % command)
    if p.returncode != 0:
        raise CommandFailedException(command, stdout, stderr, p.returncode)
    return (stdout, stderr)

compile_flags = ['-O2', '-DNDEBUG']

make_args = ['-j', multiprocessing.cpu_count() + 1]

def parse_results(result_lines):
    """
     Parses results from the format:
     ['Dimension name1        = 123',
      'Long dimension name2   = 23.45']

     Into a dict {'Dimension name1': 123.0, 'Dimension name2': 23.45}
    """
    result_dict = dict()
    for line in result_lines:
        line_splits = line.split('=')
        metric = line_splits[0].strip()
        value = float(line_splits[1].strip())
        result_dict[metric] = value
    return result_dict


# We memoize the result since this might be called repeatedly and it's somewhat expensive.
@memoize(maxsize=None)
def determine_compiler_name(compiler_executable_name):
    tmpdir = tempfile.gettempdir() + '/fruit-determine-compiler-version-dir'
    ensure_empty_dir(tmpdir)
    with open(tmpdir + '/CMakeLists.txt', 'w') as file:
        file.write('message("@@@${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}@@@")\n')
    modified_env = os.environ.copy()
    modified_env['CXX'] = compiler_executable_name
    # By converting to a list, we force all output to be read (so the command execution is guaranteed to be complete after this line).
    # Otherwise, subsequent calls to determine_compiler_name might have trouble deleting the temporary directory because the cmake
    # process is still writing files in there.
    _, stderr = run_command('cmake', args=['.'], cwd=tmpdir, env=modified_env)
    cmake_output = stderr.splitlines()
    for line in cmake_output:
        re_result = re.search('@@@(.*)@@@', line)
        if re_result:
            pretty_name = re_result.group(1)
            # CMake calls GCC 'GNU', change it into 'GCC'.
            return pretty_name.replace('GNU ', 'GCC ')
    raise Exception('Unable to determine compiler. CMake output was: \n', cmake_output)


# Returns a pair (sha256_hash, version_name), where version_name will be None if no version tag was found at HEAD.
@memoize(maxsize=None)
def git_repo_info(repo_path):
    repo = git.Repo(repo_path)
    head_tags = [tag.name for tag in repo.tags if tag.commit == repo.head.commit and re.match('v[0-9].*', tag.name)]
    if head_tags == []:
        head_tag = None
    else:
        # There should be only 1 version at any given commit.
        [head_tag] = head_tags
        # Remove the 'v' prefix.
        head_tag = head_tag[1:]
    return (repo.head.commit.hexsha, head_tag)


# Some benchmark parameters, e.g. 'compiler_name' are synthesized automatically from other dimensions (e.g. 'compiler' dimension) or from the environment.
# We put the compiler name/version in the results because the same 'compiler' value might refer to different compiler versions
# (e.g. if GCC 6.0.0 is installed when benchmarks are run, then it's updated to GCC 6.0.1 and finally the results are formatted, we
# want the formatted results to say "GCC 6.0.0" instead of "GCC 6.0.1").
def add_synthetic_benchmark_parameters(original_benchmark_parameters, path_to_code_under_test):
    benchmark_params = original_benchmark_parameters.copy()
    benchmark_params['compiler_name'] = determine_compiler_name(original_benchmark_parameters['compiler'])
    if path_to_code_under_test is not None:
        sha256_hash, version_name = git_repo_info(path_to_code_under_test)
        benchmark_params['di_library_git_commit_hash'] = sha256_hash
        if version_name is not None:
            benchmark_params['di_library_version_name'] = version_name
    return benchmark_params


class SimpleNewDeleteRunTimeBenchmark:
    def __init__(self, benchmark_definition, fruit_benchmark_sources_dir):
        self.benchmark_definition = add_synthetic_benchmark_parameters(benchmark_definition, path_to_code_under_test=None)
        self.fruit_benchmark_sources_dir = fruit_benchmark_sources_dir

    def prepare(self):
        cxx_std = self.benchmark_definition['cxx_std']
        num_classes = self.benchmark_definition['num_classes']
        compiler_executable_name = self.benchmark_definition['compiler']

        self.tmpdir = tempfile.gettempdir() + '/fruit-benchmark-dir'
        ensure_empty_dir(self.tmpdir)
        run_command(compiler_executable_name,
                    args=compile_flags + [
                        '-std=%s' % cxx_std,
                        '-DMULTIPLIER=%s' % num_classes,
                        self.fruit_benchmark_sources_dir + '/extras/benchmark/new_delete_benchmark.cpp',
                        '-o',
                        self.tmpdir + '/main',
                    ])

    def run(self):
        loop_factor = self.benchmark_definition['loop_factor']
        stdout, _ = run_command(self.tmpdir + '/main', args = [int(5000000 * loop_factor)])
        return parse_results(stdout.splitlines())

    def describe(self):
        return self.benchmark_definition


class FruitSingleFileCompileTimeBenchmark:
    def __init__(self, benchmark_definition, fruit_sources_dir, fruit_build_dir, fruit_benchmark_sources_dir):
        self.benchmark_definition = add_synthetic_benchmark_parameters(benchmark_definition, path_to_code_under_test=fruit_sources_dir)
        self.fruit_sources_dir = fruit_sources_dir
        self.fruit_build_dir = fruit_build_dir
        self.fruit_benchmark_sources_dir = fruit_benchmark_sources_dir
        num_bindings = self.benchmark_definition['num_bindings']
        assert (num_bindings % 5) == 0, num_bindings

    def prepare(self):
        pass

    def run(self):
        start = timer()
        cxx_std = self.benchmark_definition['cxx_std']
        num_bindings = self.benchmark_definition['num_bindings']
        compiler_executable_name = self.benchmark_definition['compiler']

        run_command(compiler_executable_name,
                    args = compile_flags + [
                        '-std=%s' % cxx_std,
                        '-DMULTIPLIER=%s' % (num_bindings // 5),
                        '-I', self.fruit_sources_dir + '/include',
                        '-I', self.fruit_build_dir + '/include',
                        '-ftemplate-depth=1000',
                        '-c',
                        self.fruit_benchmark_sources_dir + '/extras/benchmark/compile_time_benchmark.cpp',
                        '-o',
                        '/dev/null',
                    ])
        end = timer()
        return {"compile_time": end - start}

    def describe(self):
        return self.benchmark_definition


def ensure_empty_dir(dirname):
    # We start by creating the directory instead of just calling rmtree with ignore_errors=True because that would ignore
    # all errors, so we might otherwise go ahead even if the directory wasn't properly deleted.
    os.makedirs(dirname, exist_ok=True)
    shutil.rmtree(dirname)
    os.makedirs(dirname)


class GenericGeneratedSourcesBenchmark:
    def __init__(self,
                 di_library,
                 benchmark_definition,
                 path_to_code_under_test=None,
                 **other_args):
        self.di_library = di_library
        self.benchmark_definition = add_synthetic_benchmark_parameters(benchmark_definition, path_to_code_under_test=path_to_code_under_test)
        self.other_args = other_args
        self.arbitrary_file = None

    def prepare_compile_benchmark(self):
        num_classes = self.benchmark_definition['num_classes']
        cxx_std = self.benchmark_definition['cxx_std']
        compiler_executable_name = self.benchmark_definition['compiler']
        benchmark_generation_flags = {flag_name: True for flag_name in self.benchmark_definition['benchmark_generation_flags']}

        self.tmpdir = tempfile.gettempdir() + '/fruit-benchmark-dir'
        ensure_empty_dir(self.tmpdir)
        num_classes_with_no_deps = int(num_classes * 0.1)
        return generate_benchmark(
            compiler=compiler_executable_name,
            num_components_with_no_deps=num_classes_with_no_deps,
            num_components_with_deps=num_classes - num_classes_with_no_deps,
            num_deps=10,
            output_dir=self.tmpdir,
            cxx_std=cxx_std,
            di_library=self.di_library,
            **benchmark_generation_flags,
            **self.other_args)

    def run_make_build(self):
        run_command('make', args=make_args, cwd=self.tmpdir)

    def prepare_incremental_compile_benchmark(self):
        files = self.prepare_compile_benchmark()
        self.run_make_build()
        files = list(sorted(file for file in files if file.endswith('.h')))
        # 5 files, equally spaced (but not at beginning/end) in the sorted sequence.
        num_files_changed = 5
        self.arbitrary_files = [files[i * (len(files) // (num_files_changed + 2))]
                                for i in range(1, num_files_changed + 1)]

    def prepare_runtime_benchmark(self):
        self.prepare_compile_benchmark()
        self.run_make_build()

    def prepare_startup_benchmark(self):
        self.prepare_compile_benchmark()
        self.run_make_build()
        run_command('strip', args=[self.tmpdir + '/main'])

    def prepare_executable_size_benchmark(self):
        self.prepare_runtime_benchmark()
        run_command('strip', args=[self.tmpdir + '/main'])

    def run_compile_benchmark(self):
        run_command('make',
                    args=make_args + ['clean'],
                    cwd=self.tmpdir)
        start = timer()
        self.run_make_build()
        end = timer()
        result = {'compile_time': end - start}
        return result

    def run_incremental_compile_benchmark(self):
        run_command('touch', args=self.arbitrary_files, cwd=self.tmpdir)
        start = timer()
        self.run_make_build()
        end = timer()
        result = {'incremental_compile_time': end - start}
        return result

    def run_runtime_benchmark(self):
        num_classes = self.benchmark_definition['num_classes']
        loop_factor = self.benchmark_definition['loop_factor']

        results, _ = run_command(self.tmpdir + '/main',
                                 args = [
                                     # 40M loops with 100 classes, 40M with 1000
                                     int(4 * 1000 * 1000 * 1000 * loop_factor / num_classes),
                                 ])
        return parse_results(results.splitlines())
    
    def run_startup_benchmark(self):
        n = 1000
        start = timer()
        for i in range(0, n):
            run_command(self.tmpdir + '/main', args = [])
        end = timer()
        result = {'startup_time': (end - start) / n}
        return result

    def run_executable_size_benchmark(self):
        wc_result, _ = run_command('wc', args=['-c', self.tmpdir + '/main'])
        num_bytes = wc_result.splitlines()[0].split(' ')[0]
        return {'num_bytes': float(num_bytes)}

    def describe(self):
        return self.benchmark_definition


class CompileTimeBenchmark(GenericGeneratedSourcesBenchmark):
    def __init__(self, **kwargs):
        super().__init__(generate_runtime_bench_code=False,
                         **kwargs)

    def prepare(self):
        self.prepare_compile_benchmark()

    def run(self):
        return self.run_compile_benchmark()

class IncrementalCompileTimeBenchmark(GenericGeneratedSourcesBenchmark):
    def __init__(self, **kwargs):
        super().__init__(generate_runtime_bench_code=False,
                         **kwargs)

    def prepare(self):
        self.prepare_incremental_compile_benchmark()

    def run(self):
        return self.run_incremental_compile_benchmark()

class StartupTimeBenchmark(GenericGeneratedSourcesBenchmark):
    def __init__(self, **kwargs):
        super().__init__(generate_runtime_bench_code=False,
                         **kwargs)

    def prepare(self):
        self.prepare_startup_benchmark()

    def run(self):
        return self.run_startup_benchmark()

class RunTimeBenchmark(GenericGeneratedSourcesBenchmark):
    def __init__(self, **kwargs):
        super().__init__(generate_runtime_bench_code=True,
                         **kwargs)

    def prepare(self):
        self.prepare_runtime_benchmark()

    def run(self):
        return self.run_runtime_benchmark()

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class ExecutableSizeBenchmark(GenericGeneratedSourcesBenchmark):
    def __init__(self, **kwargs):
        super().__init__(generate_runtime_bench_code=False,
                         **kwargs)

    def prepare(self):
        self.prepare_executable_size_benchmark()

    def run(self):
        return self.run_executable_size_benchmark()

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class ExecutableSizeBenchmarkWithoutExceptionsAndRtti(ExecutableSizeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_exceptions=False,
                         use_rtti=False,
                         **kwargs)

class FruitCompileTimeBenchmark(CompileTimeBenchmark):
    def __init__(self, fruit_sources_dir, **kwargs):
        super().__init__(di_library='fruit',
                         path_to_code_under_test=fruit_sources_dir,
                         fruit_sources_dir=fruit_sources_dir,
                         **kwargs)

class FruitIncrementalCompileTimeBenchmark(IncrementalCompileTimeBenchmark):
    def __init__(self, fruit_sources_dir, **kwargs):
        super().__init__(di_library='fruit',
                         path_to_code_under_test=fruit_sources_dir,
                         fruit_sources_dir=fruit_sources_dir,
                         **kwargs)

class FruitRunTimeBenchmark(RunTimeBenchmark):
    def __init__(self, fruit_sources_dir, **kwargs):
        super().__init__(di_library='fruit',
                         path_to_code_under_test=fruit_sources_dir,
                         fruit_sources_dir=fruit_sources_dir,
                         **kwargs)

class FruitStartupTimeBenchmark(StartupTimeBenchmark):
    def __init__(self, fruit_sources_dir, **kwargs):
        super().__init__(di_library='fruit',
                         path_to_code_under_test=fruit_sources_dir,
                         fruit_sources_dir=fruit_sources_dir,
                         **kwargs)

class FruitStartupTimeWithNormalizedComponentBenchmark(FruitStartupTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_normalized_component=True,
                         **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class FruitExecutableSizeBenchmark(ExecutableSizeBenchmark):
    def __init__(self, fruit_sources_dir, **kwargs):
        super().__init__(di_library='fruit',
                         path_to_code_under_test=fruit_sources_dir,
                         fruit_sources_dir=fruit_sources_dir,
                         **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class FruitExecutableSizeBenchmarkWithoutExceptionsAndRtti(ExecutableSizeBenchmarkWithoutExceptionsAndRtti):
    def __init__(self, fruit_sources_dir, **kwargs):
        super().__init__(di_library='fruit',
                         path_to_code_under_test=fruit_sources_dir,
                         fruit_sources_dir=fruit_sources_dir,
                         **kwargs)

class BoostDiCompileTimeBenchmark(CompileTimeBenchmark):
    def __init__(self, boost_di_sources_dir, **kwargs):
        super().__init__(di_library='boost_di',
                         path_to_code_under_test=boost_di_sources_dir,
                         boost_di_sources_dir=boost_di_sources_dir,
                         **kwargs)

class BoostDiIncrementalCompileTimeBenchmark(IncrementalCompileTimeBenchmark):
    def __init__(self, boost_di_sources_dir, **kwargs):
        super().__init__(di_library='boost_di',
                         path_to_code_under_test=boost_di_sources_dir,
                         boost_di_sources_dir=boost_di_sources_dir,
                         **kwargs)

class BoostDiRunTimeBenchmark(RunTimeBenchmark):
    def __init__(self, boost_di_sources_dir, **kwargs):
        super().__init__(di_library='boost_di',
                         path_to_code_under_test=boost_di_sources_dir,
                         boost_di_sources_dir=boost_di_sources_dir,
                         **kwargs)

class BoostDiStartupTimeBenchmark(StartupTimeBenchmark):
    def __init__(self, boost_di_sources_dir, **kwargs):
        super().__init__(di_library='boost_di',
                         path_to_code_under_test=boost_di_sources_dir,
                         boost_di_sources_dir=boost_di_sources_dir,
                         **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class BoostDiExecutableSizeBenchmark(ExecutableSizeBenchmark):
    def __init__(self, boost_di_sources_dir, **kwargs):
        super().__init__(di_library='boost_di',
                         path_to_code_under_test=boost_di_sources_dir,
                         boost_di_sources_dir=boost_di_sources_dir,
                         **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class BoostDiExecutableSizeBenchmarkWithoutExceptionsAndRtti(ExecutableSizeBenchmarkWithoutExceptionsAndRtti):
    def __init__(self, boost_di_sources_dir, **kwargs):
        super().__init__(di_library='boost_di',
                         path_to_code_under_test=boost_di_sources_dir,
                         boost_di_sources_dir=boost_di_sources_dir,
                         **kwargs)

class SimpleDiCompileTimeBenchmark(CompileTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(di_library='none',
                         **kwargs)

class SimpleDiIncrementalCompileTimeBenchmark(IncrementalCompileTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(di_library='none',
                         **kwargs)

class SimpleDiRunTimeBenchmark(RunTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(di_library='none',
                         **kwargs)

class SimpleDiStartupTimeBenchmark(StartupTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(di_library='none',
                         **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class SimpleDiExecutableSizeBenchmark(ExecutableSizeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(di_library='none',
                         **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class SimpleDiExecutableSizeBenchmarkWithoutExceptionsAndRtti(ExecutableSizeBenchmarkWithoutExceptionsAndRtti):
    def __init__(self, **kwargs):
        super().__init__(di_library='none',
                         **kwargs)

class SimpleDiWithInterfacesCompileTimeBenchmark(SimpleDiCompileTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_interfaces=True, **kwargs)

class SimpleDiWithInterfacesIncrementalCompileTimeBenchmark(SimpleDiIncrementalCompileTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_interfaces=True, **kwargs)

class SimpleDiWithInterfacesRunTimeBenchmark(SimpleDiRunTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_interfaces=True, **kwargs)

class SimpleDiWithInterfacesStartupTimeBenchmark(SimpleDiStartupTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_interfaces=True, **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class SimpleDiWithInterfacesExecutableSizeBenchmark(SimpleDiExecutableSizeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_interfaces=True, **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class SimpleDiWithInterfacesExecutableSizeBenchmarkWithoutExceptionsAndRtti(SimpleDiExecutableSizeBenchmarkWithoutExceptionsAndRtti):
    def __init__(self, **kwargs):
        super().__init__(use_interfaces=True, **kwargs)

class SimpleDiWithInterfacesAndNewDeleteCompileTimeBenchmark(SimpleDiWithInterfacesCompileTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_new_delete=True, **kwargs)

class SimpleDiWithInterfacesAndNewDeleteIncrementalCompileTimeBenchmark(SimpleDiWithInterfacesIncrementalCompileTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_new_delete=True, **kwargs)

class SimpleDiWithInterfacesAndNewDeleteRunTimeBenchmark(SimpleDiWithInterfacesRunTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_new_delete=True, **kwargs)

class SimpleDiWithInterfacesAndNewDeleteStartupTimeBenchmark(SimpleDiWithInterfacesStartupTimeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_new_delete=True, **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class SimpleDiWithInterfacesAndNewDeleteExecutableSizeBenchmark(SimpleDiWithInterfacesExecutableSizeBenchmark):
    def __init__(self, **kwargs):
        super().__init__(use_new_delete=True, **kwargs)

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class SimpleDiWithInterfacesAndNewDeleteExecutableSizeBenchmarkWithoutExceptionsAndRtti(SimpleDiWithInterfacesExecutableSizeBenchmarkWithoutExceptionsAndRtti):
    def __init__(self, **kwargs):
        super().__init__(use_new_delete=True, **kwargs)


def round_to_significant_digits(n, num_significant_digits):
    if n <= 0:
        # We special-case this, otherwise the log10 below will fail.
        return 0
    return round(n, num_significant_digits - int(floor(log10(n))) - 1)

def run_benchmark(benchmark, max_runs, timeout_hours, output_file, min_runs=3):
    def run_benchmark_once():
        print('Running benchmark... ', end='', flush=True)
        result = benchmark.run()
        print(result)
        for dimension, value in result.items():
            results_by_dimension[dimension] += [value]

    results_by_dimension = defaultdict(lambda: [])
    print('Preparing for benchmark... ', end='', flush=True)
    benchmark.prepare()
    print('Done.')

    start_time = timer()

    # Run at least min_runs times
    for i in range(min_runs):
        run_benchmark_once()

    # Then consider running a few more times to get the desired precision.
    while True:
        if timer() - start_time > timeout_hours * 3600:
            print("Warning: timed out, couldn't determine a result with the desired precision.")
            break

        for dimension, results in results_by_dimension.items():
            if all(result == results[0] for result in results):
                # If all results are exactly the same the code below misbehaves. We don't need to run again in this case.
                continue
            confidence_interval = stats.DescrStatsW(results).tconfint_mean(0.05)
            confidence_interval_2dig = (round_to_significant_digits(confidence_interval[0], 2),
                                        round_to_significant_digits(confidence_interval[1], 2))
            if abs(confidence_interval_2dig[0] - confidence_interval_2dig[1]) > numpy.finfo(float).eps * 10:
                if len(results) < max_runs:
                    print("Running again to get more precision on the metric %s. Current confidence interval: [%.3g, %.3g]" % (
                        dimension, confidence_interval[0], confidence_interval[1]))
                    break
                else:
                    print("Warning: couldn't determine a precise result for the metric %s. Confidence interval: [%.3g, %.3g]" % (
                        dimension, confidence_interval[0], confidence_interval[1]))
        else:
            # We've reached sufficient precision in all metrics, or we've reached the max number of runs.
            break

        run_benchmark_once()

    # We've reached the desired precision in all dimensions or reached the maximum number of runs. Record the results.
    rounded_confidence_intervals_by_dimension = {}
    confidence_intervals_by_dimension = {}
    for dimension, results in results_by_dimension.items():
        confidence_interval = stats.DescrStatsW(results).tconfint_mean(0.05)
        confidence_interval_2dig = (round_to_significant_digits(confidence_interval[0], 2),
                                    round_to_significant_digits(confidence_interval[1], 2))
        rounded_confidence_intervals_by_dimension[dimension] = confidence_interval_2dig
        confidence_intervals_by_dimension[dimension] = (confidence_interval, confidence_interval_2dig)
    with open(output_file, 'a') as f:
        json.dump({"benchmark": benchmark.describe(), "results": confidence_intervals_by_dimension}, f)
        print(file=f)
    print('Benchmark finished. Result: ', rounded_confidence_intervals_by_dimension)
    print()


def expand_benchmark_definition(benchmark_definition):
    """
    Takes a benchmark definition, e.g.:
    [{name: 'foo', compiler: ['g++-5', 'g++-6']},
     {name: ['bar', 'baz'], compiler: ['g++-5'], cxx_std: 'c++14'}]

    And expands it into the individual benchmarks to run, in the example above:
    [{name: 'foo', compiler: 'g++-5'},
     {name: 'foo', compiler: 'g++-6'},
     {name: 'bar', compiler: 'g++-5', cxx_std: 'c++14'},
     {name: 'baz', compiler: 'g++-5', cxx_std: 'c++14'}]
    """
    dict_keys = sorted(benchmark_definition.keys())
    # Turn non-list values into single-item lists.
    benchmark_definition = {dict_key: value if isinstance(value, list) else [value]
                            for dict_key, value in benchmark_definition.items()}
    # Compute the cartesian product of the value lists
    value_combinations = itertools.product(*(benchmark_definition[dict_key] for dict_key in dict_keys))
    # Then turn the result back into a dict.
    return [dict(zip(dict_keys, value_combination))
            for value_combination in value_combinations]


def expand_benchmark_definitions(benchmark_definitions):
    return list(itertools.chain(*[expand_benchmark_definition(benchmark_definition) for benchmark_definition in benchmark_definitions]))

def group_by(l, element_to_key):
    """Takes a list and returns a dict of sublists, where the elements are grouped using the provided function"""
    result = defaultdict(list)
    for elem in l:
        result[element_to_key(elem)].append(elem)
    return result.items()

def main():
    # This configures numpy/scipy to raise an exception in case of errors, instead of printing a warning and going ahead.
    numpy.seterr(all='raise')
    scipy.seterr(all='raise')

    parser = argparse.ArgumentParser(description='Runs a set of benchmarks defined in a YAML file.')
    parser.add_argument('--fruit-benchmark-sources-dir', help='Path to the fruit sources (used for benchmarking code only)')
    parser.add_argument('--fruit-sources-dir', help='Path to the fruit sources')
    parser.add_argument('--boost-di-sources-dir', help='Path to the Boost.DI sources')
    parser.add_argument('--output-file',
                        help='The output file where benchmark results will be stored (1 per line, with each line in JSON format). These can then be formatted by e.g. the format_bench_results script.')
    parser.add_argument('--benchmark-definition', help='The YAML file that defines the benchmarks (see fruit_wiki_benchs_fruit.yml for an example).')
    parser.add_argument('--continue-benchmark', help='If this is \'true\', continues a previous benchmark run instead of starting from scratch (taking into account the existing benchmark results in the file specified with --output-file).')
    args = parser.parse_args()

    if args.output_file is None:
        raise Exception('You must specify --output_file')
    if args.continue_benchmark == 'true':
        try:
            with open(args.output_file, 'r') as f:
                previous_run_completed_benchmarks = [json.loads(line)['benchmark'] for line in f.readlines()]
        except FileNotFoundError:
            previous_run_completed_benchmarks = []
    else:
        previous_run_completed_benchmarks = []
        run_command('rm', args=['-f', args.output_file])

    fruit_build_dir = tempfile.gettempdir() + '/fruit-benchmark-build-dir'

    with open(args.benchmark_definition, 'r') as f:
        yaml_file_content = yaml.load(f)
        global_definitions = yaml_file_content['global']
        benchmark_definitions = expand_benchmark_definitions(yaml_file_content['benchmarks'])

    benchmark_index = 0

    for (compiler_executable_name, additional_cmake_args), benchmark_definitions_with_current_config \
            in group_by(benchmark_definitions,
                        lambda benchmark_definition:
                            (benchmark_definition['compiler'], tuple(benchmark_definition['additional_cmake_args']))):

        print('Preparing for benchmarks with the compiler %s, with additional CMake args %s' % (compiler_executable_name, additional_cmake_args))
        # We compute this here (and memoize the result) so that the benchmark's describe() will retrieve the cached
        # value instantly.
        determine_compiler_name(compiler_executable_name)

        # Build Fruit in fruit_build_dir, so that fruit_build_dir points to a built Fruit (useful for e.g. the config header).
        shutil.rmtree(fruit_build_dir, ignore_errors=True)
        os.makedirs(fruit_build_dir)
        modified_env = os.environ.copy()
        modified_env['CXX'] = compiler_executable_name
        run_command('cmake',
                    args=[
                        args.fruit_sources_dir,
                        '-DCMAKE_BUILD_TYPE=Release',
                        *additional_cmake_args,
                    ],
                    cwd=fruit_build_dir,
                    env=modified_env)
        run_command('make', args=make_args, cwd=fruit_build_dir)

        for benchmark_definition in benchmark_definitions_with_current_config:
            benchmark_index += 1
            print('%s/%s: %s' % (benchmark_index, len(benchmark_definitions), benchmark_definition))
            benchmark_name = benchmark_definition['name']

            if (benchmark_name in {'boost_di_compile_time', 'boost_di_run_time', 'boost_di_executable_size'}
                    and args.boost_di_sources_dir is None):
                raise Exception('Error: you need to specify the --boost-di-sources-dir flag in order to run Boost.DI benchmarks.')

            if benchmark_name == 'new_delete_run_time':
                benchmark = SimpleNewDeleteRunTimeBenchmark(
                    benchmark_definition,
                    fruit_benchmark_sources_dir=args.fruit_benchmark_sources_dir)
            elif benchmark_name == 'fruit_single_file_compile_time':
                benchmark = FruitSingleFileCompileTimeBenchmark(
                    benchmark_definition,
                    fruit_sources_dir=args.fruit_sources_dir,
                    fruit_benchmark_sources_dir=args.fruit_benchmark_sources_dir,
                    fruit_build_dir=fruit_build_dir)
            elif benchmark_name.startswith('fruit_'):
                benchmark_class = {
                    'fruit_compile_time': FruitCompileTimeBenchmark,
                    'fruit_incremental_compile_time': FruitIncrementalCompileTimeBenchmark,
                    'fruit_run_time': FruitRunTimeBenchmark,
                    'fruit_startup_time': FruitStartupTimeBenchmark,
                    'fruit_startup_time_with_normalized_component': FruitStartupTimeWithNormalizedComponentBenchmark,
                    'fruit_executable_size': FruitExecutableSizeBenchmark,
                    'fruit_executable_size_without_exceptions_and_rtti': FruitExecutableSizeBenchmarkWithoutExceptionsAndRtti,
                }[benchmark_name]
                benchmark = benchmark_class(
                    benchmark_definition=benchmark_definition,
                    fruit_sources_dir=args.fruit_sources_dir,
                    fruit_build_dir=fruit_build_dir)
            elif benchmark_name.startswith('boost_di_'):
                benchmark_class = {
                    'boost_di_compile_time': BoostDiCompileTimeBenchmark,
                    'boost_di_incremental_compile_time': BoostDiIncrementalCompileTimeBenchmark,
                    'boost_di_run_time': BoostDiRunTimeBenchmark,
                    'boost_di_startup_time': BoostDiStartupTimeBenchmark,
                    'boost_di_executable_size': BoostDiExecutableSizeBenchmark,
                    'boost_di_executable_size_without_exceptions_and_rtti': BoostDiExecutableSizeBenchmarkWithoutExceptionsAndRtti,
                }[benchmark_name]
                benchmark = benchmark_class(
                    benchmark_definition=benchmark_definition,
                    boost_di_sources_dir=args.boost_di_sources_dir)
            elif benchmark_name.startswith('simple_di_'):
                benchmark_class = {
                    'simple_di_compile_time': SimpleDiCompileTimeBenchmark,
                    'simple_di_incremental_compile_time': SimpleDiIncrementalCompileTimeBenchmark,
                    'simple_di_run_time': SimpleDiRunTimeBenchmark,
                    'simple_di_startup_time': SimpleDiStartupTimeBenchmark,
                    'simple_di_executable_size': SimpleDiExecutableSizeBenchmark,
                    'simple_di_executable_size_without_exceptions_and_rtti': SimpleDiExecutableSizeBenchmarkWithoutExceptionsAndRtti,
                    'simple_di_with_interfaces_compile_time': SimpleDiWithInterfacesCompileTimeBenchmark,
                    'simple_di_with_interfaces_incremental_compile_time': SimpleDiWithInterfacesIncrementalCompileTimeBenchmark,
                    'simple_di_with_interfaces_run_time': SimpleDiWithInterfacesRunTimeBenchmark,
                    'simple_di_with_interfaces_startup_time': SimpleDiWithInterfacesStartupTimeBenchmark,
                    'simple_di_with_interfaces_executable_size': SimpleDiWithInterfacesExecutableSizeBenchmark,
                    'simple_di_with_interfaces_executable_size_without_exceptions_and_rtti': SimpleDiWithInterfacesExecutableSizeBenchmarkWithoutExceptionsAndRtti,
                    'simple_di_with_interfaces_and_new_delete_compile_time': SimpleDiWithInterfacesAndNewDeleteCompileTimeBenchmark,
                    'simple_di_with_interfaces_and_new_delete_incremental_compile_time': SimpleDiWithInterfacesAndNewDeleteIncrementalCompileTimeBenchmark,
                    'simple_di_with_interfaces_and_new_delete_run_time': SimpleDiWithInterfacesAndNewDeleteRunTimeBenchmark,
                    'simple_di_with_interfaces_and_new_delete_startup_time': SimpleDiWithInterfacesAndNewDeleteStartupTimeBenchmark,
                    'simple_di_with_interfaces_and_new_delete_executable_size': SimpleDiWithInterfacesAndNewDeleteExecutableSizeBenchmark,
                    'simple_di_with_interfaces_and_new_delete_executable_size_without_exceptions_and_rtti': SimpleDiWithInterfacesAndNewDeleteExecutableSizeBenchmarkWithoutExceptionsAndRtti,
                }[benchmark_name]
                benchmark = benchmark_class(
                    benchmark_definition=benchmark_definition)
            else:
                raise Exception("Unrecognized benchmark: %s" % benchmark_name)

            if benchmark.describe() in previous_run_completed_benchmarks:
                print("Skipping benchmark that was already run previously (due to --continue-benchmark):", benchmark.describe())
                continue

            run_benchmark(benchmark,
                          output_file=args.output_file,
                          max_runs=global_definitions['max_runs'],
                          timeout_hours=global_definitions['max_hours_per_combination'])


if __name__ == "__main__":
    main()
