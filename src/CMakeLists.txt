file(GLOB LIB_SOURCES "*.cc")

add_library(network_stack_debug STATIC ${LIB_SOURCES})

add_library(network_stack_sanitized EXCLUDE_FROM_ALL STATIC ${LIB_SOURCES})
target_compile_options(network_stack_sanitized PUBLIC ${SANITIZING_FLAGS})

add_library(network_stack_optimized EXCLUDE_FROM_ALL STATIC ${LIB_SOURCES})
target_compile_options(network_stack_optimized PUBLIC "-O2")
