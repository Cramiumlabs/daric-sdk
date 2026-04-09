
add_library(usercase STATIC)

# Get project and board configurations
target_link_libraries(usercase PRIVATE configs)

target_link_libraries(usercase PRIVATE driver)
target_link_libraries(usercase PRIVATE threadx)
target_link_libraries(usercase PRIVATE bsp)

target_include_directories(usercase PUBLIC Inc)

target_sources(usercase PRIVATE Src/main.c)