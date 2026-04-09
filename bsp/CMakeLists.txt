add_library(bsp INTERFACE)

# Get project and board configurations
#target_link_libraries(bsp PRIVATE configs)

#target_link_libraries(bsp PRIVATE driver)
#target_link_libraries(bsp PRIVATE threadx)
#target_link_libraries(bsp PRIVATE c)

add_subdirectory(${CONFIG_BOARD} ${CONFIG_BOARD})
target_link_libraries(bsp INTERFACE ${CONFIG_BOARD})