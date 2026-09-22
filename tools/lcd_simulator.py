#!/usr/bin/env python3
"""Interactive 240x240 virtual LCD/HAT for Mouse Bridge Remapper.

The C backend uses the same MbrApp, projector and RGB565 renderer as firmware.
Tkinter only displays the generated framebuffer and forwards virtual HAT events.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
import tempfile
from pathlib import Path

try:
    import tkinter as tk
    from tkinter import messagebox
except ModuleNotFoundError:
    tk = None
    messagebox = None

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BACKEND = ROOT / "build-host" / "mbr_lcd_simulator"

SCALE_FACTORS = {
    75: (3, 4),
    100: (1, 1),
    125: (5, 4),
    150: (3, 2),
    200: (2, 1),
    300: (3, 1),
}

BG = "#181818"
PANEL = "#242424"
CONTROL = "#303030"
CONTROL_ACTIVE = "#444444"
FG = "#f2f2f2"
MUTED = "#b8b8b8"
ACCENT = "#7ad7ff"
ENTRY_BG = "#111111"

EXPECTED_SCALED_PIXELS = {
    75: 180,
    100: 240,
    125: 300,
    150: 360,
    200: 480,
    300: 720,
}

KEYS = {
    "Up": "up",
    "Down": "down",
    "Left": "left",
    "Right": "right",
    "Return": "press",
    "space": "press",
    "a": "a",
    "A": "a",
    "b": "b",
    "B": "b",
    "x": "x",
    "X": "x",
    "y": "y",
    "Y": "y",
}


class Simulator:
    def __init__(self, root: tk.Tk, backend: Path, scale_percent: int) -> None:
        self.root = root
        self.backend = backend
        self.scale_percent = tk.IntVar(value=scale_percent)
        self.brightness = tk.IntVar(value=300)
        self.brightness_text = tk.StringVar(value="Backlight: 300% • effective: 300%")
        self.mouse_id = tk.StringVar(value="1")
        self.mouse_name = tk.StringVar(value="LOGITECH LIFT")
        self.status = tk.StringVar(value="Starting simulator...")
        self.temp = tempfile.TemporaryDirectory(prefix="mbr-lcd-")
        self.frame = Path(self.temp.name) / "frame.ppm"
        self.process = subprocess.Popen(
            [str(backend), str(self.frame)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )
        if self.process.stdin is None or self.process.stdout is None:
            raise RuntimeError("failed to create simulator pipes")

        first = self.process.stdout.readline().rstrip()
        if not first:
            error = self.process.stderr.read() if self.process.stderr else ""
            raise RuntimeError(f"simulator backend exited early: {error}")
        self.status.set(first)
        self._sync_backlight_status(first)

        self._build_ui()
        self._refresh()
        self.root.protocol("WM_DELETE_WINDOW", self.close)
        self.root.bind("<KeyPress>", self._key_down)
        self.root.bind("<KeyRelease>", self._key_up)
        self.root.focus_force()

    def _button(self, parent: tk.Widget, text: str, command=None, *, bold: bool = False) -> tk.Button:
        return tk.Button(
            parent,
            text=text,
            command=command,
            bg=CONTROL,
            fg=FG,
            activebackground=CONTROL_ACTIVE,
            activeforeground=FG,
            disabledforeground=MUTED,
            highlightthickness=1,
            highlightbackground="#505050",
            relief="flat",
            bd=0,
            padx=9,
            pady=7,
            font=("TkDefaultFont", 11, "bold" if bold else "normal"),
        )

    def _build_ui(self) -> None:
        self.root.title("Mouse Bridge Remapper — Virtual LCD/HAT")
        self.root.configure(bg=BG, padx=12, pady=12)

        top = tk.Frame(self.root, bg=BG)
        top.pack(fill="x")
        tk.Label(top, text="Mouse ID", bg=BG, fg=FG, font=("TkDefaultFont", 12)).pack(side="left")
        tk.Entry(
            top,
            textvariable=self.mouse_id,
            width=5,
            bg=ENTRY_BG,
            fg=FG,
            insertbackground=FG,
            highlightbackground="#555555",
            highlightcolor=ACCENT,
            relief="flat",
            font=("TkDefaultFont", 12),
        ).pack(side="left", padx=(6, 12), ipady=5)
        tk.Label(top, text="Mouse name", bg=BG, fg=FG, font=("TkDefaultFont", 12)).pack(side="left")
        tk.Entry(
            top,
            textvariable=self.mouse_name,
            width=24,
            bg=ENTRY_BG,
            fg=FG,
            insertbackground=FG,
            highlightbackground="#555555",
            highlightcolor=ACCENT,
            relief="flat",
            font=("TkDefaultFont", 12),
        ).pack(side="left", padx=6, ipady=5)
        self._button(top, "CONNECT", self._connect, bold=True).pack(side="left", padx=4)
        self._button(top, "DISCONNECT", lambda: self.command("disconnect")).pack(side="left", padx=4)

        tools = tk.Frame(self.root, bg=BG)
        tools.pack(fill="x", pady=(8, 5))
        for text, command in (
            ("+1 s", "tick 1000"),
            ("+8 s", "tick 8000"),
            ("+15 s", "tick 15000"),
            ("HOME", "home"),
            ("REBOOT", "reboot"),
            ("FACTORY RESET", "factory-reset"),
        ):
            self._button(tools, text, lambda c=command: self.command(c)).pack(side="left", padx=3)
        tk.Label(tools, text=" Scale:", bg=BG, fg=FG, font=("TkDefaultFont", 11)).pack(side="left", padx=(10, 2))
        for value in SCALE_FACTORS:
            tk.Radiobutton(
                tools,
                text=f"{value}%",
                value=value,
                variable=self.scale_percent,
                command=self._refresh,
                bg=BG,
                fg=FG,
                activebackground=BG,
                activeforeground=FG,
                selectcolor=CONTROL,
                highlightthickness=0,
            ).pack(side="left")

        light = tk.Frame(self.root, bg=PANEL, padx=10, pady=8)
        light.pack(fill="x", pady=(0, 8))
        tk.Label(
            light,
            text="LCD backlight gain",
            bg=PANEL,
            fg=FG,
            font=("TkDefaultFont", 11, "bold"),
        ).pack(side="left")
        self.backlight_scale = tk.Scale(
            light,
            from_=0,
            to=1000,
            resolution=10,
            orient="horizontal",
            variable=self.brightness,
            command=self._brightness_changed,
            showvalue=False,
            length=420,
            bg=PANEL,
            fg=FG,
            troughcolor=ENTRY_BG,
            activebackground=ACCENT,
            highlightthickness=0,
            bd=0,
            sliderrelief="flat",
        )
        self.backlight_scale.pack(side="left", padx=10)
        for value in (100, 300, 500, 750, 1000):
            self._button(light, f"{value}%", lambda v=value: self._set_brightness(v)).pack(side="left", padx=2)
        tk.Label(
            light,
            textvariable=self.brightness_text,
            bg=PANEL,
            fg=ACCENT,
            font=("TkFixedFont", 10, "bold"),
        ).pack(side="left", padx=(10, 0))

        self.image_label = tk.Label(
            self.root,
            bd=2,
            relief="sunken",
            bg="#000000",
            highlightthickness=1,
            highlightbackground="#555555",
        )
        self.image_label.pack()

        pad = tk.Frame(self.root, bg=BG)
        pad.pack(pady=(10, 4))
        controls = [
            ("↑", "up", 0, 1), ("←", "left", 1, 0), ("JOY", "press", 1, 1), ("→", "right", 1, 2), ("↓", "down", 2, 1),
            ("A", "a", 0, 4), ("B", "b", 1, 4), ("X", "x", 0, 5), ("Y", "y", 1, 5),
        ]
        for label, control, row, col in controls:
            button = self._button(pad, label, bold=True)
            button.configure(width=6, height=2, font=("TkDefaultFont", 13, "bold"))
            button.grid(row=row, column=col, padx=3, pady=3)
            button.bind("<ButtonPress-1>", lambda _e, c=control: self.command(f"down {c}"))
            button.bind("<ButtonRelease-1>", lambda _e, c=control: self.command(f"up {c}"))

        tk.Label(
            self.root,
            text="Keyboard: arrows = joystick • Enter/Space = JOY PRESS • A/B/X/Y = HAT keys",
            bg=BG,
            fg=MUTED,
            font=("TkDefaultFont", 11),
        ).pack(pady=(6, 2))
        tk.Label(
            self.root,
            text="100% = RGB565 digital reference • >100% = virtual backlight gain for color inspection • Lock forces effective 0%",
            bg=BG,
            fg=MUTED,
            font=("TkDefaultFont", 10),
        ).pack(pady=(0, 2))
        tk.Label(
            self.root,
            textvariable=self.status,
            anchor="w",
            justify="left",
            bg=PANEL,
            fg=FG,
            padx=8,
            pady=6,
            font=("TkFixedFont", 10),
        ).pack(fill="x", pady=(4, 0))

    def _set_brightness(self, value: int) -> None:
        self.brightness.set(value)
        self.command(f"brightness {value}")

    def _brightness_changed(self, value: str) -> None:
        gain = max(0, min(1000, int(float(value))))
        self.command(f"brightness {gain}")

    def _sync_backlight_status(self, line: str) -> None:
        values = {}
        for token in line.split("\t"):
            if "=" in token:
                key, value = token.split("=", 1)
                values[key] = value
        gain = values.get("backlight", str(self.brightness.get()))
        effective = values.get("effective_backlight", gain)
        locked = values.get("locked", "0") == "1"
        suffix = " • LOCKED / backlight OFF" if locked else ""
        self.brightness_text.set(f"Backlight: {gain}% • effective: {effective}%{suffix}")

    def _connect(self) -> None:
        try:
            mouse_id = int(self.mouse_id.get())
            if mouse_id <= 0:
                raise ValueError
        except ValueError:
            messagebox.showerror("Invalid mouse ID", "Mouse ID must be a positive integer.")
            return
        name = self.mouse_name.get().strip()
        if not name:
            messagebox.showerror("Invalid mouse name", "Mouse name cannot be empty.")
            return
        self.command(f"connect {mouse_id} {name}")

    def command(self, command: str) -> None:
        if self.process.poll() is not None:
            self.status.set("Backend is no longer running.")
            return
        assert self.process.stdin is not None
        assert self.process.stdout is not None
        self.process.stdin.write(command + "\n")
        self.process.stdin.flush()
        line = self.process.stdout.readline().rstrip()
        if line:
            self.status.set(line)
            self._sync_backlight_status(line)
        self._refresh()

    def _refresh(self) -> None:
        if not self.frame.exists():
            return
        image = tk.PhotoImage(file=str(self.frame))
        scale = self.scale_percent.get()
        numerator, denominator = SCALE_FACTORS.get(scale, SCALE_FACTORS[300])
        if numerator != 1:
            image = image.zoom(numerator, numerator)
        if denominator != 1:
            image = image.subsample(denominator, denominator)
        self._photo = image
        self.image_label.configure(image=image)

    def _key_down(self, event: tk.Event) -> None:
        control = KEYS.get(event.keysym) or KEYS.get(event.char)
        if control:
            self.command(f"down {control}")

    def _key_up(self, event: tk.Event) -> None:
        control = KEYS.get(event.keysym) or KEYS.get(event.char)
        if control:
            self.command(f"up {control}")

    def close(self) -> None:
        try:
            if self.process.poll() is None and self.process.stdin is not None:
                self.process.stdin.write("quit\n")
                self.process.stdin.flush()
                self.process.wait(timeout=1)
        except Exception:
            self.process.kill()
        self.temp.cleanup()
        self.root.destroy()


def self_test() -> int:
    for percent, expected in EXPECTED_SCALED_PIXELS.items():
        numerator, denominator = SCALE_FACTORS[percent]
        actual = 240 * numerator // denominator
        if actual != expected:
            raise AssertionError(f"{percent}% produced {actual}px, expected {expected}px")
    if set(SCALE_FACTORS) != set(EXPECTED_SCALED_PIXELS):
        raise AssertionError("scale presets and expected dimensions differ")
    print("lcd simulator GUI self-test PASS")
    return 0


def build_backend() -> None:
    subprocess.run(["cmake", "-S", str(ROOT), "-B", str(ROOT / "build-host"), "-DCMAKE_BUILD_TYPE=Debug"], check=True)
    subprocess.run(["cmake", "--build", str(ROOT / "build-host"), "--target", "mbr_lcd_simulator", "--parallel"], check=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--backend", type=Path, default=DEFAULT_BACKEND)
    parser.add_argument("--build", action="store_true", help="build the C backend before opening the window")
    parser.add_argument("--scale", type=int, choices=tuple(SCALE_FACTORS), default=300, help="initial LCD display scale in percent")
    parser.add_argument("--self-test", action="store_true", help="validate scale presets without opening Tk")
    args = parser.parse_args()

    if args.self_test:
        return self_test()

    if tk is None or messagebox is None:
        print("Tkinter is required for the GUI. On Debian: sudo apt install python3-tk", file=sys.stderr)
        return 2

    if args.build:
        build_backend()
    backend = args.backend.resolve()
    if not backend.exists():
        print(f"Backend not found: {backend}", file=sys.stderr)
        print("Run: python3 tools/lcd_simulator.py --build", file=sys.stderr)
        return 2

    root = tk.Tk()
    try:
        Simulator(root, backend, args.scale)
    except Exception as exc:
        root.destroy()
        print(f"Simulator startup failed: {exc}", file=sys.stderr)
        return 1
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
