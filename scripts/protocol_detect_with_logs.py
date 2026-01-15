#!/usr/bin/env python3
"""
Exercise protocol auto-detection while capturing firmware debug logs.

Requires a serial-debug firmware build (e.g., `nano168_dual_serial`) so the
device prints `proto.selected=...` once the RX stream is idle.
"""

from __future__ import annotations

import argparse
import sys
import threading
import time
from typing import List

try:
    import serial  # type: ignore
except ImportError as exc:  # pragma: no cover
    sys.stderr.write("pyserial is required: pip install pyserial\n")
    raise

BAUDRATE = 57600
MO_PREFIX = 0xFE


def mo_cmd(*bytes_: int) -> bytes:
    return bytes([MO_PREFIX, *[b & 0xFF for b in bytes_]])


def build_mo_init() -> bytes:
    # LCD Smartie matrix.dll initLCD sequence.
    return b"".join(
        (
            mo_cmd(ord("T")),
            mo_cmd(ord("X")),
            mo_cmd(ord("K")),
            mo_cmd(ord("R")),
            mo_cmd(ord("D")),
            mo_cmd(0x41),
            mo_cmd(ord("`")),
            mo_cmd(ord("G"), 1, 1),
            b"MO",
        )
    )


def build_los_panel_burst() -> bytes:
    # Standard clear+home+text.
    return bytes([0xFE, 0x01, 0xFE, 0x02]) + b"LOS-PANEL"


def reader_thread(ser: serial.Serial, stop: threading.Event, sink: List[str]) -> None:
    while not stop.is_set():
        line = ser.readline()
        if not line:
            time.sleep(0.01)
            continue
        text = line.decode("utf-8", errors="replace").rstrip()
        print(text)
        sink.append(text)


def run_once(port: str, payload: bytes, delay_before_send: float, capture_s: float) -> List[str]:
    logs: List[str] = []
    with serial.Serial(port, BAUDRATE, timeout=0.05) as ser:
        ser.reset_input_buffer()
        time.sleep(delay_before_send)
        stop = threading.Event()
        t = threading.Thread(target=reader_thread, args=(ser, stop, logs), daemon=True)
        t.start()
        try:
            ser.write(payload)
            ser.flush()
            time.sleep(capture_s)
        finally:
            stop.set()
            t.join(timeout=1.0)
    return logs


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", required=True)
    ap.add_argument("--delay", type=float, default=3.0)
    ap.add_argument("--capture", type=float, default=3.0)
    args = ap.parse_args()

    print("[MO] sending init...")
    logs_mo = run_once(args.port, build_mo_init(), args.delay, args.capture)

    print("[LOS] re-opening and sending burst...")
    logs_los = run_once(args.port, build_los_panel_burst(), args.delay, args.capture)

    ok = True
    if not any("proto.selected=matrix-orbital" in line for line in logs_mo):
        sys.stderr.write("[MO] WARNING: did not observe proto.selected=matrix-orbital\n")
        ok = False
    if not any("proto.selected=los-panel" in line for line in logs_los):
        sys.stderr.write("[LOS] WARNING: did not observe proto.selected=los-panel\n")
        ok = False
    return 0 if ok else 2


if __name__ == "__main__":
    raise SystemExit(main())

