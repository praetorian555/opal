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

## Profiling with AMD uProf

A benchmark says how long an operation takes. A profile says what the core was doing meanwhile, which decides whether fewer
instructions or fewer cache lines would help. AMD uProf reads the hardware counters of a Zen CPU and attributes them to functions
and source lines, given a binary with debug information.

### A binary uProf can read

Attribution needs a PDB, and the numbers only mean something for optimized code, so build Release with `/Zi` added rather than
RelWithDebInfo, which also lowers the inlining level. Configure from PowerShell or cmd, since Git Bash rewrites `/O2` into a path.

```powershell
cmake -S . -B build/bench-prof -DOPAL_BUILD_BENCHMARKS=ON -DOPAL_BUILD_TESTS=OFF -DOPAL_HARDENING=OFF `
    "-DCMAKE_CXX_FLAGS_RELEASE=/O2 /Ob2 /DNDEBUG /Zi" "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=/DEBUG /OPT:REF /OPT:ICF"
cmake --build build/bench-prof --config Release --target opal_benchmark
```

### Collecting and reporting

```powershell
$cli = "C:/Program Files/AMD/AMDuProf/bin/AMDuProfCLI.exe"
& $cli collect --config assess -o out build/bench-prof/Release/opal_benchmark.exe `
    "--benchmark_filter=HashSet/.*/Opal/1024" --benchmark_min_time=1s
& $cli report -i out/AMDuProf-opal_benchmark-EBP_<timestamp> --detail --cutoff 15
```

`assess` samples cycles, instructions, branches, L1 data cache misses and misaligned loads together. `report` writes `report.csv`
into the session directory: a function summary, then a table per function with one row per source line. The `session.uprof` next
to it opens in the uProf GUI for the assembly view. Profile one size per session. A function that runs at 1K and 4M keys in the
same session gets one blended row, and the two regimes look nothing alike.

### Reading the counters

- `CYCLES_NOT_IN_HALT` is the core clock cycles spent in the code, which is the time column. `RETIRED_INST` is instructions that
  completed; speculated and discarded work does not count.
- **IPC** is instructions per cycle. Zen 3 retires up to 6 per cycle, and 3 to 4 is what tight, well-fed code reaches. Above 3 the
  core is busy and only fewer instructions make it faster. Below 1 it is waiting, almost always on memory. **CPI** is the inverse.
- **PTI** is per thousand retired instructions. Raw counts scale with how long the run was, so uProf divides them by instructions.
  It makes functions of different length comparable, and it converts to cost: at ten instructions per lookup, 66 misses PTI is
  about two thirds of a miss per lookup.
- `L1_DC_ACCESSES` and `L1_DC_MISSES` are loads and stores hitting or missing the 32 KB level 1 data cache. A miss goes on to L2,
  L3, then DRAM, at roughly 4, 12, 45 and 250 or more cycles. The counter does not say where a miss ended, so pair it with IPC:
  many misses at IPC near 0.4 is DRAM.
- `MISALIGNED_LOADS` counts loads that cross a 64-byte cache line or a 4 KB page, which the hardware splits in two. A 16-byte
  group read that starts at an arbitrary hash position crosses a line about one time in four.
- `RETIRED_BR_INST` and `RETIRED_BR_INST_MISP` are branches executed and branches guessed wrong. A mispredict discards 15 to 20
  cycles of work. As PTI, 0.3 is noise and 5 to 10 is a real cost.

Reading a row: FindHit at 4M keys shows IPC 0.41, 66 misses PTI, 0.5 mispredicts PTI. No branch trouble, a miss every fifteen
instructions, the core idle three quarters of the time. That function is bound by DRAM latency, and only touching fewer lines per
lookup helps. The same row at 1024 keys shows IPC 3.7 and 2 misses PTI. Nothing to wait for, so only instruction count matters.

One trap: a small benchmark repeats the same input thousands of times, and the branch predictor memorizes it. Iteration over the
same 1024 keys showed 0.25 mispredicts PTI, the same loop over 4M keys 8.8. Branchy code measured only at small sizes looks
better than it is.

## Writing a benchmark

- Add the file to `OPAL_BENCHMARK_FILES` in `CMakeLists.txt`. Key sequences and the size sweep live in
  `benchmark/benchmark-helpers.h`, so a new file sees the same input and sizes as the others.
- Do setup before the `for (auto _ : state)` loop. Only the loop body is timed.
- Feed every result into `benchmark::DoNotOptimize`, or the optimizer deletes the work.
- When comparing against a standard container, write the body once as a template over the container type so both sides do
  identical work.
- One operation per benchmark, swept across sizes that cross the cache levels. `state.SetItemsProcessed` adds a throughput
  column that makes the sweep readable.
