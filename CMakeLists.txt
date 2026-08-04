cmake_minimum_required(VERSION 3.28)
project(OPAL VERSION 0.3.8 LANGUAGES CXX)

option(OPAL_BUILD_TESTS "Build tests" ON)
message(STATUS "OPAL_BUILD_TESTS: ${OPAL_BUILD_TESTS}")
option(OPAL_HARDENING "Enable hardening options" ON)
message(STATUS "OPAL_HARDENING: ${OPAL_HARDENING}")
option(OPAL_SHARED_LIBS "Build shared libraries" OFF)
message(STATUS "OPAL_SHARED_LIBS: ${OPAL_SHARED_LIBS}")
option(OPAL_EXCEPTIONS "Allow the library to throw. When off, a failure that cannot be returned ends the program instead" ON)
message(STATUS "OPAL_EXCEPTIONS: ${OPAL_EXCEPTIONS}")

# Catch2 is built around exceptions, and the suite asserts on thrown types in the places the library still throws. There is
# nothing to gain from a half-built test binary, so say what happened rather than fail the configure.
if (NOT OPAL_EXCEPTIONS AND OPAL_BUILD_TESTS)
    message(WARNING "OPAL_EXCEPTIONS=OFF: the tests need exceptions, so OPAL_BUILD_TESTS is being forced off for this build.")
    set(OPAL_BUILD_TESTS OFF)
endif ()

# Report the toolchain being used to ease debugging of platform-specific issues.
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if (WIN32)
    # The Visual Studio generator records the targeted SDK in this variable. With other
    # generators (e.g. Ninja) the SDK is selected by the environment, so fall back to the
    # WindowsSDKVersion variable set by vcvarsall.bat.
    if (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        set(OPAL_WINDOWS_SDK_VERSION "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
    elseif (DEFINED ENV{WindowsSDKVersion})
        # Strip the trailing backslash the environment variable carries (e.g. "10.0.22621.0\").
        string(REGEX REPLACE "\\\\$" "" OPAL_WINDOWS_SDK_VERSION "$ENV{WindowsSDKVersion}")
    else ()
        set(OPAL_WINDOWS_SDK_VERSION "unknown")
    endif ()
    message(STATUS "Windows SDK: ${OPAL_WINDOWS_SDK_VERSION}")
endif ()

# Don't use C++ 20 modules
set(CMAKE_CXX_SCAN_FOR_MODULES 0)

## C++ language configuration boilerplate, hide symbols by default
if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif ()

## Let OPAL_SHARED_LIBS override BUILD_SHARED_LIBS
if (DEFINED OPAL_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${OPAL_SHARED_LIBS}")
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

# Setup opal library target
set(OPAL_FILES
        src/assert.cpp
        src/allocator.cpp
        src/time.cpp
        src/string-encoding.cpp
        src/paths.cpp
        src/bit.cpp
        src/hash.cpp
        src/rng.cpp
        src/file-system.cpp
        src/program-arguments.cpp
        src/string.cpp
        src/thread.cpp
        src/mutex.cpp
        src/condition-variable.cpp
        src/signal.cpp
        src/logging.cpp
        src/thread-pool.cpp
        src/json-reader.cpp
        src/json-writer.cpp
        src/optional.cpp
        include/opal/defines.h
        include/opal/assert.h
        include/opal/type-traits.h
        include/opal/casts.h
        include/opal/allocator.h
        include/opal/types.h
        include/opal/exceptions.h
        include/opal/error-codes.h
        include/opal/time.h
        include/opal/paths.h
        include/opal/file-system.h
        include/opal/common.h
        include/opal/source-location.h
        include/opal/bit.h
        include/opal/hash.h
        include/opal/rng.h
        include/opal/program-arguments.h
        include/opal/math-base.h
        include/opal/math-constants.h
        include/opal/container/iterator.h
        include/opal/container/expected.h
        include/opal/container/optional.h
        include/opal/container/deque.h
        include/opal/container/array-view.h
        include/opal/container/string.h
        include/opal/container/string-encoding.h
        include/opal/container/string-hash.h
        include/opal/container/dynamic-array.h
        include/opal/container/in-place-array.h
        include/opal/container/priority-queue.h
        include/opal/container/hash-table-base.h
        include/opal/container/hash-set.h
        include/opal/container/hash-map.h
        include/opal/container/scope-ptr.h
        include/opal/container/ref.h
        include/opal/delegate.h
        include/opal/sort/key-indexed-counting.h
        third-party/wyhash/include/wyhash/wyhash.h
        include/opal/math/vector2.h
        include/opal/math/vector3.h
        include/opal/math/vector4.h
        include/opal/math/point2.h
        include/opal/math/point3.h
        include/opal/math/point4.h
        include/opal/math/normal3.h
        include/opal/math/bounds2.h
        include/opal/math/bounds3.h
        include/opal/math/matrix.h
        include/opal/math/rotator.h
        include/opal/math/quaternion.h
        include/opal/math/dual-quaternion.h
        include/opal/math/transform.h
        include/opal/logging.h
        include/opal/threading/thread.h
        include/opal/threading/mutex.h
        include/opal/threading/condition-variable.h
        include/opal/threading/signal.h
        include/opal/threading/channel-spsc.h
        include/opal/threading/channel-mpmc.h
        include/opal/threading/thread-pool.h
        include/opal/clonable-base.h
        ${CMAKE_CURRENT_BINARY_DIR}/include/opal/export.h
        include/opal/variant.h
        include/opal/container/string-format.h
)
add_library(opal ${OPAL_FILES})
add_library(opal::opal ALIAS opal)
target_include_directories(opal PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(opal PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third-party/wyhash/include>)
target_include_directories(opal PUBLIC $<INSTALL_INTERFACE:include>)
target_link_libraries(opal PRIVATE $<BUILD_INTERFACE:opal_warnings>)
target_link_libraries(opal PRIVATE $<BUILD_INTERFACE:opal_options>)
if (WIN32)
    target_link_libraries(opal PRIVATE synchronization)
endif ()
target_compile_features(opal PUBLIC cxx_std_20)
set_target_properties(opal PROPERTIES VERSION ${OPAL_VERSION} SOVERSION ${OPAL_VERSION_MAJOR})

# Whether the library may throw is part of what its headers mean, not just how the .cpp files were built: OPAL_RAISE decides
# between a throw and the contract violation handler, and the containers are templates a consumer instantiates in its own
# translation units. Both the flag and the define are PUBLIC so a consumer cannot end up disagreeing with what it links against.
if (NOT OPAL_EXCEPTIONS)
    target_compile_definitions(opal PUBLIC OPAL_NO_EXCEPTIONS)
    if (MSVC)
        # CMake puts /EHsc in the default flags, and leaving it next to /EHs-c- is a command line conflict rather than an
        # override. Note that Microsoft does not support _HAS_EXCEPTIONS=0 for its standard library, so this configuration is
        # best effort on MSVC; the GCC and Clang ones are the supported paths.
        string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        target_compile_options(opal PUBLIC /EHs-c-)
        target_compile_definitions(opal PUBLIC _HAS_EXCEPTIONS=0)
    else ()
        target_compile_options(opal PUBLIC -fno-exceptions)
    endif ()
endif ()

## Generate the export header for opal and attach it to the target
include(GenerateExportHeader)
generate_export_header(opal EXPORT_FILE_NAME include/opal/export.h)
target_compile_definitions(opal PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:OPAL_STATIC_DEFINE>")
target_include_directories(opal PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")
target_include_directories(opal PUBLIC "$<INSTALL_INTERFACE:include>")

# Install the library
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(OPAL_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/opal)

# A hardened build bakes sanitizer instrumentation into opal, producing an install that
# consumers cannot link without enabling the same sanitizers. Refuse to install such a build.
if (OPAL_HARDENING)
    install(CODE "message(FATAL_ERROR \"Refusing to install: OPAL_HARDENING=ON bakes sanitizers into opal, so the install is not consumable. Reconfigure with -DOPAL_HARDENING=OFF to install.\")")
endif ()

install(TARGETS opal
        EXPORT opalTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # For Windows DLL files
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*.hpp")

# Generate and install the CMake package config so consumers can use find_package(opal)
install(EXPORT opalTargets
        FILE opalTargets.cmake
        NAMESPACE opal::
        DESTINATION ${OPAL_INSTALL_CMAKEDIR})

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/opalConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/opalConfig.cmake
        INSTALL_DESTINATION ${OPAL_INSTALL_CMAKEDIR})

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/opalConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/opalConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/opalConfigVersion.cmake
        DESTINATION ${OPAL_INSTALL_CMAKEDIR})

if (OPAL_BUILD_TESTS)
    set(OPAL_TEST_FILES
            test/main-test.cpp
            test/assert-test.cpp
            test/allocator-test.cpp
            test/dynamic-array-test.cpp
            test/deque-test.cpp
            test/array-view-test.cpp
            test/string-test.cpp
            test/string-view-test.cpp
            test/expected-test.cpp
            test/optional-test.cpp
            test/time-test.cpp
            test/sort-test.cpp
            test/paths-test.cpp
            test/scope-ptr-test.cpp
            test/in-place-array-test.cpp
            test/common-test.cpp
            test/hash-set-test.cpp
            test/hash-map-test.cpp
            test/bit-test.cpp
            test/rng-test.cpp
            test/math-base-test.cpp
            test/vector2-test.cpp
            test/vector3-test.cpp
            test/vector4-test.cpp
            test/point2-test.cpp
            test/point3-test.cpp
            test/point4-test.cpp
            test/normal3-test.cpp
            test/bounds2-test.cpp
            test/bounds3-test.cpp
            test/matrix-test.cpp
            test/rotator-test.cpp
            test/quaternion-test.cpp
            test/dual-quaternion-test.cpp
            test/transform-test.cpp
            test/file-system-test.cpp
            test/program-arguments-test.cpp
            test/shared-ptr-test.cpp
            test/thread-test.cpp
            test/delegate-test.cpp
            test/logging-test.cpp
            test/enum-flags-test.cpp
            test/type-traits-test.cpp
            test/variant-test.cpp
            test/json-reader-test.cpp
            test/json-value-test.cpp
            test/json-writer-test.cpp
            test/string-format-test.cpp
            third-party/catch2/src/catch_amalgamated.cpp)
    add_executable(opal_test ${OPAL_TEST_FILES})
    target_include_directories(opal_test PRIVATE third-party/catch2/include)
    target_include_directories(opal_test PRIVATE third-party/catch2/include/catch2)
    target_link_libraries(opal_test opal)
    target_link_libraries(opal_test opal_warnings)
    target_link_libraries(opal_test opal_options)
    target_compile_definitions(opal_test PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN)
    target_compile_definitions(opal_test PRIVATE cxx_std_20)
    enable_testing()
    add_test(NAME opal_test COMMAND opal_test)
endif ()
