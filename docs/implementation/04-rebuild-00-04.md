# Clean rebuild: MBR-00 through MBR-04

## Authorization and baseline

User explicitly authorized sequential implementation of MBR-00..04 with physical gates deferred until delivery. This overrides the historical stop-for-physical-acceptance rule, not the requirement to report physical evidence honestly. No physical PASS is claimed.

Product clean base: `2f305838f7ab10a2d6affd63e5b359cd9a464543` (documentation only). Earlier MBR branches are historical, not implementation inputs. Branch: `mbr/rebuild-00-through-04`.

G06 reference: `blu2usb@7eee024ad4ee726c5a85ffa2f32b9f47187878af`. Inspected renderer.c, st7789_pico.c, renderer.h, hat.c, hat_pico.c, USB descriptors/adapter, toolchain.env and CI. Reuse is limited to the font, physical panel initialization, geometry/palette, pin mapping and USB design evidence. No old application, Bluetooth or keyboard product code is imported.

## MBR-00 contract revalidation

Current product documentation and planner frozen contract are authoritative. All 30 canonical screens, amended connected HOME and search Help, one authoritative Mouse, 8/8/15-second search policy, confirmation-only mutations and fixed CAFE:4011 USB are retained. Explicit didactic token columns override the approximate spacing in Markdown examples. Hints remain LIGHT_GRAY (RGB565 C618), with WHITE only for actual press feedback.

## Pre-implementation sequence

1. MBR-01: CMake host/Pico composition, domain and bounded module interfaces, dependency/ownership checks. Verify host bootstrap and target scaffold.
2. MBR-02: pure interaction, projector and application orchestration; exact literal and transition tests, stale generation rejection, Help/lock ownership, model-only asynchronous operation requests. No fake production pairing or persistence success.
3. MBR-03: physical renderer, active-low 20ms HAT debounce, explicit pixel geometry, image/command tests; separate qualification target for all screens.
4. MBR-04: sole TinyUSB owner, exact descriptors and canonical report builders; report/backpressure tests and RP2350 UF2 validation. Separate, explicit USB fixture mode for physical testing.

Expected modules: domain, application, interaction, ui_projector, renderer, hat, usb_hid, mouse_session, pairing_coordinator, mouse_registry, output_state and declared future profiles/remap/storage/BLE/runtime/HID++ boundaries.

Risks/regressions: hint/body overlap, token emphasis, white/cyan priority, release actions crossing screen transitions, swallowed USB releases, unintended serial interface. Tests target those boundaries. Real BLE, persistent profiles, HID++ and real registry integration remain MBR-05 onward.

Recovery: BOOTSEL can restore any earlier UF2. These gates do not write flash product state or BT credentials. Qualification uses RAM-only fixtures and is a separate executable; production never fabricates a connected device.

Verification: host CMake/CTest with assertions enabled, architecture/source guards, rendered screen geometry/color/literal tests, pinned Pico SDK 2.2.0 and ARM GCC 13.2.Rel1 target builds, UF2 magic/family/address checks and SHA-256. Physical scenarios and final evidence follow after implementation.

## Implementation completion (physical acceptance deferred)

- Planner source inspected: `repo-planner@1b4d8ad1398343659c4bc6cde0bcd2bc6b207867`. Its 2026-09-20 reset supersedes historical ACCEPTED headings.
- MBR-00: contract revalidated; no unresolved decision blocks the requested scope.
- MBR-01: clean host/Pico CMake, module ownership interfaces and guards, sole live session and separate non-authoritative candidate.
- MBR-02: 30 canonical literal screens generated from the current product reference; release interaction with screen epochs; one HOME resolver; 8/8/15-second model searches; stale event filtering; async request/confirmation model; exact Help and didactic controls; dynamic names/status/profile/draft.
- MBR-03: adapted G06 font, panel initialization, pin map and geometry; independent projector and framebuffer rasterizer; C618 hint text; explicit didactic token emphasis; 1ms cooperative input sampling, 20ms debounce and bounded queue. LCD flushes two rows per loop to keep USB/input service responsive. The 30 rendered screens were inspected, with Help body KEY B explicitly prevented from being mistaken for a footer hint.
- MBR-04: exact fixed descriptors, five-byte Mouse reports and eight-byte Escape-only reports, sole TinyUSB adapter, independent endpoint backpressure, bounded report queue, overflow/all-up handling, no forced USB re-enumeration, no CDC/UART stdio. Report-protocol interfaces do not claim Boot Protocol support.
- Separate qualification executable: 30-screen gallery, RAM-only interactive fixtures and explicit HAT-to-USB fixture tests. Production has no fixture source linked and remains in real first-search UI without claiming BLE connection.

### Automated evidence

CMake host Debug build with `-Wall -Wextra -Werror`, assertions explicitly enabled for tests. Seven CTest suites pass: core, UX, renderer, USB, qualification, architecture and canonical screen regeneration. Coverage includes stale session/transaction rejection, press-release epoch ownership, ordinary/instructional Lock, Help consuming actions, new-only eligibility, preservation of old session until confirmed handoff, confirmed mutations, didactic columns, hint/body palette, every visible row fitting 240x240, debounce/overflow, descriptors interpreted for report widths, delayed endpoint behavior, Escape release and report-queue overflow safety.

Pico toolchain: Ubuntu 24.04; ARM GCC `13.2.1 20231009` (package `15:13.2.rel1-2`); Pico SDK `2.2.0` at `a1438dff1d38bd9c65dbd693f0e5db4b9ae91779`; TinyUSB `86ad6e56c1700e85f1c5678607a762cfe3aa2f47`; Picotool 2.2.0; CMake 3.28.3. Target `pico2_w`, `rp2350-arm-s`, Release. Local toolchain packages were extracted into a private toolchain directory; commands are the README commands with that toolchain on PATH. Newlib 4.4.0.20231231-2.

Both target builds succeed. UF2 verification checks magic, 256-byte payload, unique sequence/address coverage, Pico SDK's absolute-family ignore extension, RP2350 ARM-S family and Pico 2 W flash range. Artifact hashes/sizes are recorded in the delivery manifest. Physical evidence: **NOT RUN / DEFERRED BY USER** for MBR-03 and MBR-04.

### Limits and next gate

No real BLE transport, radio initialization, persistent product-state writes, credential deletion, HID++ or runtime button remapping is claimed. Their ownership facades explicitly return unavailable. Handoff/apply/removal behavior before those gates is a host model with confirmation boundaries, exercised physically only via labeled RAM fixtures. The report queue is output-only; authoritative source ownership/remap accumulation is MBR-05/06 work.

The exact next implementation gate is MBR-05; this request stops at MBR-04. Continue only with its scope and the applicable physical validation policy. The current exception allows delivery before physical testing, not an invented physical PASS.
