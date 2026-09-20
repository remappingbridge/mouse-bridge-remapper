cmake_minimum_required(VERSION 3.20)

option(MBR_BUILD_PICO "Build Raspberry Pi Pico 2 W production scaffold" OFF)
option(MBR_BUILD_TESTS "Build host tests" ON)
option(MBR_BUILD_PICO_QUALIFICATION "Build the MBR-03 renderer/HAT qualification firmware" OFF)
option(MBR_BUILD_USB_HID_QUALIFICATION "Build the MBR-04 USB HID qualification firmware" OFF)

if(MBR_BUILD_PICO)
    if((NOT DEFINED PICO_SDK_PATH OR PICO_SDK_PATH STREQUAL "") AND
       DEFINED ENV{PICO_SDK_PATH} AND NOT "$ENV{PICO_SDK_PATH}" STREQUAL "")
        set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}" CACHE PATH "Path to Raspberry Pi Pico SDK")
    endif()
    if(NOT DEFINED PICO_SDK_PATH OR PICO_SDK_PATH STREQUAL "")
        message(FATAL_ERROR "PICO_SDK_PATH is required when MBR_BUILD_PICO=ON")
    endif()
    include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")
endif()

project(mouse_bridge_remapper VERSION 0.1.0 LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(MBR_BUILD_PICO)
    if(NOT PICO_BOARD STREQUAL "pico2_w")
        message(FATAL_ERROR "Production scaffold is intentionally fixed to PICO_BOARD=pico2_w")
    endif()
    pico_sdk_init()
endif()

include(cmake/MouseBridgeRemapperModules.cmake)
mbr_declare_contract_modules()

foreach(module IN LISTS MBR_MODULES)
    add_library(mbr_impl_${module} STATIC src/${module}/${module}.c)
    target_include_directories(mbr_impl_${module} PUBLIC include)
endforeach()

mbr_wire_contract_modules()

if(MBR_BUILD_PICO)
    target_compile_definitions(mbr_impl_renderer PRIVATE MBR_PLATFORM_PICO=1)
    target_compile_definitions(mbr_impl_hat PRIVATE MBR_PLATFORM_PICO=1)
    target_link_libraries(mbr_impl_renderer PUBLIC pico_stdlib hardware_gpio hardware_spi)
    target_link_libraries(mbr_impl_hat PUBLIC pico_stdlib hardware_gpio)

    target_sources(mbr_impl_usb_hid PRIVATE
        src/usb_hid/usb_hid_pico.c
        src/usb_hid/usb_descriptors.c
    )
    target_link_libraries(mbr_impl_usb_hid PUBLIC tinyusb_device)

    target_sources(mbr_impl_bt_runtime PRIVATE
        src/bt_runtime/bt_runtime_pico.c
    )
    target_link_libraries(mbr_impl_bt_runtime PRIVATE
        pico_stdlib
        pico_btstack_ble
        pico_btstack_cyw43
        pico_cyw43_arch_threadsafe_background
    )
    target_compile_definitions(mbr_impl_bt_runtime PUBLIC
        CYW43_LWIP=0
        PICO_BTSTACK_CYW43_MAX_HCI_PROCESS_LOOP_COUNT=8
    )
    pico_btstack_make_gatt_header(
        mbr_impl_bt_runtime
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/bt_runtime/g05_hog_host.gatt
    )

    target_sources(mbr_impl_ble_hogp PRIVATE
        src/ble_hogp/ble_hogp_pico.c
    )
    target_link_libraries(mbr_impl_ble_hogp PRIVATE
        pico_stdlib
        pico_btstack_ble
        mbr_impl_bt_runtime
    )
endif()

target_compile_definitions(mbr_impl_domain PUBLIC MBR_VERSION_STRING="0.5.0-mbr05")

if(MBR_BUILD_TESTS)
    enable_testing()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    add_executable(mbr_test_bootstrap tests/test_bootstrap.c)
    target_link_libraries(mbr_test_bootstrap PRIVATE mbr_module_app)
    add_test(NAME bootstrap_contract COMMAND mbr_test_bootstrap)

    add_executable(mbr_test_ux_golden tests/test_ux_golden.c)
    target_link_libraries(mbr_test_ux_golden PRIVATE mbr_module_app)
    add_test(NAME ux_golden_contract COMMAND mbr_test_ux_golden)

    add_executable(mbr_test_ux_behavior tests/test_ux_behavior.c)
    target_link_libraries(mbr_test_ux_behavior PRIVATE mbr_module_app)
    add_test(NAME ux_behavior_contract COMMAND mbr_test_ux_behavior)

    add_executable(mbr_test_renderer tests/test_renderer.c)
    target_link_libraries(mbr_test_renderer PRIVATE mbr_module_app)
    add_test(NAME renderer_hat_contract COMMAND mbr_test_renderer)

    add_executable(mbr_test_usb_hid tests/test_usb_hid.c)
    target_link_libraries(mbr_test_usb_hid PRIVATE mbr_module_usb_hid)
    add_test(NAME usb_hid_contract COMMAND mbr_test_usb_hid)

    add_executable(mbr_test_output_state tests/test_output_state.c)
    target_link_libraries(mbr_test_output_state PRIVATE mbr_module_output_state)
    add_test(NAME output_state_contract COMMAND mbr_test_output_state)

    add_executable(mbr_test_ble_hogp tests/test_ble_hogp.c)
    target_link_libraries(mbr_test_ble_hogp PRIVATE mbr_module_ble_hogp)
    add_test(NAME ble_hogp_contract COMMAND mbr_test_ble_hogp)

    add_test(
        NAME architecture_contract
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_architecture.py
                ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

if(MBR_BUILD_PICO)
    add_executable(mouse_bridge_remapper src/app/main.c)
    target_link_libraries(mouse_bridge_remapper PRIVATE pico_stdlib mbr_module_app)

    pico_enable_stdio_usb(mouse_bridge_remapper 0)
    pico_enable_stdio_uart(mouse_bridge_remapper 0)
    pico_add_extra_outputs(mouse_bridge_remapper)
endif()

if(MBR_BUILD_PICO AND MBR_BUILD_USB_HID_QUALIFICATION)
    add_executable(mbr_usb_hid_qualification tests/pico_usb_hid_qualification.c)
    target_link_libraries(mbr_usb_hid_qualification PRIVATE pico_stdlib mbr_module_usb_hid mbr_module_hat)
    pico_enable_stdio_usb(mbr_usb_hid_qualification 0)
    pico_enable_stdio_uart(mbr_usb_hid_qualification 0)
    pico_add_extra_outputs(mbr_usb_hid_qualification)
endif()

if(MBR_BUILD_PICO AND MBR_BUILD_PICO_QUALIFICATION)
    add_executable(mbr_renderer_hat_qualification tests/pico_renderer_hat_qualification.c)
    target_link_libraries(mbr_renderer_hat_qualification PRIVATE pico_stdlib mbr_module_app)
    pico_enable_stdio_usb(mbr_renderer_hat_qualification 0)
    pico_enable_stdio_uart(mbr_renderer_hat_qualification 0)
    pico_add_extra_outputs(mbr_renderer_hat_qualification)
endif()
