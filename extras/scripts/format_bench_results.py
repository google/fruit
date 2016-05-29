#!/usr/bin/python

import sys
import re
from math import floor, log10

# results[benchmark_name][compiler][bench_size] = n
results = {}
percentage_re = re.compile("[0-9]*/[0-9]* \([0-9]*%\)")

bench_size_patterns = {
  'fruit_setup_time': '%s classes',
  'fruit_request_time': '%s classes',
  'new_delete_time': '%s classes',
  'fruit_compile_time': '%s bindings',
}

ordered_bench_names = [
  'fruit_setup_time',
  'fruit_request_time',
  'new_delete_time',
  'fruit_compile_time',
]

def pretty_print_bench_size(bench_name, n):
  return bench_size_patterns[bench_name] % n

def round_to_significant_digits(n, num_significant_digits):
  return round(n, num_significant_digits-int(floor(log10(n)))-1)

def float_to_string(n):
  s = str(round_to_significant_digits(n, 2))
  if s.endswith(".0"):
    return s[:-2]
  else:
    return s

def pretty_print_runtime_bench_result(n):
  if n > 500:
    return "%s us (%s ms)" % (float_to_string(n), float_to_string(n/1000))
  else:
    return "%s us" % float_to_string(n)

def pretty_print_compile_time_bench_result(n):
  if n >= 60:
    return "%s s (%s mins)" % (float_to_string(n), float_to_string(n/60))
  else:
    return "%s s" % float_to_string(n)
  
bench_result_pretty_printers = {
  'fruit_setup_time': pretty_print_runtime_bench_result,
  'fruit_request_time': pretty_print_runtime_bench_result,
  'new_delete_time': pretty_print_runtime_bench_result,
  'fruit_compile_time': pretty_print_compile_time_bench_result,
}

def pretty_print_bench_result(bench_name, n):
  return bench_result_pretty_printers[bench_name](n)

for line in sys.stdin.readlines():
  line = line.strip()
  if not percentage_re.match(line):
    line_splits = line.split()
    compiler = line_splits[0]
    bench_size = int(line_splits[1])
    benchmark_name = line_splits[2]
    bench_result = float(line_splits[3])
    benchmark_results = results.setdefault(benchmark_name, {})
    compiler_results = benchmark_results.setdefault(compiler, {})
    compiler_results[bench_size] = bench_result

for bench_name in ordered_bench_names:
  benchmark_results = results[bench_name]
  table_line_format = "| %20s |"
  table_second_line = "|-" + ("-" * 20) + "-|"
  bench_sizes = sorted(benchmark_results.values()[0].keys())
  for bench_size in bench_sizes:
    table_line_format += " %20s |"
    table_second_line += "-" + ("-" * 20) + "-|"
  table_first_line = table_line_format % ((bench_name,) + tuple([pretty_print_bench_size(bench_name, bench_size) for bench_size in bench_sizes]))
  print ""
  print table_first_line
  print table_second_line
  for (compiler_name, results_for_compiler) in sorted(benchmark_results.items()):
    bench_results = [bench_result for (bench_size, bench_result) in sorted(results_for_compiler.items())]
    print table_line_format % ((compiler_name,) + tuple([pretty_print_bench_result(bench_name, bench_result) for bench_result in bench_results]))


    


