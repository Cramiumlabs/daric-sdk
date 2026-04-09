
# target_include_directories(daric_crypto_lib PUBLIC
#     "${CMAKE_CURRENT_SOURCE_DIR}/../../drivers/inc"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../../drivers/src/sce/"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../../device/inc"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../daric_sdk/devices/Include/"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../daric_sdk/middleware/cmsis/CMSIS/Core/Include"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../../../inc"
#     #"${CMAKE_CURRENT_SOURCE_DIR}/../../cifra/src/"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/crypto/"
# )

add_library(daric_crypto STATIC)

set(CMAKE_C_STANDARD 99)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shadow -Wno-unused-variable -Wno-unused-function -Wno-implicit-function-declaration -Wno-parentheses -Wno-maybe-uninitialized")
# target_compile_options(daric_crypto_lib PRIVATE -Wno-shadow)

target_link_libraries(daric_crypto PRIVATE driver)
target_link_libraries(daric_crypto PRIVATE threadx)
target_link_libraries(daric_crypto PRIVATE c)

target_include_directories(daric_crypto PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/.
)


aux_source_directory(${CMAKE_CURRENT_LIST_DIR} srcs)
target_sources(daric_crypto PRIVATE ${srcs})