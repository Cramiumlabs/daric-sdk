cmake_minimum_required(VERSION 3.16)

# Because the cross-compiler cannot directly generate a binary without complaining, 
# just test compiling a static library instead of an executable program
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_ASM_COMPILER_WORKS 1)

set(CMAKE_TOOLCHAIN_FILE toolchain-arm-none-eabi.cmake)

project(uart_demo C ASM)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-error -Wno-shadow -Wno-format)

set(TARGET uart_demo)
get_filename_component(GCC_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
get_filename_component(ROOT_DIR "${GCC_PROJECT_DIR}/../../../../" ABSOLUTE)

# Include project and board configurations
include(${GCC_PROJECT_DIR}/GCC/project.cmake)
include(${ROOT_DIR}/bsp/${CONFIG_BOARD}/board.cmake)

# Define configurations for C code
add_definitions(-DCONFIG_SOC_DARIC_NTO_A=1)
if(CONFIG_SUPPORT_800MFREQ STREQUAL "y")
    add_definitions(-DCONFIG_SUPPORT_800MFREQ)
endif()
if(HAL_UART_MODULE_ENABLED STREQUAL "y")
    add_definitions(-DHAL_UART_MODULE_ENABLED=1)
endif()
if(HAL_UDMA_MODULE_ENABLED STREQUAL "y")
    add_definitions(-DHAL_UDMA_MODULE_ENABLED=1)
endif()
if(HAL_NVIC_MODULE_ENABLED STREQUAL "y")
    add_definitions(-DHAL_NVIC_MODULE_ENABLED=1)
endif()
if(HAL_IFRAMMGR_MODULE_ENABLED STREQUAL "y")
    add_definitions(-DHAL_IFRAMMGR_MODULE_ENABLED=1)
endif()
if(HAL_PINMAP_MODULE_ENABLED STREQUAL "y")
    add_definitions(-DHAL_PINMAP_MODULE_ENABLED=1)
endif()
if(HAL_GPIO_MODULE_ENABLED STREQUAL "y")
    add_definitions(-DHAL_GPIO_MODULE_ENABLED=1)
endif()
if(HAL_ATIMER_MODULE_ENABLED STREQUAL "y")
    add_definitions(-DHAL_ATIMER_MODULE_ENABLED=1)
endif()
if(HAL_I2C_MODULE_ENABLED STREQUAL "y")
    add_definitions(-DHAL_I2C_MODULE_ENABLED=1)
endif()

add_definitions(-DCONFIG_PMIC_I2C_ID=${CONFIG_PMIC_I2C_ID})
add_definitions(-DCONFIG_PMIC_I2C_SPEED=${CONFIG_PMIC_I2C_SPEED})
add_definitions(-DCONFIG_PMIC_I2C_ADDR=${CONFIG_PMIC_I2C_ADDR})

# Find all sources in project
set(project_srcs
    "${GCC_PROJECT_DIR}/src/main.c"
    "${GCC_PROJECT_DIR}/src/syscalls.c"
    "${GCC_PROJECT_DIR}/src/startup_daric.c"
)

# Find necessary driver sources
set(DRIVER_SRC_DIR "${ROOT_DIR}/drivers/Src")
set(BSP_COMPONENT_DIR "${ROOT_DIR}/bsp/Components")

set(driver_srcs
    "${DRIVER_SRC_DIR}/daric_hal_uart.c"
    "${DRIVER_SRC_DIR}/daric_hal_nvic.c"
    "${DRIVER_SRC_DIR}/daric_hal.c"
    "${DRIVER_SRC_DIR}/daric_ifram.c"
    "${DRIVER_SRC_DIR}/daric_hal_pinmap.c"
    "${DRIVER_SRC_DIR}/daric_hal_gpio.c"
    "${DRIVER_SRC_DIR}/daric_hal_i2c.c"
    "${DRIVER_SRC_DIR}/daric_hal_atimer.c"
    "${DRIVER_SRC_DIR}/arch/arm/system_daric.c"
    "${DRIVER_SRC_DIR}/../Tlsf/Src/daric_tlsf.c"
    "${BSP_COMPONENT_DIR}/TG28/tg28.c"
)

set(main_srcs ${project_srcs} ${driver_srcs})

add_executable(${TARGET} ${main_srcs})

# Include directories
target_include_directories(${TARGET} PRIVATE 
    ${GCC_PROJECT_DIR}/inc
    ${ROOT_DIR}/drivers/Inc
    ${ROOT_DIR}/drivers/Inc/arch/arm
    ${ROOT_DIR}/drivers/Tlsf/Inc
    ${ROOT_DIR}/bsp/include
    ${ROOT_DIR}/bsp/Components/Common
    ${ROOT_DIR}/bsp/Components/TG28
)

target_include_directories(${TARGET} PRIVATE
    ${ROOT_DIR}/middleware/third_party/cmsis/CMSIS/Include
    ${ROOT_DIR}/middleware/third_party/cmsis/CMSIS/Core/Include
)

set_target_properties(${TARGET} PROPERTIES SUFFIX ".elf")

# Linker script selection
if(CONFIG_EXECUTION_MODE STREQUAL "SRAM_MODE")
    set(GCC_LD_FILE "${GCC_PROJECT_DIR}/GCC/ld/uart_demo_sram.ld")
else()
    set(GCC_LD_FILE "${GCC_PROJECT_DIR}/GCC/ld/uart_demo_reram.ld")
endif()

target_link_directories(${TARGET} PRIVATE . ${GCC_PROJECT_DIR}/GCC)

target_link_options(${TARGET} PRIVATE
    -T ${GCC_LD_FILE}
    -Wl,-Map=${TARGET}.map
    -u _printf_float
)

target_link_libraries(${TARGET} PRIVATE m)

add_custom_target(${TARGET}.bin ALL
    DEPENDS ${TARGET}
    COMMAND ${CMAKE_OBJCOPY} -Obinary ${TARGET}.elf ${TARGET}.bin
    COMMAND ${CMAKE_OBJCOPY} -Oihex ${TARGET}.elf ${TARGET}.hex
    COMMAND ${CMAKE_OBJDUMP} -S ${TARGET}.elf > ${TARGET}.S
)
