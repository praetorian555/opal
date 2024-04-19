add_executable(opal_test
        allocator-test.cpp
        array-test.cpp
        deque-test.cpp
        expected-test.cpp)
target_link_libraries(opal_test opal)
target_link_libraries(opal_test catch2)
target_link_libraries(opal_test opal_warnings)
target_link_libraries(opal_test opal_options)