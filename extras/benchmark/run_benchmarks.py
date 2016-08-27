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
from timeit import default_timer as timer
import tempfile
import os
import shutil
import numpy
from numpy import floor, log10
import scipy
import multiprocessing
import sh
import json
import statsmodels.stats.api as stats

# This configures numpy/scipy to raise an exception in case of errors, instead of printing a warning and going ahead.
numpy.seterr(all='raise')
scipy.seterr(all='raise')

parser = argparse.ArgumentParser(description='Runs all the benchmarks whose results are on the Fruit website.')
parser.add_argument('--only-benchmarks', help='Runs only the specified benchmarks instead of all of them. Specify a comma-separated list of the benchmark names: '
                         + '{new_delete_run_time,fruit_single_file_compile_time,fruit_compile_time,fruit_run_time,fruit_executable_size,boost_di_compile_time,boost_di_run_time,boost_di_executable_size}. (default: run all benchmarks)')
parser.add_argument('--compilers', help='Compilers to benchmark (comma-separated)')
parser.add_argument('--fruit-benchmark-sources-dir', help='Path to the fruit sources (used for benchmarking code only)')
parser.add_argument('--fruit-sources-dir', help='Path to the fruit sources')
parser.add_argument('--boost-di-sources-dir', help='Path to the Boost.DI sources')
parser.add_argument('--num-classes', help='Number(s) of classes to test with, in runtime tests (comma-separated). (default: 100,1000 for Fruit, 100 for Boost.DI)')
parser.add_argument('--num-bindings', default='20,80,320', help='Number(s) of bindings to test with, in compile-time tests (comma-separated). (default: 20,80,320)')
parser.add_argument('--output-file', help='The output file where benchmark results will be stored (1 per line, with each line in JSON format). These can then be formatted by e.g. the format_bench_results script.')
parser.add_argument('--max-runs', default=50, help='The maximum number of runs for a given benchmark (per combination of parameters). Each benchmark is run at most this number of times, but it could be run fewer times if the results are stable enough that we reach 2 significant digits in the results earlier.')
parser.add_argument('--cxx-std', help='Version of the C++ standard to use. Typically one of \'c++11\' and \'c++14\'. (default: \'c++11\' for Fruit, \'c++14\' for Boost.DI)')
args = parser.parse_args()
if args.only_benchmarks is None:
    only_benchmarks = ['new_delete_run_time', 'fruit_single_file_compile_time', 'fruit_compile_time', 'fruit_run_time', 'fruit_executable_size', 'boost_di_compile_time', 'boost_di_run_time', 'boost_di_executable_size']
else:
    only_benchmarks = args.only_benchmarks.split(',')

if args.output_file is None:
    raise Exception('You must specify --output_file')
sh.rm('-f', args.output_file)

fruit_build_tmpdir = tempfile.gettempdir() + '/fruit-benchmark-build-dir'

compile_flags = ['-O2', '-DNDEBUG']

make_command = sh.make.bake(j=multiprocessing.cpu_count() + 1)

if args.cxx_std is None:
    standard_cxx_std = 'c++11'
    boost_di_cxx_std = 'c++14'
else:
    standard_cxx_std = args.cxx_std
    boost_di_cxx_std = args.cxx_std

if args.num_classes is None:
    fruit_num_classes_list = [100,1000]
    # Boost.DI fails to compile the benchmark code with 1000 classes.
    boost_di_num_classes_list = [100]
else:
    fruit_num_classes_list = args.num_classes.split(',')
    boost_di_num_classes_list = fruit_num_classes_list

# Parses results from the format:
# ['Dimension name1        = 123',
#  'Long dimension name2   = 23.45']
#
# Into a dict {'Dimension name1': 123.0, 'Dimension name2': 23.45}
def parse_results(result_lines):
    result_dict = dict()
    for line in result_lines:
        line_splits = line.split('=')
        metric = line_splits[0].strip()
        value = float(line_splits[1].strip())
        result_dict[metric] = value
    return result_dict

class NewDeleteRunTimeBenchmark:
    def __init__(self, compiler_command, compiler_name, num_classes, cxx_std):
        self.compiler_command = compiler_command
        self.compiler_name = compiler_name
        self.num_classes = num_classes
        self.cxx_std = cxx_std
    def prepare(self):
        self.tmpdir = tempfile.gettempdir() + '/fruit-benchmark-dir'
        ensure_empty_dir(self.tmpdir)
        self.compiler_command(
            '-std=%s' % (self.cxx_std),
            '-DMULTIPLIER=%s' % self.num_classes,
            args.fruit_benchmark_sources_dir + '/extras/benchmark/new_delete_benchmark.cpp',
            o=self.tmpdir + '/main')
        sh.chmod('+x', self.tmpdir + '/main')
    def run(self):
        main = sh.Command(self.tmpdir + '/main')
        results = main(5000000)
        return parse_results(results.splitlines())
    def describe(self):
        return {'name': 'new_delete_run_time', 'num_classes': self.num_classes, 'compiler_name': self.compiler_name, 'cxx_std': self.cxx_std}
    def __str__(self):
        return str(self.describe())

class FruitSingleFileCompileTimeBenchmark:
    def __init__(self, compiler_command, num_bindings, compiler_name, cxx_std):
        assert (num_bindings % 5) == 0, num_bindings
        self.compiler_command = compiler_command
        self.num_bindings = num_bindings
        self.compiler_name = compiler_name
        self.cxx_std = cxx_std
    def prepare(self):
        pass
    def run(self):
        start = timer()
        self.compiler_command(
            '-std=%s' % (self.cxx_std),
            '-DMULTIPLIER=%s' % (self.num_bindings // 5),
            '-I', args.fruit_sources_dir + '/include',
            '-I', fruit_build_tmpdir + '/include',
            '-ftemplate-depth=1000',
            c=args.fruit_benchmark_sources_dir + '/examples/compile_time_benchmark/module.cpp',
            o='/dev/null')
        end = timer()
        return {"compile_time": end - start}
    def describe(self):
        return {'name': 'fruit_single_file_compile_time', 'num_bindings': self.num_bindings, 'compiler_name': self.compiler_name, 'cxx_std': self.cxx_std}
    def __str__(self):
        return str(self.describe())

def ensure_empty_dir(dirname):
    # We start by creating the directory instead of just calling rmtree with ignore_errors=True because that would ignore
    # all errors, so we might otherwise go ahead even if the directory wasn't properly deleted.
    os.makedirs(dirname, exist_ok=True)
    shutil.rmtree(dirname)
    os.makedirs(dirname)

class GenericGeneratedSourcesBenchmark:
    def __init__(self, name, compiler_executable_name, num_classes, compiler_name, cxx_std, **other_args):
        self.name = name
        self.compiler_executable_name = compiler_executable_name
        self.num_classes = num_classes
        self.compiler_name = compiler_name
        self.cxx_std = cxx_std
        self.other_args = other_args
    def prepare_compile_benchmark(self):
        self.tmpdir = tempfile.gettempdir() + '/fruit-benchmark-dir'
        ensure_empty_dir(self.tmpdir)
        num_classes_with_no_deps = int(self.num_classes * 0.1)
        generate_benchmark_command = sh.Command(args.fruit_benchmark_sources_dir + '/extras/benchmark/generate_benchmark.py')
        generate_benchmark_command(
            compiler=self.compiler_executable_name,
            fruit_sources_dir=args.fruit_sources_dir,
            fruit_build_dir=fruit_build_tmpdir,
            num_components_with_no_deps=num_classes_with_no_deps,
            num_components_with_deps=self.num_classes - num_classes_with_no_deps,
            num_deps=10,
            output_dir=self.tmpdir,
            cxx_std=self.cxx_std,
            **self.other_args)
    def prepare_runtime_benchmark(self):
        self.prepare_compile_benchmark()
        make_command(_cwd=self.tmpdir)
    def prepare_executable_size_benchmark(self):
        self.prepare_runtime_benchmark()
        sh.strip(self.tmpdir + '/main')
    def run_compile_benchmark(self):
        make_command('clean', _cwd=self.tmpdir)
        start = timer()
        make_command(_cwd=self.tmpdir)
        end = timer()
        result = {'compile_time': end - start}
        return result
    def run_runtime_benchmark(self):
        main_command = sh.Command(self.tmpdir + '/main')
        results = main_command(400*1000*1000/self.num_classes) # 4M loops with 100 classes, 400K with 1000
        return parse_results(results.splitlines())
    def run_executable_size_benchmark(self):
        num_bytes = sh.wc('-c', self.tmpdir + '/main').splitlines()[0].split(' ')[0]
        return {'num_bytes': float(num_bytes)}
    def describe(self):
        return {'name': self.name, 'num_classes': self.num_classes, 'compiler_name': self.compiler_name, 'cxx_std': self.cxx_std}

class FruitCompileTimeBenchmark:
    def __init__(self, **kwargs):
        self.generic_benchmark = GenericGeneratedSourcesBenchmark(
            name='fruit_compile_time',
            di_library='fruit',
            **kwargs)
    def prepare(self):
        self.generic_benchmark.prepare_compile_benchmark()
    def run(self):
        return self.generic_benchmark.run_compile_benchmark()
    def describe(self):
        return self.generic_benchmark.describe()
    def __str__(self):
        return str(self.describe())

class FruitRunTimeBenchmark:
    def __init__(self, **kwargs):
        self.generic_benchmark = GenericGeneratedSourcesBenchmark(
            name='fruit_run_time',
            di_library='fruit',
            **kwargs)
    def prepare(self):
        self.generic_benchmark.prepare_runtime_benchmark()
    def run(self):
        return self.generic_benchmark.run_runtime_benchmark()
    def describe(self):
        return self.generic_benchmark.describe()
    def __str__(self):
        return str(self.describe())

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class FruitExecutableSizeBenchmark:
    def __init__(self, **kwargs):
        self.generic_benchmark = GenericGeneratedSourcesBenchmark(
            name='fruit_executable_size',
            di_library='fruit',
            **kwargs)
    def prepare(self):
        self.generic_benchmark.prepare_executable_size_benchmark()
    def run(self):
        return self.generic_benchmark.run_executable_size_benchmark()
    def describe(self):
        return self.generic_benchmark.describe()
    def __str__(self):
        return str(self.describe())

class BoostDiCompileTimeBenchmark:
    def __init__(self, **kwargs):
        self.generic_benchmark = GenericGeneratedSourcesBenchmark(
            name='boost_di_compile_time',
            di_library='boost_di',
            boost_di_sources_dir=args.boost_di_sources_dir,
            **kwargs)
    def prepare(self):
        self.generic_benchmark.prepare_compile_benchmark()
    def run(self):
        return self.generic_benchmark.run_compile_benchmark()
    def describe(self):
        return self.generic_benchmark.describe()
    def __str__(self):
        return str(self.describe())

class BoostDiRunTimeBenchmark:
    def __init__(self, **kwargs):
        self.generic_benchmark = GenericGeneratedSourcesBenchmark(
            name='boost_di_run_time',
            di_library='boost_di',
            boost_di_sources_dir=args.boost_di_sources_dir,
            **kwargs)
    def prepare(self):
        self.generic_benchmark.prepare_runtime_benchmark()
    def run(self):
        return self.generic_benchmark.run_runtime_benchmark()
    def describe(self):
        return self.generic_benchmark.describe()
    def __str__(self):
        return str(self.describe())

# This is not really a 'benchmark', but we consider it as such to reuse the benchmark infrastructure.
class BoostDiExecutableSizeBenchmark:
    def __init__(self, **kwargs):
        self.generic_benchmark = GenericGeneratedSourcesBenchmark(
            name='boost_di_executable_size',
            di_library='boost_di',
            boost_di_sources_dir=args.boost_di_sources_dir,
            **kwargs)
    def prepare(self):
        self.generic_benchmark.prepare_executable_size_benchmark()
    def run(self):
        return self.generic_benchmark.run_executable_size_benchmark()
    def describe(self):
        return self.generic_benchmark.describe()
    def __str__(self):
        return str(self.describe())

def round_to_significant_digits(n, num_significant_digits):
    assert n >= 0
    if n == 0:
        # We special-case this, otherwise the log10 below will fail.
        return 0
    return round(n, num_significant_digits-int(floor(log10(n)))-1)

def run_benchmark(benchmark, min_runs=3, max_runs=int(args.max_runs)):
    def run_benchmark_once():
        print('Running benchmark... ', end='', flush=True)
        result = benchmark.run()
        print(result)
        for dimension, value in result.items():
            previous_results = results_by_dimension.setdefault(dimension, [])
            previous_results += [value]

    results_by_dimension = {}
    print('Preparing for benchmark... ', end='', flush=True)
    try:
        benchmark.prepare()
    except Exception as e:
        print("Error while preparing for benchmark: ", e)
        return
    print('Done.')

    # Run at least min_runs times
    for i in range(min_runs):
        try:
            run_benchmark_once()
        except Exception as e:
            print("Error while executing benchmark: ", e.__class__, e.__doc__, e.message)
            return
    # Then consider running a few more times to get the desired precision.
    while True:
        for dimension, results in results_by_dimension.items():
            if all(result == results[0] for result in results):
                # If all results are exactly the same the code below misbehaves. We don't need to run again in this case.
                continue
            confidence_interval = stats.DescrStatsW(results).tconfint_mean(0.05)
            confidence_interval_2dig = (round_to_significant_digits(confidence_interval[0], 2),
                                        round_to_significant_digits(confidence_interval[1], 2))
            if abs(confidence_interval_2dig[0] - confidence_interval_2dig[1]) > numpy.finfo(float).eps * 10:
                if len(results) < max_runs:
                    print("Running again to get more precision on the metric %s. Current confidence interval: [%.3g, %.3g]" % (dimension, confidence_interval[0], confidence_interval[1]))
                    break
                else:
                    print("Warning: couldn't determine a precise result for the metric %s. Confidence interval: [%.3g, %.3g]" % (dimension, confidence_interval[0], confidence_interval[1]))
        else:
            # We've reached sufficient precision in all metrics, or we've reached the max number of runs.
            break

        try:
            run_benchmark_once()
        except Exception as e:
            print("Error while executing benchmark: ", e)
            return
    # We've reached the desired precision in all dimensions or reached the maximum number of runs. Record the results.
    confidence_interval_by_dimension = {}
    for dimension, results in results_by_dimension.items():
        confidence_interval = stats.DescrStatsW(results).tconfint_mean(0.05)
        confidence_interval = (round_to_significant_digits(confidence_interval[0], 2),
                               round_to_significant_digits(confidence_interval[1], 2))
        confidence_interval_by_dimension[dimension] = confidence_interval
    with open(args.output_file, 'a') as f:
        json.dump({"benchmark": benchmark.describe(), "results": confidence_interval_by_dimension}, f)
        print(file=f)
    print('Benchmark finished. Result: ', confidence_interval_by_dimension)
    print()

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
    cmake_output = list(sh.cmake('.', _cwd=tmpdir, _env=modified_env, _iter='err'))
    for line in cmake_output:
        re_result = re.search('@@@(.*)@@@', line)
        if re_result:
            pretty_name = re_result.group(1)
            # CMake calls GCC 'GNU', change it into 'GCC'.
            return pretty_name.replace('GNU ', 'GCC ')
    raise Exception('Unable to determine compiler. CMake output was: \n', cmake_output)

# Calculate the benchmarks to run
compilers = args.compilers.split(',')
benchmarks_by_compiler = {}
for compiler_executable_name in args.compilers.split(','):
    compiler_name = determine_compiler_name(compiler_executable_name)
    compiler_command = sh.Command(compiler_executable_name).bake(compile_flags)

    benchmarks_by_compiler[compiler_executable_name] = []
    if 'new_delete_run_time' in only_benchmarks:
        benchmarks_by_compiler[compiler_executable_name] += [
            NewDeleteRunTimeBenchmark(
                compiler_command=compiler_command,
                compiler_name=compiler_name,
                num_classes=num_classes,
                cxx_std=standard_cxx_std)
            for num_classes in args.num_classes.split(',')]
    if 'fruit_single_file_compile_time' in only_benchmarks:
        benchmarks_by_compiler[compiler_executable_name] += [
            FruitSingleFileCompileTimeBenchmark(
                compiler_command=compiler_command,
                num_bindings=int(num_bindings),
                compiler_name=compiler_name,
                cxx_std=standard_cxx_std)
            for num_bindings in args.num_bindings.split(',')]
    if 'fruit_compile_time' in only_benchmarks:
        benchmarks_by_compiler[compiler_executable_name] += [
            FruitCompileTimeBenchmark(
                compiler_executable_name=compiler_executable_name,
                num_classes=int(num_classes),
                compiler_name=compiler_name,
                cxx_std=standard_cxx_std)
            for num_classes in fruit_num_classes_list]
    if 'fruit_run_time' in only_benchmarks:
        benchmarks_by_compiler[compiler_executable_name] += [
            FruitRunTimeBenchmark(
                compiler_executable_name=compiler_executable_name,
                num_classes=int(num_classes),
                compiler_name=compiler_name,
                cxx_std=standard_cxx_std)
            for num_classes in fruit_num_classes_list]
    if 'fruit_executable_size' in only_benchmarks:
        benchmarks_by_compiler[compiler_executable_name] += [
            FruitExecutableSizeBenchmark(
                compiler_executable_name=compiler_executable_name,
                num_classes=int(num_classes),
                compiler_name=compiler_name,
                cxx_std=standard_cxx_std)
            for num_classes in fruit_num_classes_list]
    if ('boost_di_compile_time' in only_benchmarks
        or 'boost_di_run_time' in only_benchmarks
        or 'boost_di_executable_size' in only_benchmarks):
        if args.boost_di_sources_dir is None:
            if args.only_benchmarks is not None:
                raise Exception('Error: you need to specify the --boost-di-sources-dir flag in order to run Boost.DI benchmarks.')
            else:
                # The Boost.DI benchmarks weren't specified explicitly, skip them and go ahead.
                print('Warning: skipping boost_di_compile_time benchmark since --boost-di-sources-dir was not specified')
        else:
            if 'boost_di_compile_time' in only_benchmarks:
                benchmarks_by_compiler[compiler_executable_name] += [
                    BoostDiCompileTimeBenchmark(
                        compiler_executable_name=compiler_executable_name,
                        num_classes=int(num_classes),
                        compiler_name=compiler_name,
                        cxx_std=boost_di_cxx_std)
                    for num_classes in boost_di_num_classes_list]
            if 'boost_di_run_time' in only_benchmarks:
                benchmarks_by_compiler[compiler_executable_name] += [
                    BoostDiRunTimeBenchmark(
                        compiler_executable_name=compiler_executable_name,
                        num_classes=int(num_classes),
                        compiler_name=compiler_name,
                        cxx_std=boost_di_cxx_std)
                    for num_classes in boost_di_num_classes_list]
            if 'boost_di_executable_size' in only_benchmarks:
                benchmarks_by_compiler[compiler_executable_name] += [
                    BoostDiExecutableSizeBenchmark(
                        compiler_executable_name=compiler_executable_name,
                        num_classes=int(num_classes),
                        compiler_name=compiler_name,
                        cxx_std=boost_di_cxx_std)
                    for num_classes in boost_di_num_classes_list]

# Count them
num_benchmarks = sum([len(benchmarks) for benchmarks in benchmarks_by_compiler.values()])
benchmark_index = 1

# Now actually run the benchmarks
for compiler in compilers:
    # Build Fruit in fruit_build_tmpdir, so that fruit_build_tmpdir points to a built Fruit (useful for e.g. the config header).
    shutil.rmtree(fruit_build_tmpdir, ignore_errors=True)
    os.makedirs(fruit_build_tmpdir)
    modified_env = os.environ.copy()
    modified_env['CXX'] = compiler
    sh.cmake(args.fruit_sources_dir, '-DCMAKE_BUILD_TYPE=Release', _cwd=fruit_build_tmpdir, _env=modified_env)
    make_command(_cwd=fruit_build_tmpdir)

    for benchmark in benchmarks_by_compiler[compiler]:
        print('%s/%s: %s' % (benchmark_index, num_benchmarks, benchmark))
        benchmark_index += 1
        run_benchmark(benchmark)
