# Host virtual LCD/HAT simulator

The experimental MBR-08 branch includes a desktop simulator for the 240×240 Waveshare display and HAT controls.

It does **not** emulate RP2350, CYW43, BTstack, TinyUSB or physical flash. Its purpose is to exercise the exact host-pure application state machine, UI projector and RGB565 renderer used by the firmware, making layout and navigation bugs visible before flashing hardware.

## What is shared with firmware

The C backend calls the production code directly:

`MbrApp -> mbr_app_view() -> mbr_project() -> mbr_render()`

The framebuffer is the same 240×240 RGB565 framebuffer sent to the ST7789 backend on the Pico 2 W. When the application is locked the virtual backlight is represented by a black LCD.

## Build and run

Requirements:

- CMake and a C compiler;
- Python 3 with Tkinter;
- no Pillow, SDL, Qt or other GUI dependency.

From the repository root:

```bash
git switch experimental/mbr08-integrated-recovery-20260920
python3 tools/lcd_simulator.py --build
```

The default scale is 3×, so the 240×240 LCD appears as a 720×720 virtual panel. Use `--zoom 1`, `2`, `3` or `4` to choose another initial scale.

If the backend is already built:

```bash
python3 tools/lcd_simulator.py
```

## Controls

Keyboard:

- arrows: joystick directions;
- Enter or Space: joystick press;
- A/B/X/Y: physical HAT keys.

The on-screen buttons also send separate press/release events, so the same white pressed-token feedback can be inspected.

Toolbar:

- CONNECT: inject a BLE Mouse READY event into the current search transaction;
- DISCONNECT: disconnect the authoritative virtual mouse;
- +1 s / +8 s / +15 s: advance deterministic application time;
- HOME: run the HOME resolver;
- REBOOT: restart the application while preserving simulated product state;
- FACTORY RESET: restart with no saved product state.

CONNECT intentionally obeys the real search eligibility rules. For example, a saved Mouse cannot win a Pair New search, and a new Mouse cannot win SEARCH_SAVED.

## Suggested layout-flow checks

1. Start at SEARCHING FIRST MOUSE and hold/release every virtual HAT control to inspect didactic highlighting.
2. CONNECT ID 1 as `LOGITECH LIFT`; inspect FIRST MOUSE CONNECTED, then Y to HOME.
3. Navigate Pair New, Saved Devices, Learn and every profile using only virtual HAT controls.
4. In Pair New, CONNECT ID 2 with another name; verify HOME and registry projection after handoff.
5. DISCONNECT on HOME and advance +8 s to inspect SEARCHING SAVED MOUSE -> DEVICE NOT FOUND.
6. REBOOT with saved devices and inject a saved ID to test saved reconnect.
7. Open Help during a search, advance time, and verify that returning does not reset the search deadline.
8. Use long names and names containing MOUSE to inspect HOME/Saved Devices truncation and suffix rules.
9. Test Lock; the virtual LCD becomes black while the application remains alive.

For raw layout inspection without navigation, the backend protocol also supports `screen <canonical-screen-id>`.

## Backend protocol

The GUI launches `build-host/mbr_lcd_simulator FRAME.ppm`. Commands are line-oriented:

```text
tap up
down a
up a
connect 1 LOGITECH LIFT
disconnect
tick 8000
home
reboot
factory-reset
screen saved-devices
status
quit
```

After each command the backend rewrites the PPM framebuffer and prints one tab-separated state line.

## Verification boundary

CI builds the simulator backend and executes `mbr_lcd_simulator --smoke`. This verifies the simulator can traverse first pair, HOME, Pair New, replacement, disconnect, timeout and reboot/persistence using production state/rendering code.

A PASS here is not physical acceptance. BLE radio behavior, real pairing/security/HIDS, ST7789 SPI timing, GPIO/HAT electrical behavior, USB enumeration and flash interruption still require Pico 2 W tests.
