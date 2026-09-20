# Static browser simulator

The experimental MBR-08 branch includes a browser version of the virtual LCD/HAT simulator under `web/`.

Runtime files are `index.html`, `simulator.css`, `simulator-core.js` and `simulator.js`. There is no runtime backend, package manager, framework, WebAssembly, REST endpoint or network dependency. Classic local script files are used so the simulator can be opened directly from the filesystem:

```bash
xdg-open web/index.html
```

A conventional local HTTP origin also works:

```bash
python3 -m http.server 8080 -d web
```

Then open `http://127.0.0.1:8080/`.

## Implemented simulation

The browser core mirrors the host-pure product behavior needed for UI/flow inspection: all 30 canonical screens, first-Mouse search, HOME resolver, 8-second saved search, 15-second Pair New, saved/new filtering, replacement, Saved Devices/removal, Passthrough/Standard/Escape/Custom UI state, Custom draft/apply, Help ownership, Lock, HOME name policy and deterministic clock controls.

The LCD renderer ports the same 5×7 firmware glyph data, doubled-pixel geometry, row placement, RGB565 palette, black body background and dark-magenta didactic/footer background into a 240×240 canvas framebuffer.

## Backlight and scale

The logical framebuffer is kept separate from illumination. Global gain is 0–1000%; Lock forces effective 0% while preserving the selected gain. Black remains black and RGB channels saturate at 255.

The canvas remains internally 240×240. Visual scale is CSS-only: 75%=180, 100%=240, 125%=300, 150%=360, 200%=480 and 300%=720 pixels.

## Persistence

When browser storage is available, virtual saved mice/profiles/Custom state are mirrored to `localStorage`. REBOOT restores that simulated product state and FACTORY RESET clears it. The simulator still runs in-memory if storage is unavailable.

## Verification boundary

`web/test_simulator_core.js` runs in Node during host CI. It verifies 30 screens, HOME title policy, requested scale dimensions, 1000% backlight, Lock blackout, first connection, HOME, Pair New/replacement, disconnect/saved timeout and framebuffer sizes.

This is an inspection simulator. It does not emulate real CYW43/BTstack/security/HIDS, TinyUSB, flash interruption, GPIO or ST7789 optical characteristics, and it does not change physical acceptance.
