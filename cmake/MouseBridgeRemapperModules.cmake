# Frozen MBR module graph.
# Declaration and wiring are separate so each implementation library precedes
# its dependency libraries on static link lines once modules contain real code.

set(MBR_MODULES
    domain
    mouse_registry
    mouse_session
    output_state
    profiles
    remap
    pairing_coordinator
    bt_runtime
    ble_hogp
    logitech_hidpp
    product_storage
    usb_hid
    interaction
    ui_projector
    renderer
    hat
    app
)

set(MBR_DEPS_domain "")
set(MBR_DEPS_mouse_registry "domain")
set(MBR_DEPS_mouse_session "domain")
set(MBR_DEPS_output_state "domain;mouse_session")
set(MBR_DEPS_profiles "domain")
set(MBR_DEPS_remap "domain;profiles")
set(MBR_DEPS_pairing_coordinator "domain;mouse_registry;mouse_session;ble_hogp")
set(MBR_DEPS_bt_runtime "domain")
set(MBR_DEPS_ble_hogp "domain;bt_runtime")
set(MBR_DEPS_logitech_hidpp "domain;ble_hogp")
set(MBR_DEPS_product_storage "domain;mouse_registry;profiles")
set(MBR_DEPS_usb_hid "domain;output_state")
set(MBR_DEPS_interaction "domain")
set(MBR_DEPS_ui_projector "domain;mouse_registry;mouse_session;profiles;pairing_coordinator;interaction")
set(MBR_DEPS_renderer "ui_projector")
set(MBR_DEPS_hat "domain")
set(MBR_DEPS_app "domain;mouse_registry;mouse_session;output_state;profiles;remap;pairing_coordinator;ble_hogp;logitech_hidpp;product_storage;usb_hid;interaction;ui_projector;renderer;hat;bt_runtime")

function(mbr_declare_contract_modules)
    foreach(module IN LISTS MBR_MODULES)
        add_library(mbr_module_${module} INTERFACE)
    endforeach()
endfunction()

function(mbr_wire_contract_modules)
    foreach(module IN LISTS MBR_MODULES)
        target_link_libraries(mbr_module_${module} INTERFACE mbr_impl_${module})
        foreach(dep IN LISTS MBR_DEPS_${module})
            if(NOT dep STREQUAL "")
                target_link_libraries(mbr_module_${module} INTERFACE mbr_module_${dep})
            endif()
        endforeach()
    endforeach()
endfunction()
