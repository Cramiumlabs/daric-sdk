
add_library(driver STATIC)

add_definitions(-DCONFIG_RERAM_START=0x60000000)
#Code area size 3952K,daric_datasheet_v2.5 2.4.1RRC introduction
#add_definitions(-DCONFIG_RERAM_SIZE=0x3DBFFF)
add_definitions(-DCONFIG_RERAM_SIZE=0x400000)

# Get project and board configurations
target_link_libraries(driver PRIVATE configs)

target_link_libraries(driver PUBLIC cmsis)
target_link_libraries(driver PRIVATE threadx)
add_definitions(-DSOC_DARIC_NTO_A)

target_include_directories(driver PUBLIC Inc)
target_include_directories(driver PUBLIC Inc/arch/arm)
target_include_directories(driver PUBLIC Inc/sce)
target_include_directories(driver PUBLIC Tlsf/Inc)
aux_source_directory(Src srcs)
aux_source_directory(Src/sce srcs)
aux_source_directory(Src/misc srcs)
aux_source_directory(Src/arch/arm srcs)
aux_source_directory(Tlsf/Src srcs)
target_sources(driver PRIVATE ${srcs})

