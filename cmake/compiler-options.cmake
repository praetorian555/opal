function(setup_compiler_options target)

    target_compile_options(${target} INTERFACE "/MP")

endfunction()