add_library(opal
        allocator.cpp
        time.cpp
        string.cpp
        string-encoding.cpp
        ../include/opal/defines.h
        ../include/opal/assert.h
        ../include/opal/type-traits.h
        ../include/opal/allocator.h
        ../include/opal/types.h
        ../include/opal/error-codes.h
        ../include/opal/time.h
        ../include/opal/string-encoding.h
        ../include/opal/container/expected.h
        ../include/opal/container/deque.h
        ../include/opal/container/span.h
        ../include/opal/container/string.h
        ../include/opal/container/array.h)
target_include_directories(opal PUBLIC ../include)
target_link_libraries(opal PRIVATE opal_warnings)
target_link_libraries(opal PRIVATE opal_options)