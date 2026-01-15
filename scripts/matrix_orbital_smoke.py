#!/usr/bin/env python3
"""
Matrix Orbital (LCD Smartie) smoke sender for ArduLCDpp.

Sends the same early init sequence used by LCD Smartie's `matrix.dll` and then
exercises the MVP commands:
- clear screen (FE 'X')
- set cursor (FE 'G' x y) where x/y are 1-based
- custom characters (FE 4E slot + 8 bytes)
- brightness/backlight (FE 98 level, FE 'B' 0, FE 'F')

This is intentionally a "write-only" script: it can be used for visual checks
or alongside firmware debug logging.
"""

from __future__ import annotations

import argparse
import time

import serial

DEFAULT_BAUDRATE = 57600
MO_PREFIX = 0xFE


def mo_cmd(*bytes_: int) -> bytes:
    return bytes([MO_PREFIX, *[b & 0xFF for b in bytes_]])


def build_init_sequence() -> bytes:
    # From LCD Smartie (matrix.dll) initLCD:
    # FE 'T' (cursor blink off)
    # FE 'X' (clear screen)
    # FE 'K' (cursor off)
    # FE 'R' (auto scroll off)
    # FE 'D' (auto line wrap off)
    # FE 0x41 (auto transmit keys)
    # FE '`' (auto repeat off)
    return b"".join(
        (
            mo_cmd(ord("T")),
            mo_cmd(ord("X")),
            mo_cmd(ord("K")),
            mo_cmd(ord("R")),
            mo_cmd(ord("D")),
            mo_cmd(0x41),
            mo_cmd(ord("`")),
        )
    )


def build_custom_char(slot: int) -> bytes:
    # A visually distinct glyph (not easily confused with ASCII).
    # 5x8: box-with-dot.
    bitmap = [
        0b11111,
        0b10001,
        0b10101,
        0b10001,
        0b11111,
        0b00100,
        0b00000,
        0b00000,
    ]
    return mo_cmd(0x4E, slot & 0x07, *bitmap)

def send(ser: serial.Serial, payload: bytes, delay_s: float) -> None:
    ser.write(payload)
    ser.flush()
    if delay_s > 0:
        time.sleep(delay_s)


def main() -> int:
    parser = argparse.ArgumentParser(description="Send Matrix Orbital MVP sequences to ArduLCDpp.")
    parser.add_argument("--port", required=True, help="Serial port (e.g. COM6 or /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=DEFAULT_BAUDRATE, help=f"Baud rate (default: {DEFAULT_BAUDRATE})")
    parser.add_argument("--delay", type=float, default=3.0, help="Seconds to wait after opening the port (auto-reset).")
    parser.add_argument("--hold", type=float, default=2.0, help="Seconds to keep the port open after sending.")
    parser.add_argument(
        "--chunk-delay-ms",
        type=float,
        default=25.0,
        help="Delay between command/data chunks to avoid dropping tail bytes on slow targets (default: 25).",
    )
    parser.add_argument(
        "--after-init-ms",
        type=float,
        default=100.0,
        help="Extra delay after the initial matrix.dll-style init sequence (default: 100).",
    )
    args = parser.parse_args()

    chunk_delay_s = max(0.0, args.chunk_delay_ms) / 1000.0
    after_init_s = max(0.0, args.after_init_ms) / 1000.0

    with serial.Serial(args.port, args.baud, timeout=0.1) as ser:
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        time.sleep(args.delay)

        # Phase 1: init and clear.
        send(ser, build_init_sequence(), chunk_delay_s)
        time.sleep(after_init_s)

        # Phase 2: first line.
        send(ser, mo_cmd(ord("G"), 1, 1), chunk_delay_s)
        send(ser, b"MO OK (20x4)", chunk_delay_s)

        # Phase 3: custom char definition + render.
        send(ser, mo_cmd(ord("G"), 1, 2), chunk_delay_s)
        send(ser, b"Custom: ", chunk_delay_s)
        send(ser, build_custom_char(0), chunk_delay_s)
        # Render ASCII 'x' then 4 copies of CGRAM slot 0 (for comparison).
        send(ser, b"x", chunk_delay_s)
        send(ser, bytes([0, 0, 0, 0]), chunk_delay_s)

        # Phase 4: backlight label + brightness.
        send(ser, mo_cmd(ord("G"), 1, 3), chunk_delay_s)
        send(ser, b"Backlight:OK", chunk_delay_s)
        send(ser, mo_cmd(0x98, 255), chunk_delay_s)  # full brightness

        time.sleep(args.hold)

        # Blink backlight off/on at the end so it's obvious the command landed.
        send(ser, mo_cmd(ord("F")), 0.0)
        time.sleep(0.3)
        send(ser, mo_cmd(ord("B"), 0), 0.0)
        time.sleep(0.3)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
