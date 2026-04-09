# SPDX-License-Identifier: Apache-2.0

add_library(mbedtlslib STATIC)
set(MBEDTLS_VERSION 3.6.0)

# Get project and board configurations
target_link_libraries(mbedtlslib PRIVATE configs)

target_include_directories(mbedtlslib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/mbedtls-${MBEDTLS_VERSION}/include)

#message(${CMAKE_CURRENT_LIST_DIR}/mbedtls-${MBEDTLS_VERSION}/library)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/mbedtls-${MBEDTLS_VERSION}/library SRC_FILES)

target_sources(mbedtlslib PRIVATE ${SRC_FILES})

