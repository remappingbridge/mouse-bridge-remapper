# MBR-03 — Waveshare renderer and HAT

Status: **IMPLEMENTED / PHYSICAL ACCEPTANCE PENDING**.

## Scope

MBR-03 adapts the accepted BLU2USB G03/G06 physical renderer/HAT behavior to the frozen Mouse Bridge Remapper semantic UI.

Implemented:

- ST7789 240x240 RGB565 renderer;
- 5x7 glyphs scaled 2x into 10x14 cells;
- 11 px horizontal advance and 21-character semantic width;
- title origin x=7/y=8;
- standard body y=39 with 26 px advance;
- bottom-anchored hints at y=214 with dark-magenta hint region;
- didactic screens using the accepted dark-magenta field and frozen token columns;
- title/body/option/white/cyan semantic colors;
- comma/parenthesis glyphs needed by current canonical Help screens;
- Waveshare Pico-LCD-1.3 ST7789 GPIO/SPI ownership;
- active-low HAT input map with 1 ms scan, 20 ms debounce and bounded event queue;
- release-triggered integration through the accepted MBR-02 interaction engine;
- production firmware with no serial/CDC diagnostics;
- isolated qualification firmware for physical renderer/HAT acceptance.

## Waveshare pin map

| Function | GPIO |
|---|---:|
| LCD DC | 8 |
| LCD CS | 9 |
| LCD SCK | 10 |
| LCD MOSI | 11 |
| LCD RST | 12 |
| LCD backlight | 13 |
| JOY UP | 2 |
| JOY PRESS | 3 |
| JOY LEFT | 16 |
| JOY RIGHT | 20 |
| JOY DOWN | 18 |
| KEY A | 15 |
| KEY B | 17 |
| KEY X | 19 |
| KEY Y | 21 |

## Updated screen literal

The current `home-searching-help` is:

```text
HOME SEARCHING HELP
THE MATCHING ATTEMPT
TOOK PLACE ONLY FOR
DEVICES ALREADY SAVED
IN THE PREFERENCES,
BUT NOT FOR DEVICES
THAT WERE NOT SAVED.

ANY KEY: BACK
```

## Automated evidence

Candidate branch: `mbr/mbr-03-renderer-hat`.

Candidate SHA: `4d4c2b9852e0db7390d75fac943294437db0d762`.

CI run `35481175036` passed both jobs. Host tests include the renderer/HAT geometry/color/map contract.

Actions artifact: `mbr-03-pico2w-renderer-hat-uf2`, ID `10594974918`.

Production UF2:
- 77312 bytes;
- SHA-256 `382afe84c787a34e4c7bde6e70327de03e449fb62f7d06c758bb435202459cc2`.

Qualification UF2:
- 78336 bytes;
- SHA-256 `9e9151d674b166ee546e4e82099243d75f01f44bdaa63bcb81ea063b3fb9dd41`.

The qualification UF2 is the physical-test artifact. It cycles all 30 canonical screen projections and is not a production feature.

## Acceptance boundary

MBR-03 remains open until the operator reports physical PASS for the numbered scenarios in `repo-planner/mouse-bridge-remapper/executions/mbr-03/candidate.md`. A green build does not constitute physical acceptance.