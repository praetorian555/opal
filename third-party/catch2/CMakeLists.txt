add_library(catch2 src/catch_amalgamated.cpp)
target_include_directories(catch2 PRIVATE include/catch2)
target_include_directories(catch2 PUBLIC include)
