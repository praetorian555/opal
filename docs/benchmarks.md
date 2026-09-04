# Benchmarks

Opal's benchmarks use Google Benchmark. It is fetched at configure time and only when `OPAL_BUILD_BENCHMARKS` is on, so the
default build never needs it or the network. Sources live in `benchmark/`, one `*-benchmark.cpp` per feature, and build into
the `opal_benchmark` executable.

## Building

```bash
cmake -B build/bench -DCMAKE_BUILD_TYPE=Release -DOPAL_BUILD_BENCHMARKS=ON -DOPAL_BUILD_TESTS=OFF -DOPAL_HARDENING=OFF
cmake --build build/bench --config Release
```

Release and `OPAL_HARDENING=OFF` are not optional. A Debug binary measures the debug checks, and a hardened one measures the
sanitizers; the configure warns about the second. With the Visual Studio generator the build type is chosen at build time,
which is what `--config Release` is for.

## Running

```bash
build/bench/opal_benchmark                   # single-config generators
build/bench/Release/opal_benchmark.exe       # Visual Studio generator
```

Useful flags:

| Flag | Purpose |
|------|---------|
| `--benchmark_filter=<regex>` | run only the benchmarks whose name matches |
| `--benchmark_repetitions=10` | independent runs per benchmark, so the spread is visible |
| `--benchmark_enable_random_interleaving=true` | shuffle the run order so clock drift lands on both sides of a comparison |
| `--benchmark_min_warmup_time=0.5` | untimed warm-up before each run |
| `--benchmark_out=file.json --benchmark_out_format=json` | machine-readable output for comparing |

A number is only meaningful next to a baseline taken on the same machine in the same session. Rerun the baseline rather
than comparing against one saved from another day.

## Comparing two runs

Google Benchmark ships a comparison script in the fetched source, at `build/bench/_deps/benchmark-src/tools/compare.py`. It
reports the ratio of medians and a Mann-Whitney U-test p-value, which is what to read instead of eyeballing two columns.

```bash
pip install -r build/bench/_deps/benchmark-src/tools/requirements.txt

opal_benchmark --benchmark_repetitions=10 --benchmark_enable_random_interleaving=true \
    --benchmark_out=base.json --benchmark_out_format=json
# change the code, rebuild
opal_benchmark --benchmark_repetitions=10 --benchmark_enable_random_interleaving=true \
    --benchmark_out=new.json --benchmark_out_format=json

python build/bench/_deps/benchmark-src/tools/compare.py benchmarks base.json new.json
```

The script needs the raw repetition rows, so do not pass `--benchmark_report_aggregates_only` when producing the JSON. Two
benchmarks inside one file can be compared the same way, which is how the `Opal` and `Std` sides of a benchmark are paired:

```bash
python compare.py filters results.json "/Opal/" "/Std/"
```

## Writing a benchmark

- Add the file to `OPAL_BENCHMARK_FILES` in `CMakeLists.txt`.
- Do setup before the `for (auto _ : state)` loop. Only the loop body is timed.
- Feed every result into `benchmark::DoNotOptimize`, or the optimizer deletes the work.
- When comparing against a standard container, write the body once as a template over the container type so both sides do
  identical work.
- One operation per benchmark, swept across sizes that cross the cache levels. `state.SetItemsProcessed` adds a throughput
  column that makes the sweep readable.
