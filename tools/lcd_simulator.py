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
import tkinter as tk
from pathlib import Path
from tkinter import messagebox

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BACKEND = ROOT / "build-host" / "mbr_lcd_simulator"

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
    def __init__(self, root: tk.Tk, backend: Path, zoom: int) -> None:
        self.root = root
        self.backend = backend
        self.zoom = tk.IntVar(value=zoom)
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

        self._build_ui()
        self._refresh()
        self.root.protocol("WM_DELETE_WINDOW", self.close)
        self.root.bind("<KeyPress>", self._key_down)
        self.root.bind("<KeyRelease>", self._key_up)
        self.root.focus_force()

    def _build_ui(self) -> None:
        self.root.title("Mouse Bridge Remapper — Virtual LCD/HAT")
        self.root.configure(padx=12, pady=12)

        top = tk.Frame(self.root)
        top.pack(fill="x")
        tk.Label(top, text="Mouse ID", font=("TkDefaultFont", 12)).pack(side="left")
        tk.Entry(top, textvariable=self.mouse_id, width=5, font=("TkDefaultFont", 12)).pack(side="left", padx=(6, 12))
        tk.Label(top, text="Mouse name", font=("TkDefaultFont", 12)).pack(side="left")
        tk.Entry(top, textvariable=self.mouse_name, width=24, font=("TkDefaultFont", 12)).pack(side="left", padx=6)
        tk.Button(top, text="CONNECT", command=self._connect, font=("TkDefaultFont", 12, "bold")).pack(side="left", padx=4)
        tk.Button(top, text="DISCONNECT", command=lambda: self.command("disconnect"), font=("TkDefaultFont", 12)).pack(side="left", padx=4)

        tools = tk.Frame(self.root)
        tools.pack(fill="x", pady=(8, 8))
        for text, command in (
            ("+1 s", "tick 1000"),
            ("+8 s", "tick 8000"),
            ("+15 s", "tick 15000"),
            ("HOME", "home"),
            ("REBOOT", "reboot"),
            ("FACTORY RESET", "factory-reset"),
        ):
            tk.Button(tools, text=text, command=lambda c=command: self.command(c), font=("TkDefaultFont", 11)).pack(side="left", padx=3)
        tk.Label(tools, text=" Zoom:", font=("TkDefaultFont", 11)).pack(side="left", padx=(10, 2))
        for value in (1, 2, 3, 4):
            tk.Radiobutton(tools, text=f"{value}x", value=value, variable=self.zoom, command=self._refresh).pack(side="left")

        self.image_label = tk.Label(self.root, bd=2, relief="sunken")
        self.image_label.pack()

        pad = tk.Frame(self.root)
        pad.pack(pady=(10, 4))
        controls = [
            ("↑", "up", 0, 1), ("←", "left", 1, 0), ("JOY", "press", 1, 1), ("→", "right", 1, 2), ("↓", "down", 2, 1),
            ("A", "a", 0, 4), ("B", "b", 1, 4), ("X", "x", 0, 5), ("Y", "y", 1, 5),
        ]
        for label, control, row, col in controls:
            button = tk.Button(pad, text=label, width=7, height=2, font=("TkDefaultFont", 13, "bold"))
            button.grid(row=row, column=col, padx=3, pady=3)
            button.bind("<ButtonPress-1>", lambda _e, c=control: self.command(f"down {c}"))
            button.bind("<ButtonRelease-1>", lambda _e, c=control: self.command(f"up {c}"))

        tk.Label(
            self.root,
            text="Keyboard: arrows = joystick • Enter/Space = JOY PRESS • A/B/X/Y = HAT keys",
            font=("TkDefaultFont", 11),
        ).pack(pady=(6, 2))
        tk.Label(self.root, textvariable=self.status, anchor="w", justify="left", font=("TkFixedFont", 10)).pack(fill="x", pady=(4, 0))

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
        self._refresh()

    def _refresh(self) -> None:
        if not self.frame.exists():
            return
        image = tk.PhotoImage(file=str(self.frame))
        z = self.zoom.get()
        if z > 1:
            image = image.zoom(z, z)
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


def build_backend() -> None:
    subprocess.run(["cmake", "-S", str(ROOT), "-B", str(ROOT / "build-host"), "-DCMAKE_BUILD_TYPE=Debug"], check=True)
    subprocess.run(["cmake", "--build", str(ROOT / "build-host"), "--target", "mbr_lcd_simulator", "--parallel"], check=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--backend", type=Path, default=DEFAULT_BACKEND)
    parser.add_argument("--build", action="store_true", help="build the C backend before opening the window")
    parser.add_argument("--zoom", type=int, choices=(1, 2, 3, 4), default=3)
    args = parser.parse_args()

    if args.build:
        build_backend()
    backend = args.backend.resolve()
    if not backend.exists():
        print(f"Backend not found: {backend}", file=sys.stderr)
        print("Run: python3 tools/lcd_simulator.py --build", file=sys.stderr)
        return 2

    root = tk.Tk()
    try:
        Simulator(root, backend, args.zoom)
    except Exception as exc:
        root.destroy()
        print(f"Simulator startup failed: {exc}", file=sys.stderr)
        return 1
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
