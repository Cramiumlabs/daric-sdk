function(target_sources_if_not_overridden filename)
    list(FIND TX_SRC_OVERRIDES ${filename} OVERRIDE_FOUND)
    if( OVERRIDE_FOUND EQUAL -1 )
        # message(STATUS "** Using original ${filename} from common/src **")
        target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/${filename})
    endif()
endfunction()

target_sources(${PROJECT_NAME}
    PRIVATE
    # {{BEGIN_TARGET_SOURCES}}
    ${CMAKE_CURRENT_LIST_DIR}/tx_low_power.c
    ${CMAKE_CURRENT_LIST_DIR}/tx_low_power_user.c

    # {{END_TARGET_SOURCES}}
)

# Add the Common/inc directory to the project include list
target_include_directories(${PROJECT_NAME}
SYSTEM
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)