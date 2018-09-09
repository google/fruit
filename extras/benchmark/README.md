
## Fruit benchmarks

Fruit contains some code to benchmark various metrics (e.g. performance, compile time, executable size) in an automated
way.

### Benchmark suites

The `suites` folder contains suites of Fruit (or Fruit-related) benchmarks that can be run using `run_benchmarks.py`.
For example:

```bash
$ ~/projects/fruit/extras/benchmark/run_benchmarks.py \
    --continue-benchmark=true \
    --benchmark-definition ~/projects/fruit/extras/benchmark/suites/fruit_full.yml
    --output-file ~/fruit_bench_results.txt \
    --fruit-sources-dir ~/projects/fruit \
    --fruit-benchmark-sources-dir ~/projects/fruit \
    --boost-di-sources-dir ~/projects/boost-di
```

Once the benchmark run completes, you can format the results using some pre-defined tables, see the section below.

The following benchmark suites are defined:

* `fruit_full.yml`: full set of Fruit benchmarks (using the Fruit 3.x API).
* `fruit_mostly_full.yml`: a subset of the tests in `fruit_full.yml`.
* `fruit_quick.yml`: this is an even smaller subset, and the number of runs is capped at 10 so
  the confidence intervals might be wider. It's useful as a quicker (around 10-15min) way to get a rough idea of the
  performance (e.g. to evaluate the performance impact of a commit, during development).
* `fruit_single.yml`: runs the Fruit runtime benchs under a single compiler and with just 1 combination of flags. This
  also caps the number of runs at 8, so the resulting confidence intervals might be wider than they would be with
  `fruit_full.yml`. This is a quick benchmark that can used during development of performance optimizations.
* `fruit_debug.yml`: a suite used to debug Fruit's benchmarking code. This is very quick, but the actual results are
  not meaningful. Run this after changing any benchmarking code, to check that it still works.
* `boost_di`: unlike the others, this benchmark suite exercises the Boost.DI library (still in boost-experimental at the
  time of writing) instead of Fruit.

### Benchmark tables

The `tables` folder contains some table definitions that can be used to get a human-readable representations of
benchmark results generated using `run_benchmarks.py`.

Note that there *isn't* a 1:1 mapping between benchmark suites and benchmark tables; the same table definition can be
used with multiple benchmark suites (for example, a full suite and a quick variant that only has a subset of the
dimensions) and multiple table definitions might be appropriate to display the results of a single suite (for example,
there could be a table definition that displays only metrics meaningful to Fruit users and one that also displays 
more fine-grained metrics that are only meaningful to Fruit developers).

Example usage of `format_bench_results.py` with one of these table definitions:

```bash
$ ~/projects/fruit/extras/benchmark/format_bench_results.py \
    --benchmark-results ~/fruit_bench_results.txt \
    --benchmark-tables-definition ~/projects/fruit/extras/benchmark/tables/fruit_wiki.yml
```

It's also possible to compare two benchmark results (for example, when running the same benchmarks before and after
a Fruit commit):

```bash
$ ~/projects/fruit/extras/benchmark/format_bench_results.py \
    --benchmark-results ~/fruit_bench_results_after.txt \
    --benchmark-tables-definition ~/projects/fruit/extras/benchmark/tables/fruit_wiki.yml \
    --baseline-benchmark-results ~/fruit_bench_results_before.txt
```

The following tables are defined:

* `fruit_wiki.yml`: the "main" table definition, with the tables that are in Fruit's wiki. 
* `fruit_internal.yml`: a more detailed version of `fruit_wiki.yml`, also displaying metrics that are only meaningful
  to Fruit developers (e.g. splitting the setup time into component creation time and normalization time).

### Manual benchmarks

In some cases, you might want to run the benchmarks manually (e.g. if you want to use `callgrind` to profile the
benchmark run). This is how you can do that:
 
```bash
$ cd ~/projects/fruit
$ mkdir build
$ cd build
$ CXX=g++-6 cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
$ make -j 10
$ cd ..
$ mkdir generated-benchs
$ extras/benchmark/generate_benchmark.py \
    --compiler g++-6 \
    --fruit-sources-dir ~/projects/fruit \
    --fruit-build-dir ~/projects/fruit/build \
    --num-components-with-no-deps 10 \
    --num-components-with-deps 90 \
    --num-deps 10 \
    --output-dir generated-benchs \
    --generate-debuginfo=true
$ cd generated-benchs
$ make -j 10
$ valgrind \
    --tool=callgrind \
    --simulate-cache=yes \
    --dump-instr=yes \
    ./main 10000
```
