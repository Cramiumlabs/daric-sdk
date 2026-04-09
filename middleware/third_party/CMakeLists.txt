
add_library(libs INTERFACE)

if(CONFIG_MBEDTLS_LIB)
    add_subdirectory(mbedtls)
    target_link_libraries(libs INTERFACE mbedtlslib)
endif(CONFIG_MBEDTLS_LIB)

if(CONFIG_NEWLIB_LIBC)
    add_subdirectory(libc/newlib)
    target_link_libraries(libs INTERFACE newlib_libc)
endif(CONFIG_NEWLIB_LIBC)
