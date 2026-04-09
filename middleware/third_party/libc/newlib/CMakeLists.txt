# SPDX-License-Identifier: Apache-2.0

add_library(newlib_libc STATIC)

# Get project and board configurations
target_link_libraries(newlib_libc PRIVATE configs)
# Implement malloc APIs using ThreadX memory pool
target_link_libraries(newlib_libc PRIVATE threadx)

target_sources(newlib_libc PRIVATE libc-hooks.c)

if(CONFIG_NEWLIB_LIBC_NANO)
    target_link_libraries(newlib_libc INTERFACE -specs=nano.specs)
    target_compile_options(newlib_libc INTERFACE -specs=nano.specs)
endif(CONFIG_NEWLIB_LIBC_NANO)

target_compile_options(newlib_libc PRIVATE -fno-builtin)

# Links libc libm and libgcc
target_link_libraries(newlib_libc INTERFACE c m gcc)

# Forces the linker to include malloc related symbols from this component,
# instead of the implementations provided by newlib.
list(APPEND EXTRA_LINK_FLAGS "-u malloc")
list(APPEND EXTRA_LINK_FLAGS "-u calloc")
list(APPEND EXTRA_LINK_FLAGS "-u free")
list(APPEND EXTRA_LINK_FLAGS "-u realloc")
list(APPEND EXTRA_LINK_FLAGS "-u _malloc_r")
list(APPEND EXTRA_LINK_FLAGS "-u _calloc_r")
list(APPEND EXTRA_LINK_FLAGS "-u _free_r")
list(APPEND EXTRA_LINK_FLAGS "-u _realloc_r")
list(APPEND EXTRA_LINK_FLAGS "-u aligned_alloc")

target_link_libraries(newlib_libc INTERFACE "${EXTRA_LINK_FLAGS}")
