# Product contract

## Purpose

Mouse Bridge Remapper is a standalone Raspberry Pi Pico 2 W appliance that pairs Bluetooth mice, forwards one connected mouse to a host over USB and remaps mouse buttons locally through the Waveshare Pico-LCD-1.3 HAT.

The host must not require companion software for normal operation.

## Product device type

The only Bluetooth product device type is **Mouse**.

The product does not offer Bluetooth Keyboard or Bluetooth Composite pairing flows.

A mouse-capable BLE HOGP peer is accepted only after its HID capabilities are classified as compatible with the Mouse role. Transport-specific report layouts never become application-domain types.

## Saved mice and live connection

The product may persist multiple saved mice.

The live runtime capacity is exactly:

```text
0 or 1 connected mouse
```

A second mouse may not become ready while another mouse remains connected.

The connected mouse, if present, is the sole source of live Mouse input and the sole target of HOME remapper actions.

## HOME resolution

HOME is determined by saved state and live connection state:

```text
no saved mice
  -> searching-first

saved mice exist + connected mouse exists
  -> home-connected

saved mice exist + no connected mouse
  -> home-searching and automatically start bounded saved-device search
```

If that bounded saved-device search expires without a successful connection, HOME becomes `home-retry` / `DEVICE NOT FOUND`.

This same rule applies after startup, after returning to HOME, and after the connected mouse disconnects or is powered off.

## Search policy

### No saved mouse

Search for the first valid mouse continues until one is accepted.

If more than one unsaved mouse is waiting to pair, the first valid candidate wins and the search stops.

### Saved mice

Whenever HOME is entered with saved mice and no live connection, perform a bounded saved-device search.

The first saved mouse that successfully reaches ready state wins and the search stops.

If none is found before timeout, publish `DEVICE NOT FOUND`.

### Pair New

Pair New is a manual request for one unsaved mouse and a replacement of any current live connection.

If a mouse is connected when Pair New begins:

1. stop accepting new input from its session;
2. release held Mouse/Escape state;
3. disconnect it cleanly;
4. retain its saved product record and bond;
5. start Pair New discovery.

The first valid unsaved candidate that completes acceptance becomes the sole connected mouse and Pair New stops.

If Pair New fails or is canceled, previously saved records remain intact. The old mouse remains saved but disconnected. Returning to HOME with no connection starts the ordinary saved-device search automatically.

## Saved Devices

Saved and connected are distinct states.

Each saved mouse has its own page. At most one page can show `STATUS: CONNECTED` and a cyan mouse name because only one live mouse exists.

## Profiles

Every saved mouse has a confirmed profile kind:

- Passthrough;
- Default/Standard Remap;
- Escape Remap;
- Custom Remap.

A newly saved mouse starts in Passthrough unless a later documented rule changes this.

Movement, vertical wheel and horizontal pan are never remapped by these button profiles.

The currently connected mouse's confirmed profile is the one used by HOME and remapper actions.

## Escape exception

Escape Remap and Custom target Escape are retained.

The product therefore exposes enough USB HID Keyboard capability to emit standard Escape from a mouse-remap source. This is output-only infrastructure and does not create a Bluetooth Keyboard role.

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

Transient live session state is never persisted as if it were a valid connection after reboot.

## Logitech behavior

Logitech-specific HID++ behavior is automatic and capability-driven. It is never exposed as a profile or transport selection.

When supported and needed, it may preserve true Forward down/hold/up behavior under remapping. Unsupported mice must fail safe to normal HID behavior.

## USB identity

USB identity is firmware-owned and fixed from boot. Bluetooth connections, pairing, profile changes, lock/unlock and reconnect do not trigger forced USB re-enumeration.

The exact project-specific VID/PID/manufacturer/product strings remain an explicit product decision. The descriptor shape must contain Mouse output and the minimal Keyboard output capability required for synthetic Escape, with no diagnostic CDC interface.

## Debug policy

Production firmware must not depend on diagnostic USB CDC, debug-only USB identities, UART logs for normal acceptance, debug-only LCD screens or parallel user-facing debug firmware personalities.

Host tests, CI compiler logs and internal development instrumentation are allowed when they do not change the production USB/UI contract.
