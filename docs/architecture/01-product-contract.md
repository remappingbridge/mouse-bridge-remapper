# Product contract

## Purpose

Mouse Bridge Remapper is a standalone Raspberry Pi Pico 2 W appliance that pairs Bluetooth mice, forwards their input to a host over USB and remaps mouse buttons locally through the Waveshare Pico-LCD-1.3 HAT.

The host must not require companion software for normal operation.

## Product device type

The only Bluetooth product device type is **Mouse**.

The product does not offer Bluetooth Keyboard or Bluetooth Composite pairing flows.

A mouse-capable BLE HOGP peer is accepted only after its HID capabilities are classified as compatible with the Mouse role. Transport-specific report layouts never become application-domain types.

## Multiple connected mice

The runtime supports a set of connected mouse sessions, not one global active mouse.

Product/UI count range is 0 through 999. If exactly one mouse is connected, HOME displays that mouse's name. If two or more are connected, HOME displays `<N> DEVICES CONNECTED`.

The firmware must prevent the displayed count from exceeding 999. Physical support for a specific number of concurrent BLE sessions must be established separately by evidence.

## Search policy

### No saved mouse

Search for the first valid mouse continues until one is accepted.

If more than one unsaved mouse is waiting to pair, the first valid candidate wins and the search stops.

### Saved mice at startup

If saved mice exist and no mouse is connected, perform a bounded saved-device search/reconnect attempt.

As soon as one saved mouse becomes ready, that automatic search stops. Do not continue automatically to connect additional mice.

### Additional mice

Additional mice are connected only by explicit user action through Pair New or another explicitly documented manual connection action.

Each Pair New action accepts at most one new mouse. If several unsaved mice are waiting, the first valid candidate wins and the search stops.

## Saved devices

Saved and connected are distinct states.

A saved mouse persists across power loss. A saved mouse may be disconnected. Multiple saved mice may be connected simultaneously.

Saved Devices shows one mouse per page. A connected mouse's name on line 2 is cyan.

## Profiles

Every saved mouse has a confirmed profile kind:

- Passthrough;
- Default/Standard Remap;
- Escape Remap;
- Custom Remap.

A newly saved mouse starts in Passthrough unless a later documented rule changes this.

Movement, vertical wheel and horizontal pan are never remapped by these button profiles.

## Escape exception

Escape Remap and Custom target Escape are retained.

The product therefore exposes enough USB HID Keyboard capability to emit standard Escape from a mouse-remap source. This is output-only product infrastructure; it does not create a Bluetooth keyboard role.

## Custom template

The inherited contract has one persistent global Custom template containing five source-button mappings. Saved mice using Custom reference that template rather than owning private mapping tables unless this documentation is explicitly changed.

## Persistence

At minimum, product persistence must safely preserve:

- saved mouse identities and display names;
- each saved mouse's confirmed profile kind;
- global Custom template;
- Custom draft and dirty/unapplied state where inherited behavior requires it;
- vendor/capability metadata needed for safe reconnect/remap behavior.

Bluetooth credentials are not the same record as product configuration.

## Logitech behavior

Logitech-specific HID++ behavior is automatic and capability-driven. It is never exposed as a profile or transport selection.

When supported and needed, it may preserve true Forward down/hold/up behavior under remapping. Unsupported mice must fail safe to normal HID behavior.

## USB identity

USB identity is firmware-owned and fixed from boot. Bluetooth connections, pairing, profile changes, lock/unlock and reconnect do not trigger forced USB re-enumeration.

The exact project-specific VID/PID/manufacturer/product strings remain an explicit product decision. The descriptor shape must contain Mouse output and the minimal Keyboard output capability required for synthetic Escape, with no diagnostic CDC interface.

## Debug policy

Production firmware must not depend on:

- diagnostic USB CDC;
- debug-only USB product identities;
- UART logs for normal acceptance;
- debug-only LCD screens;
- parallel user-facing debug firmware personalities.

Host tests, CI compiler logs and internal development instrumentation are allowed when they do not change the production USB/UI contract.
