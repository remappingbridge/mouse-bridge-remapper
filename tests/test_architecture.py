#!/usr/bin/env python3

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()

EXPECTED_DEPS = {
    "domain": [],
    "mouse_registry": ["domain"],
    "mouse_session": ["domain"],
    "output_state": ["domain", "mouse_session"],
    "profiles": ["domain"],
    "remap": ["domain", "profiles"],
    "pairing_coordinator": ["domain", "mouse_registry", "mouse_session", "ble_hogp"],
    "bt_runtime": ["domain"],
    "ble_hogp": ["domain", "bt_runtime"],
    "logitech_hidpp": ["domain", "ble_hogp"],
    "product_storage": ["domain", "mouse_registry", "profiles"],
    "usb_hid": ["domain", "output_state"],
    "interaction": ["domain"],
    "ui_projector": ["domain", "mouse_registry", "mouse_session", "profiles", "pairing_coordinator", "interaction"],
    "renderer": ["ui_projector"],
    "hat": ["domain"],
    "app": [
        "domain", "mouse_registry", "mouse_session", "output_state", "profiles", "remap",
        "pairing_coordinator", "bt_runtime", "ble_hogp", "logitech_hidpp", "product_storage",
        "usb_hid", "interaction", "ui_projector", "renderer", "hat",
    ],
}

PURE_MODULES = {
    "domain", "mouse_registry", "mouse_session", "output_state", "profiles", "remap",
    "pairing_coordinator", "interaction", "ui_projector",
}
BT_ALLOWED = {"bt_runtime", "ble_hogp", "logitech_hidpp"}
GPIO_SPI_ALLOWED = {"hat", "renderer"}
FLASH_ALLOWED = {"product_storage"}
TINYUSB_ALLOWED = {"usb_hid"}


def fail(message: str) -> None:
    raise AssertionError(message)


def module_for(path: Path) -> str | None:
    rel = path.relative_to(ROOT)
    if len(rel.parts) >= 2 and rel.parts[0] == "src":
        return rel.parts[1]
    if len(rel.parts) >= 3 and rel.parts[0] == "include" and rel.parts[1] == "mbr":
        return rel.parts[2]
    return None


def source_files() -> list[Path]:
    result: list[Path] = []
    for base in (ROOT / "src", ROOT / "include"):
        if not base.exists():
            continue
        for path in base.rglob("*"):
            if not path.is_file() or path.suffix.lower() not in {".c", ".h", ".cc", ".cpp", ".hpp"}:
                continue
            # TinyUSB's compile-time configuration is an ownership-boundary
            # file consumed by the usb_hid adapter, not an implementation
            # module and therefore has no module_for() owner.
            if path == ROOT / "include" / "tusb_config.h":
                continue
            result.append(path)
    return result


def check_module_graph() -> None:
    path = ROOT / "cmake" / "MouseBridgeRemapperModules.cmake"
    text = path.read_text(encoding="utf-8")
    match = re.search(r"set\(MBR_MODULES\s+(.*?)\n\)", text, re.S)
    if not match:
        fail("MBR_MODULES declaration is missing")
    modules = match.group(1).split()
    if modules != list(EXPECTED_DEPS):
        fail(f"module list/order differs from frozen MBR-01 graph: {modules}")

    for module, expected in EXPECTED_DEPS.items():
        dep_match = re.search(rf'set\(MBR_DEPS_{re.escape(module)}\s+"([^"]*)"\)', text)
        if not dep_match:
            fail(f"missing dependency declaration for {module}")
        actual = [item for item in dep_match.group(1).split(";") if item]
        if actual != expected:
            fail(f"dependency mismatch for {module}: expected {expected}, got {actual}")
        source = ROOT / "src" / module / f"{module}.c"
        if not source.is_file():
            fail(f"module source not materialized: {source.relative_to(ROOT)}")

    if "add_library(mbr_module_${module} INTERFACE)" not in text:
        fail("architecture facade modules must remain INTERFACE targets")


def check_forbidden_scope() -> None:
    paths = [ROOT / "CMakeLists.txt", ROOT / "cmake" / "MouseBridgeRemapperModules.cmake"] + source_files()
    combined = "\n".join(path.read_text(encoding="utf-8") for path in paths).lower()
    forbidden = (
        "classic_hid",
        "keyboard_transport",
        "hid_aggregator",
        "multi_mouse",
        "focused_mouse_id",
        "connected_mouse_count",
        "max_connected_mice",
        "pico_multicore",
        "multicore_launch_core1",
    )
    for token in forbidden:
        if token in combined:
            fail(f"forbidden/superseded product architecture token present: {token}")


def check_source_boundaries() -> None:
    textual_c_include = re.compile(r'^\s*#\s*include\s*[<"][^>"]+\.c[>"]', re.M)
    macro_interception = re.compile(r'^\s*#\s*define\s+(?:tud_|hids_|gap_|sm_|cyw43_|gpio_|spi_|flash_range_)', re.M)
    platform_token = re.compile(r'(?:pico/|hardware/|btstack|cyw43|tusb\.h|tinyusb|\btud_)', re.I)
    bt_token = re.compile(r'(?:btstack|cyw43|\bhci_|\bhids_|\bgap_|\bsm_)', re.I)
    bt_lifecycle_token = re.compile(
        r'(?:\bcyw43_arch_(?:init|deinit)\b|\bhci_power_control\b|\bhci_add_event_handler\b|'
        r'\bbtstack_memory_init\b|\bbtstack_run_loop_(?:init|execute)\b)',
        re.I,
    )
    tinyusb_token = re.compile(r'(?:tusb\.h|tinyusb|\btud_)', re.I)
    gpio_spi_token = re.compile(r'(?:hardware/(?:gpio|spi)|\bgpio_(?:init|put|get|set)|\bspi_(?:init|write|read))', re.I)
    flash_token = re.compile(r'(?:hardware/flash|\bflash_range_(?:erase|program))', re.I)
    app_raw = re.compile(r'(?:btstack|cyw43|tusb\.h|\btud_|hardware/|\bgpio_|\bspi_|\bflash_range_)', re.I)
    reenumeration = re.compile(r'\btud_(?:disconnect|connect)\s*\(', re.I)

    for path in source_files():
        text = path.read_text(encoding="utf-8")
        rel = path.relative_to(ROOT)
        module = module_for(path)

        if textual_c_include.search(text):
            fail(f"textual .c include prohibited: {rel}")
        if macro_interception.search(text):
            fail(f"transport/HAL macro interception prohibited: {rel}")
        if module in PURE_MODULES and platform_token.search(text):
            fail(f"host-pure module leaks platform/transport dependency: {rel}")
        if bt_token.search(text) and module not in BT_ALLOWED:
            fail(f"Bluetooth runtime primitive leaked outside adapter/runtime boundary: {rel}")
        if bt_lifecycle_token.search(text) and module != "bt_runtime":
            fail(f"CYW43/BTstack lifecycle ownership leaked outside bt_runtime: {rel}")
        if tinyusb_token.search(text) and module not in TINYUSB_ALLOWED:
            fail(f"TinyUSB ownership leaked outside usb_hid: {rel}")
        if gpio_spi_token.search(text) and module not in GPIO_SPI_ALLOWED:
            fail(f"GPIO/SPI ownership leaked outside hat/renderer: {rel}")
        if flash_token.search(text) and module not in FLASH_ALLOWED:
            fail(f"raw flash ownership leaked outside product_storage: {rel}")
        if module == "app" and app_raw.search(text):
            fail(f"app contains raw transport/HAL primitive: {rel}")
        if reenumeration.search(text):
            fail(f"USB re-enumeration path is prohibited: {rel}")


def check_single_authoritative_slot() -> None:
    header = (ROOT / "include" / "mbr" / "mouse_session" / "mouse_session.h").read_text(encoding="utf-8")
    required = (
        "mbr_authoritative_mouse_slot_t",
        "bool occupied;",
        "bool ready;",
        "mbr_mouse_session_id_t session_id;",
        "mbr_mouse_session_ready_count",
    )
    for token in required:
        if token not in header:
            fail(f"single-authoritative-slot contract missing: {token}")
    if re.search(r'mbr_mouse_session_id_t\s+\w+\s*\[', header):
        fail("authoritative Mouse session storage must not be an array")

    candidate = (ROOT / "include" / "mbr" / "pairing_coordinator" / "pairing_coordinator.h").read_text(encoding="utf-8")
    for token in ("mbr_pairing_candidate_t", "replacement_ready"):
        if token not in candidate:
            fail(f"non-authoritative Pair New candidate scaffold missing: {token}")


def check_production_scaffold() -> None:
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    required = (
        'PICO_BOARD STREQUAL "pico2_w"',
        "pico_enable_stdio_usb(mouse_bridge_remapper 0)",
        "pico_enable_stdio_uart(mouse_bridge_remapper 0)",
        "pico_add_extra_outputs(mouse_bridge_remapper)",
        "add_executable(mouse_bridge_remapper src/app/main.c)",
    )
    for token in required:
        if token not in cmake:
            fail(f"production scaffold guard missing: {token}")

    combined = cmake + "\n" + "\n".join(path.read_text(encoding="utf-8") for path in source_files())
    prohibited = {
        "production USB stdio": r"pico_enable_stdio_usb\s*\(\s*mouse_bridge_remapper\s+1\s*\)",
        "production UART stdio": r"pico_enable_stdio_uart\s*\(\s*mouse_bridge_remapper\s+1\s*\)",
        "diagnostic CDC runtime": r"\btud_cdc_",
        "forced USB disconnect": r"\btud_disconnect\s*\(",
        "forced USB reconnect": r"\btud_connect\s*\(",
    }
    for label, pattern in prohibited.items():
        if re.search(pattern, combined, re.I):
            fail(f"production scaffold prohibition violated: {label}")

    executable_names = re.findall(r"add_executable\s*\(\s*([A-Za-z0-9_.-]+)", cmake)
    firmware_names = [name for name in executable_names if not name.startswith("mbr_test_")]
    allowed = {"mouse_bridge_remapper", "mbr_renderer_hat_qualification", "mbr_usb_hid_qualification"}
    unexpected = [name for name in firmware_names if name not in allowed]
    if unexpected:
        fail(f"unexpected firmware executable targets: {unexpected}")
    if "mouse_bridge_remapper" not in firmware_names:
        fail("production firmware executable target is missing")


def check_toolchain_lock() -> None:
    values: dict[str, str] = {}
    for raw in (ROOT / "ci" / "toolchain.env").read_text(encoding="utf-8").splitlines():
        raw = raw.strip()
        if not raw or raw.startswith("#"):
            continue
        key, value = raw.split("=", 1)
        values[key] = value
    expected = {
        "CI_RUNNER": "ubuntu-24.04",
        "PICO_SDK_VERSION": "2.2.0",
        "ARM_GCC_PACKAGE_VERSION": "15:13.2.rel1-2",
        "ARM_GCC_UPSTREAM_VERSION": "13.2.Rel1",
    }
    if values != expected:
        fail(f"toolchain lock differs from MBR-01 baseline: {values}")



def check_usb_contract() -> None:
    header = (ROOT / "include" / "mbr" / "usb_hid" / "usb_hid.h").read_text(encoding="utf-8")
    required_header = (
        "#define MBR_USB_HID_VID UINT16_C(0xcafe)",
        "#define MBR_USB_HID_PID UINT16_C(0x4011)",
        "#define MBR_USB_HID_BCD_DEVICE UINT16_C(0x0100)",
        "#define MBR_USB_HID_INTERFACE_COUNT 2u",
        "#define MBR_USB_HID_MOUSE_INTERFACE 0u",
        "#define MBR_USB_HID_KEYBOARD_INTERFACE 1u",
        '#define MBR_USB_HID_MANUFACTURER "tiagooliveirajs"',
        '#define MBR_USB_HID_PRODUCT "Mouse Bridge Remapper"',
        "#define MBR_USB_HID_KEY_ESCAPE 0x29u",
    )
    for token in required_header:
        if token not in header:
            fail(f"USB identity/report contract missing: {token}")

    config = (ROOT / "include" / "tusb_config.h").read_text(encoding="utf-8")
    for token in (
        "#define CFG_TUD_HID 2",
        "#define CFG_TUD_CDC 0",
        "#define CFG_TUD_MSC 0",
        "#define CFG_TUD_MIDI 0",
        "#define CFG_TUD_VENDOR 0",
    ):
        if token not in config:
            fail(f"TinyUSB forbidden-interface guard missing: {token}")

    descriptors = (ROOT / "src" / "usb_hid" / "usb_descriptors.c").read_text(encoding="utf-8")
    for token in (
        "TUD_HID_REPORT_DESC_MOUSE()",
        "TUD_HID_REPORT_DESC_KEYBOARD()",
        "HID_ITF_PROTOCOL_MOUSE",
        "HID_ITF_PROTOCOL_KEYBOARD",
        "EPNUM_MOUSE 0x81u",
        "EPNUM_KEYBOARD 0x82u",
        ".iSerialNumber = 0x00",
        "MBR_USB_HID_PID",
        "MBR_USB_HID_MANUFACTURER",
        "MBR_USB_HID_PRODUCT",
    ):
        if token not in descriptors:
            fail(f"USB descriptor contract missing: {token}")

    descriptor_callbacks = {
        "tud_descriptor_device_cb",
        "tud_descriptor_configuration_cb",
        "tud_descriptor_string_cb",
        "tud_hid_descriptor_report_cb",
    }
    for callback in descriptor_callbacks:
        matches = 0
        for path in source_files():
            text = path.read_text(encoding="utf-8")
            matches += len(re.findall(rf"\b{re.escape(callback)}\s*\(", text))
        if matches != 1:
            fail(f"USB descriptor callback must have exactly one owner: {callback}")

    combined = "\n".join(
        path.read_text(encoding="utf-8")
        for path in (ROOT / "src" / "usb_hid").glob("*.c")
    )
    if re.search(r"\btud_(?:disconnect|connect)\s*\(", combined):
        fail("USB HID module must not force re-enumeration")


def main() -> int:
    check_module_graph()
    check_forbidden_scope()
    check_source_boundaries()
    check_single_authoritative_slot()
    check_production_scaffold()
    check_usb_contract()
    check_toolchain_lock()
    print("MBR-01 architecture contract: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
