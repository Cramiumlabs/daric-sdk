# SPDX-License-Identifier: MIT License

add_library(filex STATIC)
set(FILEX_VERSION 6.4.1)

# Get project and board configurations
target_link_libraries(filex PRIVATE configs)
target_link_libraries(filex PRIVATE threadx)
target_link_libraries(filex PRIVATE driver)

target_include_directories(filex PUBLIC ${CMAKE_CURRENT_LIST_DIR}/filex-${FILEX_VERSION}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN}/inc)
target_include_directories(filex PUBLIC ${CMAKE_CURRENT_LIST_DIR}/filex-${FILEX_VERSION}/common/inc)
target_include_directories(filex PUBLIC ${CMAKE_CURRENT_LIST_DIR}/ports/daric)

#message(${CMAKE_CURRENT_LIST_DIR}/filex-${FILEX_VERSION}/common/src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/filex-${FILEX_VERSION}/common/src SRC_FILES)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/ports/daric SRC_FILES)


target_sources(filex PRIVATE ${SRC_FILES})

