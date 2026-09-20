# Host virtual LCD/HAT simulator

The experimental MBR-08 branch includes a desktop simulator for the 240×240 Waveshare display and HAT controls.

It does **not** emulate RP2350, CYW43, BTstack, TinyUSB or physical flash. Its purpose is to exercise the exact host-pure application state machine, UI projector and RGB565 renderer used by the firmware, making layout and navigation bugs visible before flashing hardware.

## What is shared with firmware

The C backend calls the production code directly:

`MbrApp -> mbr_app_view() -> mbr_project() -> mbr_render()`

The framebuffer is the same 240×240 RGB565 framebuffer sent to the ST7789 backend on the Pico 2 W. The simulator keeps that logical framebuffer intact and models LCD illumination as a separate global backlight gain applied only when converting the virtual panel to desktop RGB. Lock therefore switches the **effective** backlight to 0% without destroying or recoloring the underlying framebuffer.

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

The default scale is 3×, so the 240×240 LCD appears as a 720×720 virtual panel. Use `--zoom 1`, `2`, `3` or `4` to choose another initial scale. The desktop shell uses a dark theme.

If the backend is already built:

```bash
python3 tools/lcd_simulator.py
```


## Virtual backlight model

The RGB565 values are not changed in firmware or in the logical simulator framebuffer. Instead, the desktop viewer applies one global gain to the converted RGB channels:

- `100%`: digital RGB565 reference;
- `200%`, `300%`, `400%`: progressively brighter inspection gain applied equally to red, green and blue, with each channel saturated at 255;
- `0%`: backlight off;
- application Lock: always forces **effective 0%**, regardless of the selected gain.

The default viewer gain is **300%** because the frozen dark-magenta value `0x0801` converts to approximately RGB(8, 0, 8) at the 100% digital reference and can look black on a normal monitor. At 300%, that same framebuffer value is displayed approximately as RGB(24, 0, 24), while every other non-black color is affected by the same global gain.

This is deliberately an **inspection model**, not a photometrically calibrated ST7789/backlight model. Values above 100% emulate increased apparent panel luminance on a desktop monitor so very dark palette choices can be judged. They do not claim a measured relationship to PWM duty cycle, nits or the physical Waveshare module.

The selected gain is preserved while Lock is active. For example, with the viewer set to 300%, pressing the product's Lock key makes effective backlight 0%; unlocking restores effective backlight to 300%.

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
- FACTORY RESET: restart with no saved product state;
- LCD backlight slider: 0–400% in 10% steps;
- quick backlight presets: 100%, 200%, 300% and 400%.

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
9. Set backlight to 300% or 400% and compare black, dark magenta, gray, white and cyan; every non-black color must respond to the same global gain.
10. Set a nonzero gain, activate Lock with the screen's KEY Y control, and verify effective backlight becomes 0% while application state remains alive; unlock and verify the previous gain is restored.

For raw layout inspection without navigation, the backend protocol also supports `screen <canonical-screen-id>`.

## Backend protocol

The GUI launches `build-host/mbr_lcd_simulator FRAME.ppm`. Commands are line-oriented:

```text
tap up
down a
up a
connect 1 LOGITECH LIFT
disconnect
brightness 300
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
