#include "builder/rules.hpp"

// Google Benchmark, from the archive builder.json fetches only with -Dbenchmarks=true. Paths are
// relative to the archive's root, not to this file, which builder.json excludes from opal's own rules.
// The library only: opal's benchmarks bring their own main. The defines are what Google Benchmark's
// CMake feature checks find on every compiler opal builds with.
BUILDER_MODULE(Benchmark, m, t)
{
    m.public_system_includes = {"include"};
    m.public_defines = {"BENCHMARK_STATIC_DEFINE"};
    m.private_defines = {"HAVE_STD_REGEX", "HAVE_STEADY_CLOCK", "BENCHMARK_VERSION=\"v1.9.5\""};
    m.sources.AddDir("src").Exclude("src/benchmark_main.cc");
    m.warnings = Warnings::Off;
    if (t.platform.Is(Platform::Windows))
    {
        m.libs = {"shlwapi.lib"};
    }
}
