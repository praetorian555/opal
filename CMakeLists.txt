cmake_minimum_required(VERSION 3.28)
project(OPAL VERSION 1.0.0 LANGUAGES CXX)

option(OPAL_BUILD_TESTS "Build tests" ON)
message(STATUS "OPAL_BUILD_TESTS: ${OPAL_BUILD_TESTS}")
option(OPAL_HARDENING "Enable hardening options" ON)
message(STATUS "OPAL_HARDENING: ${OPAL_HARDENING}")
option(OPAL_SHARED_LIBS "Build shared libraries" OFF)
message(STATUS "OPAL_SHARED_LIBS: ${OPAL_SHARED_LIBS}")

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
        src/allocator.cpp
        src/time.cpp
        src/string-encoding.cpp
        src/paths.cpp
        include/opal/defines.h
        include/opal/assert.h
        include/opal/type-traits.h
        include/opal/allocator.h
        include/opal/types.h
        include/opal/error-codes.h
        include/opal/time.h
        include/opal/paths.h
        include/opal/common.h
        include/opal/source-location.h
        include/opal/container/iterator.h
        include/opal/container/expected.h
        include/opal/container/deque.h
        include/opal/container/array-view.h
        include/opal/container/string.h
        include/opal/container/string-encoding.h
        include/opal/container/string-hash.h
        include/opal/container/dynamic-array.h
        include/opal/container/in-place-array.h
        include/opal/container/priority-queue.h
        include/opal/container/hash-set.h
        include/opal/container/hash-map.h
        include/opal/container/scope-ptr.h
        include/opal/container/ref.h
        include/opal/sort/key-indexed-counting.h
        ${CMAKE_CURRENT_BINARY_DIR}/include/opal/export.h
)
add_library(opal ${OPAL_FILES})
target_include_directories(opal PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(opal PUBLIC $<INSTALL_INTERFACE:include>)
target_link_libraries(opal PRIVATE opal_warnings)
target_link_libraries(opal PRIVATE opal_options)
target_compile_features(opal PUBLIC cxx_std_20)
set_target_properties(opal PROPERTIES VERSION ${OPAL_VERSION} SOVERSION ${OPAL_VERSION_MAJOR})

## Generate the export header for opal and attach it to the target
include(GenerateExportHeader)
generate_export_header(opal EXPORT_FILE_NAME include/opal/export.h)
target_compile_definitions(opal PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:OPAL_STATIC_DEFINE>")
target_include_directories(opal PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")
target_include_directories(opal PUBLIC "$<INSTALL_INTERFACE:include>")

# Install the library
install(TARGETS opal
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # For Windows DLL files
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*.hpp")

if (OPAL_BUILD_TESTS)
    set(OPAL_TEST_FILES
            test/main-test.cpp
            test/allocator-test.cpp
            test/dynamic-array-test.cpp
            test/deque-test.cpp
            test/array-view-test.cpp
            test/string-test.cpp
            test/expected-test.cpp
            test/time-test.cpp
            test/sort-test.cpp
            test/paths-test.cpp
            test/scope-ptr-test.cpp
            test/common-test.cpp
            third-party/catch2/src/catch_amalgamated.cpp)
    add_executable(opal_test ${OPAL_TEST_FILES})
    target_include_directories(opal_test PRIVATE third-party/catch2/include)
    target_include_directories(opal_test PRIVATE third-party/catch2/include/catch2)
    target_link_libraries(opal_test opal)
    target_link_libraries(opal_test opal_warnings)
    target_link_libraries(opal_test opal_options)
    target_compile_definitions(opal_test PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN)
    enable_testing()
    add_test(NAME opal_test COMMAND opal_test)
    # Install the test executable
    install(TARGETS opal_test
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # For Windows DLL files
    )
endif ()
