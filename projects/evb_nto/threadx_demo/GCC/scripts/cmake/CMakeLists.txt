cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_TOOLCHAIN_FILE toolchain-arm-none-eabi.cmake)

project(threadx_demo C ASM)

set(TARGET threadx_demo)
set(GCC_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
include(${GCC_PROJECT_DIR}/project.cmake)

aux_source_directory(${GCC_PROJECT_DIR}/../Src main_srcs)

# shachuang remove test case code

add_executable(${TARGET} ${main_srcs})
target_include_directories(${TARGET} PRIVATE ${GCC_PROJECT_DIR}/../Inc)

add_library(configs INTERFACE)
target_include_directories(configs INTERFACE ${GCC_PROJECT_DIR}/../Inc)
target_include_directories(configs INTERFACE ${GCC_PROJECT_DIR}/../Inc/config)

include(${GCC_PROJECT_DIR}/../../../../bsp/${CONFIG_BOARD}/board.cmake)
include(${GCC_PROJECT_DIR}/scripts/cmake/gen_configs.cmake)
target_link_libraries(${TARGET} PRIVATE configs)

add_subdirectory(${GCC_PROJECT_DIR}/../../../../middleware/third_party/cmsis        cmsis)
add_subdirectory(${GCC_PROJECT_DIR}/../../../../drivers                 driver)
add_subdirectory(${GCC_PROJECT_DIR}/../../../../middleware/third_party/threadx      threadx)
add_subdirectory(${GCC_PROJECT_DIR}/../../../../middleware/third_party         libs)
add_subdirectory(${GCC_PROJECT_DIR}/../../../../bsp                     bsp)
# add_subdirectory(${GCC_PROJECT_DIR}/../../../../daric_sdk/middleware/SEGGER_RTT_V630d         rtt)

# target_link_libraries(${TARGET} PRIVATE rtt)
target_link_libraries(${TARGET} PRIVATE driver)
target_link_libraries(${TARGET} PRIVATE threadx)
target_link_libraries(${TARGET} PRIVATE cmsis)
target_link_libraries(${TARGET} PRIVATE bsp)

if(CONFIG_MPC_SDK STREQUAL "y")
    target_link_libraries(${TARGET} PRIVATE rust_mpc_sdk)
endif()

# Make sure "libs" is always linked behind all other libraries.
target_link_libraries(${TARGET} PRIVATE libs)

# Force bndivision_le symbol from daric_crypto to be linked (needed by mini_gmp SCE wrapper)
# This ensures the SCE hardware division function is available for BICYCL acceleration
target_link_options(${TARGET} PRIVATE "-Wl,-u,bndivision_le")

set_target_properties(${TARGET} PROPERTIES SUFFIX ".elf")

if(CONFIG_EXECUTION_MODE STREQUAL "SRAM_MODE")
    set(GCC_LD_FILE "../ld/daric_sram.ld")
    message(DEBUG "Compiling for SRAM_MODE")
elseif(CONFIG_EXECUTION_MODE STREQUAL "ITCM_MODE")
    set(GCC_LD_FILE "../ld/daric_itcm.ld")
    message(DEBUG "Compiling for ITCM_MODE")
elseif(CONFIG_EXECUTION_MODE STREQUAL "RERAM_BLOADER_MODE")
    set(GCC_LD_FILE "../ld/daric_reram_bloader.ld")
    message(DEBUG "Compiling for RERAM_BLOADER_MODE")
else()
    set(GCC_LD_FILE "../ld/daric_reram.ld")
    message(DEBUG "Compiling for RERAM_MODE")
endif()

target_link_options(${TARGET}
    PRIVATE
        -T ${GCC_LD_FILE}
        -Wl,-Map=${TARGET}.map
        # Allow multiple definitions for Rust allocator symbols
        # Multiple Rust staticlibs (bicycl_support, bicycl_testcase, tecdsa_embedded)
        # all define the same ThreadX allocator from bicycl_common
        -Wl,--allow-multiple-definition
)

add_custom_target(${TARGET}.bin ALL
    DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${TARGET}.elf ${TARGET}.bin
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${TARGET}.elf ${TARGET}.hex
        COMMAND ${CMAKE_OBJDUMP} -S ${TARGET}.elf > ${TARGET}.S
)
