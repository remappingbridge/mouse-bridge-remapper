# UI and renderer contract

## Hardware baseline

Target UI hardware is the Waveshare Pico-LCD-1.3 HAT with ST7789 240x240 display.

Starting renderer geometry inherits the physically accepted BLU2USB layout lessons:

- 5x7 glyph source, scale 2;
- glyph box 10x14 pixels;
- horizontal character advance 11 px;
- title origin x=7, y=8;
- standard first body y=39;
- standard body advance 26 px;
- final standard hint y=214, bottom anchored;
- dark-magenta hint region begins 11 px above the first visible hint;
- special didactic layouts may use their explicitly documented positions.

The new screen reference supersedes historical Keyboard/Composite pages and supplies new horizontal coordinates for the Learn/search didactic layouts.

## Semantic width

Canonical text rows use at most 21 characters.

The connected-count format is intentionally bounded by this rule:

- `2 DEVICES CONNECTED`
- ...
- `999 DEVICES CONNECTED`

`999 DEVICES CONNECTED` is 21 characters and is therefore the maximum count representation.

## Screen model separation

`ui_projector` produces semantic rows/tokens/colors/actions from application state. `renderer` draws them.

Renderer must not decide:

- whether a mouse is connected;
- which search transaction is active;
- whether a profile commit succeeded;
- whether removal succeeded;
- whether a candidate is saved/new.

It consumes already-decided screen state.

## Async projection

A connection/disconnection/profile/removal event updates the visible screen immediately when relevant. The LCD must not wait for another HAT press to notice a Bluetooth state change.

This preserves an important G06 correction: runtime events, not recent input activity, are the source of connection truth.

## Colors

Semantic palette:

- title: magenta;
- static/body text: off-white yellow;
- ordinary options/actions: light gray;
- selected or visibly pressed actionable text: white;
- connected/current/applied/success state: cyan.

White selection/press has priority over cyan. When selection leaves a still-current item, cyan returns.

### Saved Devices connected name

On a Saved Devices page, the mouse name on line 2 is cyan exactly when that specific mouse has a current ready session.

Several pages may independently have cyan names if several mice are connected.

## Dynamic fields

Dynamic values include:

- one connected mouse's name;
- multi-connected count text;
- Saved Devices pagination;
- per-mouse status;
- per-mouse confirmed profile;
- Custom draft mappings.

Planning annotations such as `(nome do mouse)`, `(paginação)` and `(customizável)` are never rendered.

## Interaction

Actions execute on release.

Visible pressed labels may become white while held. The interaction engine emits semantic commands only after the complete physical press/release interaction.

Help pages own all controls and use `ANY KEY: BACK`.

The unlock interaction is consumed and cannot also activate another screen action.

## Didactic coordinates

For the new `SEARCHING FIRST MOUSE` / Learn-style geometry:

- `JOY UP` starts at 1-based column 8;
- three `JOY` tokens start columns 3, 10, 17;
- `LEFT`, `PRESS`, `RIGHT` start columns 3, 9, 16;
- `JOY DOWN` starts column 7;
- `KEY A`/`KEY X` on the first-search screen start columns 2/16;
- `KEY B`/`KEY Y` on the first-search screen start columns 2/16;
- right-side `KEY A/B/X` on Learn/first-connected starts column 16;
- `LOCK SCREEN` starts column 1;
- `AND UNLOCK` starts column 2;
- `OPEN HOME -> KEY Y` starts column 3.

Host layout tests must assert token coordinates rather than infer them from visual spacing in Markdown.

## Dynamic-name policy

The complete long-name truncation/ellipsis policy is still open. Renderer must not invent one. Storage keeps the complete normalized available name; projection will apply the documented presentation policy once frozen.

## Removed screens

Renderer registry must not contain product pages for Pair Keyboard, Pair Composite, Keyboard Saved, Composite Saved or old Other Devices status/help flows.

Synthetic Escape requires no Bluetooth Keyboard page.
