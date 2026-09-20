cmake_minimum_required(VERSION 3.20)

option(MBR_BUILD_PICO "Build Raspberry Pi Pico 2 W production scaffold" OFF)
option(MBR_BUILD_TESTS "Build host tests" ON)
option(MBR_BUILD_PICO_QUALIFICATION "Build the MBR-03 renderer/HAT qualification firmware" OFF)

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
endif()

target_compile_definitions(mbr_impl_domain PUBLIC MBR_VERSION_STRING="0.1.0-mbr01")

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

if(MBR_BUILD_PICO AND MBR_BUILD_PICO_QUALIFICATION)
    add_executable(mbr_renderer_hat_qualification tests/pico_renderer_hat_qualification.c)
    target_link_libraries(mbr_renderer_hat_qualification PRIVATE pico_stdlib mbr_module_app)
    pico_enable_stdio_usb(mbr_renderer_hat_qualification 0)
    pico_enable_stdio_uart(mbr_renderer_hat_qualification 0)
    pico_add_extra_outputs(mbr_renderer_hat_qualification)
endif()
