# Benchmark Results

Recorded runs of the full benchmark suite, so a later change has something to be read against. How to build and run the
benchmarks, and how to compare two runs, is in `docs/benchmarks.md`.

A number here is only good on the machine that produced it. To judge a change, rerun the suite on that machine and compare
against the stored baseline, or take a fresh baseline of your own; never compare your numbers with these. Rows that hand work
between threads on different CCDs vary by 10 to 30% from one process to the next, and a few, such as `ChannelMPMC` with seven
producers and one consumer, are bimodal. Read those from the stored repetitions, not from the median alone.

Each snapshot keeps its raw JSON in `benchmark/baselines/`, with every repetition, which is what `compare.py` needs. The tables
below are generated from it. The host name is removed from the JSON before it is committed.

## Adding a snapshot

Build as `docs/benchmarks.md` describes (Release, `OPAL_HARDENING=OFF`), close anything that keeps the CPU busy, and run the
whole suite. It takes about half an hour on the machine below.

```bash
opal_benchmark --benchmark_repetitions=3 --benchmark_enable_random_interleaving=true     --benchmark_out=<date>-<commit>-<machine>-<compiler>.json --benchmark_out_format=json
python benchmark/summarize.py <date>-<commit>-<machine>-<compiler>.json > tables.md
```

Strip `host_name` from the JSON's `context`, put the file in `benchmark/baselines/`, and add a section under Snapshots with the
commit, the build, anything unusual about the run, and the tables. A new machine gets an entry under Machines first.

## Comparing against a baseline

```bash
opal_benchmark --benchmark_repetitions=3 --benchmark_enable_random_interleaving=true     --benchmark_out=new.json --benchmark_out_format=json
python build/bench/_deps/benchmark-src/tools/compare.py benchmarks     benchmark/baselines/2026-10-21-abf8636-ryzen9-5950x-msvc.json new.json
```

Only on the machine the baseline came from, and with `--benchmark_filter` to rerun just the part a change touches.

## Machines

### Ryzen 9 5950X desktop

| | |
|---|---|
| CPU | AMD Ryzen 9 5950X, Zen 3. 16 cores, 32 threads, 3.4 GHz base |
| Cache | 32 KB L1D and 512 KB L2 per core. Two CCDs of 8 cores, each with its own 32 MB L3 |
| `l3_domains` | `[0,2,4,6,8,10,12,14] [16,18,20,22,24,26,28,30]`, the first logical processor of each core |
| Memory | 32 GB DDR4-3200, 2 x 16 GB Kingston KF3200C16D4/16GX |
| Board | Gigabyte X570S AORUS ELITE AX |
| OS | Windows 11 Pro 10.0.26200, High performance power plan |
| Virtualization | Hyper-V present and virtualization-based security running, so Windows itself runs as a guest |
| Compiler | MSVC 19.44.35228 (Visual Studio 2022), `/O2 /Ob2 /DNDEBUG`, Visual Studio generator |

## Snapshots

### 2026-10-21, abf8636, Ryzen 9 5950X, MSVC

File: `benchmark/baselines/2026-10-21-abf8636-ryzen9-5950x-msvc.json`. Release, `OPAL_HARDENING=OFF`, `OPAL_EXCEPTIONS=ON`,
Google Benchmark 1.9.5. Three repetitions, interleaved; 585 benchmarks, none skipped. The median of the three is shown.

Times are wall time for one benchmark iteration, whose size each benchmark file states in its header comment: a `FillDrain`
iteration fills and empties the channel once, a `Transfer` iteration hands over 262144 items, a `ThreadPool` iteration runs
16384 tasks (one task for `RoundTrip`, one loop over `size` elements for `ParallelFor`), and a hash container iteration does its
operation once for every key, except `Copy`, which copies the whole container once.

#### ChannelMPMC/FillDrain

| | Signal | Spin | Std |
|---|---:|---:|---:|
| capacity:64 | 331 ns | 322 ns | 1.36 us |
| capacity:1024 | 6.04 us | 6.2 us | 21.6 us |
| capacity:16384 | 96.7 us | 97.3 us | 347 us |

#### ChannelMPMC/Transfer/CrossL3

| | Signal | Spin | Std |
|---|---:|---:|---:|
| capacity:64/producers:1/consumers:1 | 22 ms | 25.2 ms | 26.3 ms |
| capacity:64/producers:1/consumers:4 | 17.3 ms | 14 ms | 63.6 ms |
| capacity:64/producers:2/consumers:2 | 32.9 ms | 24.5 ms | 41.8 ms |
| capacity:64/producers:4/consumers:1 | 21.3 ms | 31.9 ms | 67 ms |
| capacity:64/producers:4/consumers:4 | 20.3 ms | 16.3 ms | 73.1 ms |
| capacity:64/producers:7/consumers:1 | 456 ms | 35.8 ms | 397 ms |
| capacity:1024/producers:1/consumers:1 | 13.5 ms | 24.3 ms | 19.1 ms |
| capacity:1024/producers:1/consumers:4 | 18.2 ms | 17.4 ms | 41.4 ms |
| capacity:1024/producers:2/consumers:2 | 29.3 ms | 25 ms | 27.6 ms |
| capacity:1024/producers:4/consumers:1 | 11.7 ms | 11.4 ms | 47.5 ms |
| capacity:1024/producers:4/consumers:4 | 21.3 ms | 19.9 ms | 44.9 ms |
| capacity:1024/producers:7/consumers:1 | 26.6 ms | 19.9 ms | 275 ms |
| capacity:16384/producers:1/consumers:1 | 16 ms | 20.9 ms | 18.4 ms |
| capacity:16384/producers:1/consumers:4 | 19.6 ms | 16.9 ms | 27.7 ms |
| capacity:16384/producers:2/consumers:2 | 28.2 ms | 23.3 ms | 26 ms |
| capacity:16384/producers:4/consumers:1 | 11.3 ms | 11.6 ms | 45.9 ms |
| capacity:16384/producers:4/consumers:4 | 21.2 ms | 19.1 ms | 39.1 ms |
| capacity:16384/producers:7/consumers:1 | 23.3 ms | 12.2 ms | 289 ms |

#### ChannelMPMC/Transfer/SameL3

| | Signal | Spin | Std |
|---|---:|---:|---:|
| capacity:64/producers:1/consumers:1 | 10.5 ms | 11.7 ms | 12.7 ms |
| capacity:64/producers:1/consumers:4 | 11.4 ms | 9.2 ms | 20.8 ms |
| capacity:64/producers:2/consumers:2 | 15.8 ms | 12.9 ms | 17.4 ms |
| capacity:64/producers:4/consumers:1 | 18.3 ms | 13.2 ms | 21.3 ms |
| capacity:64/producers:4/consumers:4 | 20.1 ms | 10.3 ms | 24.5 ms |
| capacity:64/producers:7/consumers:1 | 28.3 ms | 12.8 ms | 36.5 ms |
| capacity:1024/producers:1/consumers:1 | 5.66 ms | 11.6 ms | 10.2 ms |
| capacity:1024/producers:1/consumers:4 | 11.6 ms | 10.5 ms | 13.6 ms |
| capacity:1024/producers:2/consumers:2 | 15.4 ms | 13.2 ms | 12.6 ms |
| capacity:1024/producers:4/consumers:1 | 10.9 ms | 11.9 ms | 18.3 ms |
| capacity:1024/producers:4/consumers:4 | 15.7 ms | 13.6 ms | 16.3 ms |
| capacity:1024/producers:7/consumers:1 | 11.4 ms | 11.5 ms | 30.4 ms |
| capacity:16384/producers:1/consumers:1 | 5.6 ms | 10.4 ms | 10 ms |
| capacity:16384/producers:1/consumers:4 | 11.6 ms | 10.6 ms | 11.6 ms |
| capacity:16384/producers:2/consumers:2 | 14.9 ms | 13.1 ms | 12.2 ms |
| capacity:16384/producers:4/consumers:1 | 10.8 ms | 11.2 ms | 18 ms |
| capacity:16384/producers:4/consumers:4 | 15.1 ms | 13.5 ms | 15.3 ms |
| capacity:16384/producers:7/consumers:1 | 11.2 ms | 10.9 ms | 29.9 ms |

#### ChannelMPSC/FillDrain

| | Signal | Spin | Std |
|---|---:|---:|---:|
| capacity:64 | 244 ns | 236 ns | 1.36 us |
| capacity:1024 | 4.28 us | 4.15 us | 21.7 us |
| capacity:16384 | 69.9 us | 67.7 us | 348 us |

#### ChannelMPSC/Transfer/CrossL3

| | Signal | Spin | Std |
|---|---:|---:|---:|
| capacity:64/producers:1/consumers:1 | 16.3 ms | 14.5 ms | 26.6 ms |
| capacity:64/producers:2/consumers:1 | 20.9 ms | 17.2 ms | 34.2 ms |
| capacity:64/producers:4/consumers:1 | 13.9 ms | 11.2 ms | 60.2 ms |
| capacity:64/producers:7/consumers:1 | 108 ms | 8.07 ms | 444 ms |
| capacity:1024/producers:1/consumers:1 | 4.23 ms | 10.6 ms | 18.9 ms |
| capacity:1024/producers:2/consumers:1 | 20.9 ms | 18.2 ms | 26.3 ms |
| capacity:1024/producers:4/consumers:1 | 14.6 ms | 12.7 ms | 49.3 ms |
| capacity:1024/producers:7/consumers:1 | 15.1 ms | 8.36 ms | 333 ms |
| capacity:16384/producers:1/consumers:1 | 3.19 ms | 6.88 ms | 18.3 ms |
| capacity:16384/producers:2/consumers:1 | 20 ms | 16.9 ms | 26.4 ms |
| capacity:16384/producers:4/consumers:1 | 14.3 ms | 12.4 ms | 44.8 ms |
| capacity:16384/producers:7/consumers:1 | 10.5 ms | 9.79 ms | 385 ms |

#### ChannelMPSC/Transfer/SameL3

| | Signal | Spin | Std |
|---|---:|---:|---:|
| capacity:64/producers:1/consumers:1 | 8.11 ms | 8.51 ms | 12.9 ms |
| capacity:64/producers:2/consumers:1 | 9.54 ms | 7.68 ms | 15.5 ms |
| capacity:64/producers:4/consumers:1 | 9.51 ms | 5.34 ms | 21.8 ms |
| capacity:64/producers:7/consumers:1 | 10.1 ms | 5.15 ms | 34.3 ms |
| capacity:1024/producers:1/consumers:1 | 7.51 ms | 7.59 ms | 10.3 ms |
| capacity:1024/producers:2/consumers:1 | 9.09 ms | 6.79 ms | 12.8 ms |
| capacity:1024/producers:4/consumers:1 | 9.46 ms | 5.88 ms | 17.9 ms |
| capacity:1024/producers:7/consumers:1 | 9.55 ms | 4.97 ms | 33.8 ms |
| capacity:16384/producers:1/consumers:1 | 7.19 ms | 7.11 ms | 10.3 ms |
| capacity:16384/producers:2/consumers:1 | 8.94 ms | 6.69 ms | 12.6 ms |
| capacity:16384/producers:4/consumers:1 | 9.37 ms | 5.86 ms | 17.8 ms |
| capacity:16384/producers:7/consumers:1 | 9.43 ms | 4.98 ms | 29.1 ms |

#### ChannelSPSC/FillDrain

| | Signal | Spin | Std |
|---|---:|---:|---:|
| capacity:64 | 147 ns | 112 ns | 1.37 us |
| capacity:1024 | 2.24 us | 1.64 us | 21.8 us |
| capacity:16384 | 35 us | 26.3 us | 347 us |

#### ChannelSPSC/Transfer/CrossL3

| | Signal | Spin | Std |
|---|---:|---:|---:|
| capacity:64/producers:1/consumers:1 | 13.1 ms | 6.99 ms | 26.6 ms |
| capacity:1024/producers:1/consumers:1 | 7.95 ms | 6.1 ms | 19.4 ms |
| capacity:16384/producers:1/consumers:1 | 6.87 ms | 4.29 ms | 18.2 ms |

#### ChannelSPSC/Transfer/SameL3

| | Signal | Spin | Std |
|---|---:|---:|---:|
| capacity:64/producers:1/consumers:1 | 7.71 ms | 3.16 ms | 12.9 ms |
| capacity:1024/producers:1/consumers:1 | 3.84 ms | 3.14 ms | 11 ms |
| capacity:16384/producers:1/consumers:1 | 3.52 ms | 3.09 ms | 10.4 ms |

#### HashMap/Copy

| | Opal | Std |
|---|---:|---:|
| 1024 | 415 ns | 38.6 us |
| 8192 | 5.27 us | 337 us |
| 65536 | 211 us | 3.14 ms |
| 524288 | 1.51 ms | 43.3 ms |
| 4194304 | 12.3 ms | 625 ms |
| 16777216 | 49.6 ms | 2.75 s |

#### HashMap/Erase

| | Opal | Std |
|---|---:|---:|
| 1024 | 4.47 us | 15.5 us |
| 8192 | 35.1 us | 185 us |
| 65536 | 427 us | 2.28 ms |
| 524288 | 7.29 ms | 37.2 ms |
| 4194304 | 134 ms | 912 ms |
| 16777216 | 774 ms | 4.24 s |

#### HashMap/FindHit

| | Opal | Std |
|---|---:|---:|
| 1024 | 3.59 us | 2.16 us |
| 8192 | 32.4 us | 33.4 us |
| 65536 | 423 us | 705 us |
| 524288 | 4.59 ms | 8.27 ms |
| 4194304 | 122 ms | 180 ms |
| 16777216 | 758 ms | 775 ms |

#### HashMap/FindMiss

| | Opal | Std |
|---|---:|---:|
| 1024 | 3.35 us | 1.98 us |
| 8192 | 27.9 us | 22.7 us |
| 65536 | 258 us | 635 us |
| 524288 | 2.84 ms | 7.05 ms |
| 4194304 | 38.2 ms | 159 ms |
| 16777216 | 330 ms | 695 ms |

#### HashMap/InsertGrow

| | Opal | Std |
|---|---:|---:|
| 1024 | 19.5 us | 41.4 us |
| 8192 | 161 us | 367 us |
| 65536 | 1.73 ms | 4.04 ms |
| 524288 | 19 ms | 49.8 ms |
| 4194304 | 211 ms | 1.09 s |
| 16777216 | 1.08 s | 5.48 s |

#### HashMap/InsertReserved

| | Opal | Std |
|---|---:|---:|
| 1024 | 7.32 us | 39.4 us |
| 8192 | 73 us | 326 us |
| 65536 | 1.18 ms | 3.44 ms |
| 524288 | 11.4 ms | 35.7 ms |
| 4194304 | 154 ms | 860 ms |
| 16777216 | 994 ms | 4.11 s |

#### HashMap/Iterate

| | Opal | Std |
|---|---:|---:|
| 1024 | 1.97 us | 1.22 us |
| 8192 | 17.5 us | 16.2 us |
| 65536 | 168 us | 350 us |
| 524288 | 1.35 ms | 3.84 ms |
| 4194304 | 10.8 ms | 165 ms |
| 16777216 | 43.5 ms | 746 ms |

#### HashSet/Copy

| | Opal | Std |
|---|---:|---:|
| 1024 | 162 ns | 37 us |
| 8192 | 2.66 us | 331 us |
| 65536 | 34.1 us | 3.11 ms |
| 524288 | 967 us | 44.4 ms |
| 4194304 | 7.4 ms | 647 ms |
| 16777216 | 29.5 ms | 2.76 s |

#### HashSet/Erase

| | Opal | Std |
|---|---:|---:|
| 1024 | 4.08 us | 15.3 us |
| 8192 | 34.5 us | 183 us |
| 65536 | 406 us | 2.24 ms |
| 524288 | 6.03 ms | 37.7 ms |
| 4194304 | 120 ms | 890 ms |
| 16777216 | 773 ms | 4.28 s |

#### HashSet/FindHit

| | Opal | Std |
|---|---:|---:|
| 1024 | 3.03 us | 2.19 us |
| 8192 | 27.5 us | 26.5 us |
| 65536 | 359 us | 682 us |
| 524288 | 3.93 ms | 7.96 ms |
| 4194304 | 98.9 ms | 180 ms |
| 16777216 | 742 ms | 784 ms |

#### HashSet/FindMiss

| | Opal | Std |
|---|---:|---:|
| 1024 | 3.38 us | 1.96 us |
| 8192 | 28.2 us | 23.5 us |
| 65536 | 256 us | 636 us |
| 524288 | 2.81 ms | 7.2 ms |
| 4194304 | 35.3 ms | 160 ms |
| 16777216 | 321 ms | 694 ms |

#### HashSet/InsertGrow

| | Opal | Std |
|---|---:|---:|
| 1024 | 17.3 us | 40.9 us |
| 8192 | 138 us | 371 us |
| 65536 | 1.27 ms | 4.1 ms |
| 524288 | 13.8 ms | 47.5 ms |
| 4194304 | 145 ms | 1.08 s |
| 16777216 | 770 ms | 5.37 s |

#### HashSet/InsertReserved

| | Opal | Std |
|---|---:|---:|
| 1024 | 4.99 us | 38.1 us |
| 8192 | 44.7 us | 324 us |
| 65536 | 427 us | 3.45 ms |
| 524288 | 6.72 ms | 35.3 ms |
| 4194304 | 94.6 ms | 896 ms |
| 16777216 | 687 ms | 4.08 s |

#### HashSet/Iterate

| | Opal | Std |
|---|---:|---:|
| 1024 | 1.09 us | 1.21 us |
| 8192 | 8.87 us | 15.9 us |
| 65536 | 106 us | 350 us |
| 524288 | 875 us | 4.12 ms |
| 4194304 | 7.02 ms | 166 ms |
| 16777216 | 28.2 ms | 745 ms |

#### ThreadPool/ParallelFor/CrossL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1/size:4096 | 4.44 us | 4.34 us | 4.39 us |
| threads:1/size:65536 | 39.8 us | 43.4 us | 41 us |
| threads:1/size:1048576 | 604 us | 658 us | 633 us |
| threads:1/size:4194304 | 2.67 ms | 2.77 ms | 2.75 ms |
| threads:2/size:4096 | 4.4 us | 4.01 us | 3.4 us |
| threads:2/size:65536 | 29.7 us | 30.9 us | 29.9 us |
| threads:2/size:1048576 | 419 us | 423 us | 443 us |
| threads:2/size:4194304 | 1.96 ms | 2.01 ms | 2.12 ms |
| threads:4/size:4096 | 6.09 us | 5.55 us | 6.4 us |
| threads:4/size:65536 | 21.6 us | 25.6 us | 21.6 us |
| threads:4/size:1048576 | 277 us | 312 us | 312 us |
| threads:4/size:4194304 | 1.75 ms | 1.72 ms | 1.75 ms |
| threads:7/size:4096 | 13.2 us | 11.9 us | 7.71 us |
| threads:7/size:65536 | 17.2 us | 22.9 us | 21.7 us |
| threads:7/size:1048576 | 194 us | 278 us | 258 us |
| threads:7/size:4194304 | 1.67 ms | 1.71 ms | 1.72 ms |

#### ThreadPool/ParallelFor/SameL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1/size:4096 | 3.37 us | 3.31 us | 3.15 us |
| threads:1/size:65536 | 33.1 us | 34 us | 33.2 us |
| threads:1/size:1048576 | 520 us | 532 us | 526 us |
| threads:1/size:4194304 | 2.39 ms | 2.39 ms | 2.41 ms |
| threads:2/size:4096 | 2.74 us | 2.46 us | 2.41 us |
| threads:2/size:65536 | 24.4 us | 24.5 us | 24.3 us |
| threads:2/size:1048576 | 362 us | 371 us | 374 us |
| threads:2/size:4194304 | 1.81 ms | 1.86 ms | 1.87 ms |
| threads:4/size:4096 | 3.26 us | 2.73 us | 3.03 us |
| threads:4/size:65536 | 17.3 us | 17.4 us | 17.9 us |
| threads:4/size:1048576 | 239 us | 253 us | 260 us |
| threads:4/size:4194304 | 1.66 ms | 1.68 ms | 1.67 ms |
| threads:7/size:4096 | 4.32 us | 4.17 us | 5.24 us |
| threads:7/size:65536 | 14.5 us | 14 us | 15.4 us |
| threads:7/size:1048576 | 175 us | 157 us | 199 us |
| threads:7/size:4194304 | 1.59 ms | 1.65 ms | 1.59 ms |

#### ThreadPool/Post/CrossL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1 | 4.59 ms | 3.95 ms | 1.86 ms |
| threads:2 | 5.66 ms | 4.33 ms | 2.32 ms |
| threads:4 | 6.33 ms | 4.07 ms | 4.45 ms |
| threads:7 | 5.12 ms | 4.17 ms | 34.6 ms |

#### ThreadPool/Post/SameL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1 | 2.83 ms | 1.53 ms | 908 us |
| threads:2 | 2.19 ms | 1.58 ms | 1.13 ms |
| threads:4 | 3.56 ms | 1.58 ms | 1.5 ms |
| threads:7 | 3.97 ms | 1.5 ms | 2.61 ms |

#### ThreadPool/RoundTrip/CrossL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1 | 1.48 us | 1.47 us | 2.35 us |
| threads:2 | 2.62 us | 2.63 us | 7.22 us |
| threads:4 | 8.43 us | 7.87 us | 12.9 us |
| threads:7 | 7.78 us | 7.79 us | 12.6 us |

#### ThreadPool/RoundTrip/SameL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1 | 1.27 us | 1.24 us | 1.21 us |
| threads:2 | 2.37 us | 2.38 us | 1.92 us |
| threads:4 | 3.6 us | 1.63 us | 6.86 us |
| threads:7 | 5.93 us | 6.04 us | 4.97 us |

#### ThreadPool/Spawn/CrossL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1 | 4.41 ms | 2 ms | 736 us |
| threads:2 | 3.4 ms | 2.29 ms | 1.01 ms |
| threads:4 | 3.6 ms | 1.9 ms | 1.21 ms |
| threads:7 | 4.66 ms | 1.75 ms | 1.89 ms |

#### ThreadPool/Spawn/SameL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1 | 1.88 ms | 1.22 ms | 824 us |
| threads:2 | 2.42 ms | 1.71 ms | 915 us |
| threads:4 | 3 ms | 1.49 ms | 1.2 ms |
| threads:7 | 3.7 ms | 1.48 ms | 1.8 ms |

#### ThreadPool/Submit/CrossL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1 | 7.06 ms | 3.88 ms | 8.55 ms |
| threads:2 | 6.77 ms | 4.23 ms | 18 ms |
| threads:4 | 6.59 ms | 4 ms | 32.2 ms |
| threads:7 | 6.52 ms | 3.89 ms | 45.8 ms |

#### ThreadPool/Submit/SameL3

| | Opal | OpalSmallBlock | Std |
|---|---:|---:|---:|
| threads:1 | 2.67 ms | 1.44 ms | 3.33 ms |
| threads:2 | 2.87 ms | 1.86 ms | 5.78 ms |
| threads:4 | 2.66 ms | 1.84 ms | 8.73 ms |
| threads:7 | 2.65 ms | 1.73 ms | 18.2 ms |
