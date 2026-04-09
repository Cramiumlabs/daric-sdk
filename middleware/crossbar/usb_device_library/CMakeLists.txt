add_library(usb_device STATIC)

# Get driver headers
target_link_libraries(usb_device PRIVATE driver)
target_link_libraries(usb_device PRIVATE threadx)
target_link_libraries(usb_device PRIVATE c)
# Get project and board configurations
target_link_libraries(usb_device PRIVATE configs)

target_include_directories(usb_device PUBLIC ${CMAKE_CURRENT_LIST_DIR}/class/cdc/Inc)
if(CONFIG_USB_HID)
target_include_directories(usb_device PUBLIC ${CMAKE_CURRENT_LIST_DIR}/class/hid/Inc)
elseif(CONFIG_FIDO_USB_HID)
target_include_directories(usb_device PUBLIC ${CMAKE_CURRENT_LIST_DIR}/class/hid/Inc/Fidohid)
endif()

target_include_directories(usb_device PUBLIC ${CMAKE_CURRENT_LIST_DIR}/core/Inc)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/core/Src srcs)
target_sources(usb_device PRIVATE ${srcs})

if(CONFIG_USB_CDC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/class/cdc/Src srcs)
target_sources(usb_device PRIVATE ${srcs})
endif(CONFIG_USB_CDC)

if(CONFIG_USB_HID)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/class/hid/Src srcs)
target_sources(usb_device PRIVATE ${srcs})
elseif(CONFIG_FIDO_USB_HID)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/class/hid/Src/Fidohid srcs)
target_sources(usb_device PRIVATE ${srcs})
endif()