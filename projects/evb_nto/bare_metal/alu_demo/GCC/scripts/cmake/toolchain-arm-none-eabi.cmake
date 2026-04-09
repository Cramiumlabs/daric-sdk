# usage
# cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-none-eabi.cmake ../

# The Generic system name is used for embedded targets (targets without OS) in
set(CMAKE_SYSTEM_NAME          Generic)
set(CMAKE_SYSTEM_PROCESSOR     arm)

# Only need to set this if the toolchain isn't in your system path.
if (NOT ARM_COMPILER_PATH)
    if (DEFINED ENV{ARM_NONE_EABI_TOOLCHAIN_BINPATH})
        set(ARM_COMPILER_PATH $ENV{ARM_NONE_EABI_TOOLCHAIN_BINPATH})
    elseif (DEFINED ENV{ARM_COMPILER_PATH})
        set(ARM_COMPILER_PATH $ENV{ARM_COMPILER_PATH})
    endif()

    if(ARM_COMPILER_PATH)
        get_filename_component(ARM_COMPILER_PATH "${ARM_COMPILER_PATH}" REALPATH)
        file(TO_CMAKE_PATH "${ARM_COMPILER_PATH}" ARM_COMPILER_PATH)
        string(APPEND ARM_COMPILER_PATH "/")
    endif()
endif()

message(STATUS "ARM_COMPILER_PATH @ ${ARM_COMPILER_PATH}")

# The toolchain prefix for all toolchain executables
set(CROSS_COMPILE arm-none-eabi-)

# specify the cross compiler. We force the compiler so that CMake doesn't
# attempt to build a simple test program as this will fail without us using
# the -nostartfiles option on the command line
find_program(CMAKE_C_COMPILER   NAMES ${CROSS_COMPILE}gcc   PATHS ${ARM_COMPILER_PATH} NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES ${CROSS_COMPILE}g++   PATHS ${ARM_COMPILER_PATH} NO_DEFAULT_PATH)
find_program(CMAKE_ASM_COMPILER NAMES ${CROSS_COMPILE}gcc   PATHS ${ARM_COMPILER_PATH} NO_DEFAULT_PATH)

if(NOT CMAKE_C_COMPILER)
    # If not found in specified path, try system path
    find_program(CMAKE_C_COMPILER   NAMES ${CROSS_COMPILE}gcc)
    find_program(CMAKE_CXX_COMPILER NAMES ${CROSS_COMPILE}g++)
    find_program(CMAKE_ASM_COMPILER NAMES ${CROSS_COMPILE}gcc)
endif()

# Set other tools
if(CMAKE_C_COMPILER)
    get_filename_component(COMPILER_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
    find_program(CMAKE_OBJCOPY  NAMES ${CROSS_COMPILE}objcopy   PATHS ${COMPILER_DIR})
    find_program(CMAKE_OBJDUMP  NAMES ${CROSS_COMPILE}objdump   PATHS ${COMPILER_DIR})
    find_program(CMAKE_SIZE     NAMES ${CROSS_COMPILE}size      PATHS ${COMPILER_DIR})
    find_program(CMAKE_NM       NAMES ${CROSS_COMPILE}nm        PATHS ${COMPILER_DIR})
    find_program(CMAKE_AR       NAMES ${CROSS_COMPILE}ar        PATHS ${COMPILER_DIR})
    find_program(CMAKE_RANLIB   NAMES ${CROSS_COMPILE}ranlib    PATHS ${COMPILER_DIR})
endif()


set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Because the cross-compiler cannot directly generate a binary without complaining, just test
# compiling a static library instead of an executable program
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Compiler and linker flags
set(CPU_FLAGS "-mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16")
set(COMMON_FLAGS "${CPU_FLAGS} -g3 -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin -fno-common -Wall -Wshadow -Wdouble-promotion -Wno-unused-parameter -Wno-error -Wno-shadow")

set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} -std=gnu99")
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")
set(CMAKE_ASM_FLAGS_INIT "${COMMON_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections -Wl,-print-memory-usage --specs=nano.specs --specs=nosys.specs")

set(CMAKE_C_FLAGS_DEBUG_INIT "-O0")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0")
set(CMAKE_ASM_FLAGS_DEBUG_INIT "")

# Set Release flags based on detected GCC version
# GCC_VERSION was detected earlier (after compiler was found)
# if(GCC_VERSION AND GCC_VERSION VERSION_GREATER_EQUAL "15.0")
    # GCC >= 15: Disable LTO due to assembly incompatibility
    set(CMAKE_C_FLAGS_RELEASE_INIT "-Os")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Os")
    set(CMAKE_C_FLAGS_RELEASE "-Os" CACHE STRING "Release C flags" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "-Os" CACHE STRING "Release CXX flags" FORCE)
# else()
#     # GCC < 15 or non-GCC: Enable LTO
#     set(CMAKE_C_FLAGS_RELEASE_INIT "-Os -flto")
#     set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Os -flto")
#     set(CMAKE_C_FLAGS_RELEASE "-Os -flto" CACHE STRING "Release C flags" FORCE)
#     set(CMAKE_CXX_FLAGS_RELEASE "-Os -flto" CACHE STRING "Release CXX flags" FORCE)
# endif()

set(CMAKE_ASM_FLAGS_RELEASE_INIT "")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE STRING "Release ASM flags" FORCE)

