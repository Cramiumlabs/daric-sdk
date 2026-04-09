# SPDX-License-Identifier: MIT License

add_library(guix STATIC)
set(GUIX_VERSION 6.4.1)

# Get project and board configurations
target_link_libraries(guix PRIVATE configs)
target_link_libraries(guix PRIVATE threadx)

target_include_directories(guix PUBLIC ${CMAKE_CURRENT_LIST_DIR}/guix-${GUIX_VERSION}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN}/inc)
target_include_directories(guix PUBLIC ${CMAKE_CURRENT_LIST_DIR}/guix-${GUIX_VERSION}/common/inc)

#message(${CMAKE_CURRENT_LIST_DIR}/guix-${GUIX_VERSION}/common/src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/guix-${GUIX_VERSION}/common/src SRC_FILES)

#[[
    This section is intended to bypass strict compilation checks to resolve dangling-pointer compilation errors, 
    as the Guix code can ensure the correct use of pointers to local variables.
    UINT _gx_canvas_arc_draw(INT xcenter, INT ycenter, UINT r, INT start_angle, INT end_angle)
    {
        ......
        GX_RECTANGLE     clip_rect;
        ......
        context->gx_draw_context_clip = &clip_rect; //<------ dangling-pointer
        display->gx_display_driver_arc_fill(context, xcenter, ycenter, r, start_angle, end_angle);
        ......
    }    
]]
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/guix-${GUIX_VERSION}/common/src/gx_canvas_arc_draw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/guix-${GUIX_VERSION}/common/src/gx_canvas_circle_draw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/guix-${GUIX_VERSION}/common/src/gx_canvas_ellipse_draw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/guix-${GUIX_VERSION}/common/src/gx_canvas_polygon_draw.c
    PROPERTIES COMPILE_FLAGS "-Wno-dangling-pointer"
)

target_sources(guix PRIVATE ${SRC_FILES})

