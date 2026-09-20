# UI and renderer contract

Status: **FROZEN BY MBR-00; AMENDED BY MBR-02 AND HOME-SEARCHING HELP DECISIONS 2026-09-20; MBR-03 IMPLEMENTED**.

## Hardware baseline

Target UI hardware is Waveshare Pico-LCD-1.3 with ST7789 240x240.

Inherited physical baseline:

- 5x7 glyph source, scale 2;
- glyph box 10x14 px;
- horizontal advance 11 px;
- title origin x=7, y=8;
- standard first body y=39;
- standard body advance 26 px;
- final standard hint y=214, bottom anchored;
- dark-magenta hint region starts 11 px above first visible hint;
- didactic screens use the exact frozen token columns in the canonical screen reference.

## Physical renderer implementation

The Pico implementation uses SPI1 with the Waveshare Pico-LCD-1.3 pin contract:

- SCK GPIO10;
- MOSI GPIO11;
- CS GPIO9;
- DC GPIO8;
- RST GPIO12;
- backlight GPIO13.

The ST7789 initialization and RGB565 write path are adapted from the accepted BLU2USB G03/G06 implementation. The renderer owns these GPIO/SPI primitives; `ui_projector` remains host-pure.

The HAT input map is active-low:

- JOY UP GPIO2;
- JOY PRESS GPIO3;
- JOY LEFT GPIO16;
- JOY RIGHT GPIO20;
- JOY DOWN GPIO18;
- KEY A GPIO15;
- KEY B GPIO17;
- KEY X GPIO19;
- KEY Y GPIO21.

Physical HAT scanning is 1 ms with 20 ms debounce and a bounded 32-event queue. Application actions remain release-triggered.

The glyph source is the frozen 5x7 font scaled 2x. The MBR renderer additionally provides comma and parenthesis glyphs because current canonical Help text contains those characters.

A non-production qualification firmware target can cycle all 30 canonical screens without adding a diagnostic interface to the production firmware.

## Semantic width and dynamic names

Canonical rows are at most 21 characters.

Mouse names are stored in full normalized form within schema limits. Projection displays the first **21 renderer-supported characters** with no ellipsis or scrolling. Empty/unusable name displays `UNKNOWN MOUSE`.

## Screen authority

`docs/manual/06-screen-reference.md` is the canonical literal screen/control inventory. Renderer code cannot silently correct or reinterpret its text.

Canonical visible profile vocabulary is `STANDARD`, not historical `DEFAULT`.

Obvious historical transcription mistakes are normalized in the screen reference, including `T0`→`TO`, `FORWARED`→`FORWARD`, and `kEY`→`KEY`.

## Separation

`ui_projector` builds semantic rows/tokens/colors from application state; renderer draws them. Renderer never decides connection truth, search purpose, timeout, profile commit success, candidate identity or removal success.

## HOME projection

```text
no saved mice -> searching-first
saved + authoritative Mouse -> home-connected
saved + no authoritative Mouse -> home-searching
saved search expires/cancels -> home-retry
```

`home-connected` projects the current authoritative Mouse name as the dynamic title. Its four visible options are, in order, Pair New, current remap summary, Saved Devices and Learn the Keys. Pair New is the direct visible connected-Mouse entry into the existing 15-second replacement transaction. The current Mouse remains authoritative while that transaction qualifies a candidate.

When HOME is visible, a live disconnect reprojects immediately to `home-searching` and application starts saved search. When another page owns presentation, connection truth updates but navigation is not forcibly stolen; the HOME resolver applies on next HOME access.

## Pair New projection

Pair New may coexist with the existing authoritative Mouse during discovery. The old Mouse remains the product's live Mouse until replacement handoff.

The UI must therefore not project a false disconnect merely because Pair New began. If a candidate becomes replacement-ready and handoff commits, the old session is released/disconnected before the new one is projected as authoritative.

If Pair New times out/cancels before handoff, the original Mouse remains live. If the user manually unplugs it, the Pair New/help page may remain visible, but future HOME access resolves to saved search.

## Frozen Help

All Help literals, including the amended `home-searching-help`, are rendered exactly from the canonical screen reference. The current `home-searching-help` is:

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

## Frozen Pair New Help

`help-pair-new` and `help-retry-pair-new` literal rows are exactly those in `docs/manual/06-screen-reference.md`, including the instruction to unplug the currently connected Mouse and Back until `SEARCHING` appears when the user wants a saved device.

## Colors

- title: magenta;
- static/body: off-white yellow;
- ordinary option/action: light gray;
- selected/visibly pressed: white;
- connected/current/applied/success: cyan.

White selection/press overrides cyan and cyan returns when selection leaves a still-current item.

On Saved Devices, only the sole connected Mouse name is cyan. Disconnected pages use ordinary body color and show `STATUS: DISCONNECTED`.

## Interaction projection

Actions execute on release. Help owns all HAT input while visible and `ANY KEY: BACK` consumes the event.

No hidden controls are inherited. A control exists only if the canonical per-screen map defines it.

Instructional `first-mouse-connected` / `learn-the-keys` special B/X/Y semantics and fully didactic `searching-first` semantics are frozen in the screen reference.

`JOY LEFT: GO TO HOME` on `escape-active` is intentional and invokes HOME resolver directly.

## Didactic columns

Frozen 1-based columns:

- `JOY UP`: 8;
- three `JOY`: 3/10/17;
- `LEFT`/`PRESS`/`RIGHT`: 3/9/16;
- `JOY DOWN`: 7;
- first-search `KEY A`/`KEY X`: 2/16;
- first-search `KEY B`/`KEY Y`: 2/16;
- instructional right-side `KEY A/B/X`: 16;
- `LOCK SCREEN`: 1;
- `AND UNLOCK`: 2;
- `OPEN HOME -> KEY Y`: 3.

Tests target the intended token, never the first coincidental matching character in a row.

## Removed states

No Pair Keyboard, Pair Composite, Keyboard/Composite saved pages, Other Devices page, multi-connected count state or live-Mouse focus selector exists.
