function(setup_compiler_options target)

    target_compile_options(${target} INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/MP>)
    # Source code is encoded using UTF-8
    target_compile_options(${target} INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

    target_compile_definitions(${target} INTERFACE $<$<CXX_COMPILER_ID:MSVC>:UNICODE>)
    target_compile_definitions(${target} INTERFACE $<$<CXX_COMPILER_ID:MSVC>:_UNICODE>)

endfunction()