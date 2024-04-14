add_library(opal
        empty.cpp
        ../include/opal/assert.h
        ../include/opal/type-traits.h
        ../include/opal/container/expected.h
        ../include/opal/container/dynamic-array.h)
target_include_directories(opal PUBLIC ../include)
target_link_libraries(opal PRIVATE opal_warnings)
target_link_libraries(opal PRIVATE opal_options)