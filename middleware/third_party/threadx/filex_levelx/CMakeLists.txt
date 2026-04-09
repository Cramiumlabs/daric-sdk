# SPDX-License-Identifier: MIT License

add_library(filex_levelx STATIC)
#set(FILEX_VERSION 6.4.1)

# Get project and board configurations
target_link_libraries(filex_levelx PRIVATE configs)
target_link_libraries(filex_levelx PRIVATE threadx)
target_link_libraries(filex_levelx PRIVATE driver)
target_link_libraries(filex_levelx PRIVATE filex)
target_link_libraries(filex_levelx PRIVATE levelx)

target_include_directories(filex_levelx PUBLIC ${CMAKE_CURRENT_LIST_DIR})

#message(${CMAKE_CURRENT_LIST_DIR}/filex_levelx-${FILEX_VERSION}/common/src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRC_FILES)


target_sources(filex_levelx PRIVATE ${SRC_FILES})

