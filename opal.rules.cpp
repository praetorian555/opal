#include "builder/rules.hpp"

// Warnings and options mirror cmake/compiler-warnings.cmake and cmake/compiler-options.cmake. The
// helper's name is unique across rules files, since rules files may share a translation unit.
static void OpalCompileSettings(Builder::Rules::ModuleRules& m, const Builder::Rules::TargetInfo& t)
{
    if (t.toolchain.name == "msvc")
    {
        m.private_compile_flags = {"/W4",     "/w14242", "/w14254", "/w14263", "/w14265", "/w14287", "/we4289",
                                   "/w14296", "/w14311", "/w14545", "/w14546", "/w14547", "/w14549", "/w14555",
                                   "/w14640", "/w14826", "/w14905", "/w14906", "/w14928", "/wd4201", "/WX"};
        m.private_defines = {"UNICODE", "_UNICODE"};
        return;
    }
    m.private_compile_flags = {"-Wall",
                               "-Wextra",
                               "-Wshadow",
                               "-Wnon-virtual-dtor",
                               "-Wold-style-cast",
                               "-Wcast-align",
                               "-Wunused",
                               "-Woverloaded-virtual",
                               "-Wpedantic",
                               "-Wconversion",
                               "-Wsign-conversion",
                               "-Wnull-dereference",
                               "-Wdouble-promotion",
                               "-Wformat=2",
                               "-Wimplicit-fallthrough"};
    if (t.toolchain.name == "clang")
    {
        m.private_compile_flags.insert(m.private_compile_flags.end(),
                                       {"-Wno-gnu-anonymous-struct", "-Wno-nested-anon-types", "-Wno-dtor-name"});
    }
    else
    {
        m.private_compile_flags.insert(m.private_compile_flags.end(),
                                       {"-Wmisleading-indentation", "-Wduplicated-cond", "-Wduplicated-branches", "-Wlogical-op",
                                        "-Wuseless-cast", "-Wsuggest-override"});
    }
}

BUILDER_MODULE(Opal, m, t)
{
    m.cxx_standard = 20;
    m.public_includes = {"include", "third-party/rapidhash/include"};
    m.sources.AddDir("src");
    m.headers.AddDir("include", Recurse::Yes);
    if (t.platform.Is("Windows"))
    {
        m.libs = {"synchronization.lib", "ws2_32.lib"};
    }
    OpalCompileSettings(m, t);

    m.test_sources.AddDir("test").Exclude("test/process-test-helper.cpp");
    m.test_precompiled_header = "test/test-helpers.h";
    m.test_deps = {"Catch2"};
}

// Opal's copy of the amalgamation includes opal/defines.h. Opal's tests depend on Catch2 and Catch2 on
// Opal, which is no cycle: test dependencies are not edges of the tested module.
BUILDER_MODULE(Catch2, m, t)
{
    m.private_deps = {"Opal"};
    m.public_system_includes = {"third-party/catch2/include", "third-party/catch2/include/catch2"};
    m.public_defines = {"CATCH_AMALGAMATED_CUSTOM_MAIN"};
    m.sources.Add("third-party/catch2/src/catch_amalgamated.cpp");
    m.private_compile_flags = {t.toolchain.name == "msvc" ? "/w" : "-w"};
}

// A small program the process tests start. They look for it next to their own executable, so the
// target is named after the file they look for.
BUILDER_MODULE(OpalTestHelper, m, t)
{
    m.private_deps = {"Opal"};
    m.sources.Add("test/process-test-helper.cpp");
    if (t.platform.Is("Windows"))
    {
        m.libs = {"shell32.lib"};
    }
    OpalCompileSettings(m, t);
}

BUILDER_TARGET(Opal, t, c)
{
    t.kind = TargetKind::StaticLibrary;
    t.modules = {"Opal"};
}

// Catch2 is built around exceptions, and the suite asserts on thrown types, so a build without them
// (builder build --no-exceptions) leaves the tests out, as OPAL_EXCEPTIONS=OFF does in CMake.
BUILDER_TARGET(OpalTests, t, c)
{
    t.kind = TargetKind::Test;
    t.modules = {"Opal"};
    t.enabled = c.exceptions;
}

BUILDER_TARGET(opal_test_process_helper, t, c)
{
    t.modules = {"OpalTestHelper"};
    t.enabled = c.exceptions;
}
