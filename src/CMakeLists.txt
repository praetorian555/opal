add_library(opal
        allocator.cpp
        ../include/opal/assert.h
        ../include/opal/type-traits.h
        ../include/opal/allocator.h
        ../include/opal/types.h
        ../include/opal/error-codes.h
        ../include/opal/container/expected.h
        ../include/opal/container/array.h)
target_include_directories(opal PUBLIC ../include)
target_link_libraries(opal PRIVATE opal_warnings)
target_link_libraries(opal PRIVATE opal_options)