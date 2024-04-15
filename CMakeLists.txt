cmake_minimum_required(VERSION 3.28)

project(
        "Opal"
        VERSION 0.1
        LANGUAGES CXX
        DESCRIPTION "Data structures and algorithms"
)

option(OPAL_BUILD_TESTS "Build tests" ON)
option(OPAL_HARDENING "Enable hardening options" ON)
message(STATUS "OPAL_BUILD_TESTS: ${OPAL_BUILD_TESTS}")

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif ()

# Configure desired compilation options and warnings
include(cmake/compiler-warnings.cmake)
include(cmake/compiler-options.cmake)
add_library(opal_warnings INTERFACE)
add_library(opal_options INTERFACE)
setup_compiler_warnings(opal_warnings)
setup_compiler_options(opal_options)

if (OPAL_HARDENING)
    include(cmake/sanitizers.cmake)
    setup_sanitizers(opal_options)
endif ()

add_subdirectory(third-party/catch2)
add_subdirectory(src)

if (OPAL_BUILD_TESTS)
    add_subdirectory(test)
    enable_testing()
    add_test(NAME opal_test COMMAND opal_test)
endif ()
