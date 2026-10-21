"""Turn a Google Benchmark JSON file into Markdown tables of medians, one table per operation with the sides side by side.

Usage: python benchmark/summarize.py <results.json>

The file must hold repetitions (--benchmark_repetitions) or plain runs; the median is taken over the iteration rows of each
benchmark. Names are expected to look like "<Family>/<Operation>[/<Placement>]/<Side>/<args>", which every benchmark in this
directory follows. Standard library only.
"""

import json
import re
import statistics
import sys

SIDES = ["Opal", "OpalSmallBlock", "Signal", "Spin", "Std"]
TO_NANOSECONDS = {"ns": 1.0, "us": 1e3, "ms": 1e6, "s": 1e9}


def natural_key(text):
    """Sort key that orders the numbers inside a name by value, so capacity:64 comes before capacity:1024."""
    return [int(part) if part.isdigit() else part for part in re.split(r"(\d+)", text)]


def format_time(nanoseconds):
    for unit, scale in (("s", 1e9), ("ms", 1e6), ("us", 1e3)):
        if nanoseconds >= scale:
            return f"{nanoseconds / scale:.3g} {unit}"
    return f"{nanoseconds:.3g} ns"


def split_name(run_name):
    """Return (table, row, side) for a run name, or None when no part of it names a side."""
    parts = [part for part in run_name.split("/") if part not in ("real_time", "manual_time")]
    for index, part in enumerate(parts):
        if part in SIDES and index >= 2:
            table = "/".join(parts[:index])
            row = "/".join(parts[index + 1:])
            return table, row, part
    return None


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    with open(sys.argv[1], encoding="utf-8") as file:
        results = json.load(file)

    samples = {}
    for benchmark in results["benchmarks"]:
        if benchmark.get("run_type") != "iteration" or benchmark.get("error_occurred") or benchmark.get("skipped"):
            continue
        split = split_name(benchmark["run_name"])
        if split is None:
            continue
        table, row, side = split
        time = benchmark["real_time"] * TO_NANOSECONDS[benchmark.get("time_unit", "ns")]
        samples.setdefault(table, {}).setdefault(row, {}).setdefault(side, []).append(time)

    for table in sorted(samples, key=natural_key):
        rows = samples[table]
        sides = [side for side in SIDES if any(side in by_side for by_side in rows.values())]
        print(f"#### {table}\n")
        print("| | " + " | ".join(sides) + " |")
        print("|---|" + "---:|" * len(sides))
        for row in sorted(rows, key=natural_key):
            by_side = rows[row]
            cells = [format_time(statistics.median(by_side[side])) if side in by_side else "" for side in sides]
            print(f"| {row or '-'} | " + " | ".join(cells) + " |")
        print()


if __name__ == "__main__":
    main()
