# SPDX-License-Identifier: MIT License

add_library(levelx STATIC)
set(LEVELX_VERSION 6.4.1)

# Get project and board configurations
target_link_libraries(levelx PRIVATE configs)
target_link_libraries(levelx PRIVATE threadx)
target_link_libraries(levelx PRIVATE driver)

target_include_directories(levelx PUBLIC ${CMAKE_CURRENT_LIST_DIR}/levelx-${LEVELX_VERSION}/common/inc)
target_include_directories(levelx PUBLIC ${CMAKE_CURRENT_LIST_DIR}/ports/daric)

#message(${CMAKE_CURRENT_LIST_DIR}/levelx-${LEVELX_VERSION}/common/src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/levelx-${LEVELX_VERSION}/common/src SRC_FILES)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/ports/daric SRC_FILES)

target_sources(levelx PRIVATE ${SRC_FILES})

