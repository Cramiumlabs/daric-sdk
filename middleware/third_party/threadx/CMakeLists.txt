
add_library(threadx STATIC)

# We can't establish dependency on what we don't yet know, so we force the
# linker to not drop this symbol.
target_link_libraries(threadx INTERFACE "-u main")

# Get project and board configurations
target_link_libraries(threadx PRIVATE configs)

# Get driver headers
target_link_libraries(threadx PRIVATE driver)

# Get cmsis headers
target_link_libraries(threadx PRIVATE cmsis)

if(CONFIG_CPU_CORTEX_M7)
    set(THREADX_ARCH "cortex_m7" CACHE STRING "cortex_m7")
    set(THREADX_TOOLCHAIN "gnu" CACHE STRING "gnu")
else()
    message(FATAL_ERROR "Not yet supported cpu!")
endif()

target_sources(threadx PRIVATE ${CMAKE_CURRENT_LIST_DIR}/init.c)
target_sources(threadx PRIVATE ${CMAKE_CURRENT_LIST_DIR}/sleep.c)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/util util_srcs)
target_sources(threadx PRIVATE ${util_srcs})

set(PROJECT_NAME threadx) # for add_subdirectory only
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/low_level)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/kernel/common)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/kernel/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/kernel/utility/low_power)

target_include_directories(threadx PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_definitions(threadx PUBLIC "TX_INCLUDE_USER_DEFINE_FILE" )

if(CONFIG_THREADX_TEST)
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/kernel/test/tx/regression test_srcs)
    target_sources(threadx PRIVATE ${test_srcs})

    target_compile_definitions(threadx PRIVATE EXTERNAL_MAIN EXTERNAL_EXIT)
    target_compile_definitions(threadx PRIVATE TX_DISABLE_NOTIFY_CALLBACKS)
    target_compile_definitions(threadx PRIVATE TX_ENABLE_STACK_CHECKING)
    target_compile_definitions(threadx PRIVATE TX_REGRESSION_TEST)
    target_compile_definitions(threadx PRIVATE TEST_FLAG=UINT)
    target_compile_definitions(threadx PRIVATE TEST_STACK_SIZE_PRINTF=4096)

    target_compile_options(threadx PRIVATE -Wno-error=missing-braces)
endif(CONFIG_THREADX_TEST)