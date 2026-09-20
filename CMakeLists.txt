cmake_minimum_required(VERSION 3.20)

option(MBR_BUILD_PICO "Build Raspberry Pi Pico 2 W production scaffold" OFF)
option(MBR_BUILD_TESTS "Build host tests" ON)

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

target_compile_definitions(mbr_impl_domain PUBLIC MBR_VERSION_STRING="0.1.0-mbr01")

if(MBR_BUILD_TESTS)
    enable_testing()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    add_executable(mbr_test_bootstrap tests/test_bootstrap.c)
    target_link_libraries(mbr_test_bootstrap PRIVATE mbr_module_app)
    add_test(NAME bootstrap_contract COMMAND mbr_test_bootstrap)

    add_executable(mbr_test_ux tests/test_ux.c)
    target_link_libraries(mbr_test_ux PRIVATE mbr_module_app)
    add_test(NAME ux_contract COMMAND mbr_test_ux)

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
