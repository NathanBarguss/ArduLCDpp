#!/usr/bin/env python3
"""
T5 custom character parity probe.

Programs CGRAM slots 0..7 via los-panel bytes and then renders bytes 0x00..0x07.
Use this on LCD-only, OLED-only, and Dual builds to visually confirm parity.
"""

from __future__ import annotations

import argparse
import time

import serial


ESC = 0xFE


def cgram_set_addr(addr: int) -> bytes:
    # HD44780: Set CGRAM address = 0x40 | (addr & 0x3F)
    return bytes([ESC, 0x40 | (addr & 0x3F)])


def ddram_set_addr(addr: int) -> bytes:
    # HD44780: Set DDRAM address = 0x80 | (addr & 0x7F)
    return bytes([ESC, 0x80 | (addr & 0x7F)])


def build_sequence() -> bytes:
    # 8 glyphs x 8 rows. Each row uses only the low 5 bits (0..31).
    glyphs = [
        # 0: diagonal down-right
        [0b10000, 0b01000, 0b00100, 0b00010, 0b00001, 0b00000, 0b00000, 0b00000],
        # 1: diagonal down-left
        [0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b00000, 0b00000, 0b00000],
        # 2: checker-ish
        [0b10101, 0b01010, 0b10101, 0b01010, 0b10101, 0b01010, 0b10101, 0b01010],
        # 3: hollow box
        [0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11111],
        # 4: filled box
        [0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111],
        # 5: left bar
        [0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000],
        # 6: right bar
        [0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001],
        # 7: "X"
        [0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b00000, 0b00000, 0b00000],
    ]

    seq = bytearray()

    # Program slots 0..7.
    for slot in range(8):
        # Each slot is 8 bytes starting at CGRAM address slot<<3.
        seq += cgram_set_addr(slot << 3)
        seq += bytes([row & 0x1F for row in glyphs[slot]])

    # Switch back to DDRAM and render the glyph indices.
    # Clear + home for predictable output, then write 0..7 on row 0 and row 1.
    seq += bytes([ESC, 0x01])  # clear
    seq += bytes([ESC, 0x02])  # home

    seq += ddram_set_addr(0x00)  # row 0 col 0
    seq += bytes(range(8))       # glyphs 0..7

    seq += ddram_set_addr(0x40)  # row 1 col 0
    seq += bytes(range(8))       # glyphs 0..7 again

    return bytes(seq)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", required=True, help="Serial port (e.g., COM6)")
    parser.add_argument("--baud", type=int, default=57600)
    parser.add_argument("--delay", type=float, default=3.0, help="Seconds to wait after opening port (auto-reset)")
    parser.add_argument("--hold", type=float, default=2.0, help="Seconds to keep the port open after sending")
    args = parser.parse_args()

    seq = build_sequence()
    print(f"Sending {len(seq)} bytes to {args.port} @ {args.baud}...")

    with serial.Serial(args.port, args.baud, timeout=1) as ser:
        time.sleep(args.delay)
        ser.write(seq)
        ser.flush()
        time.sleep(args.hold)

    print("Done. Expect 8 distinct glyphs for slots 0..7 on rows 0 and 1.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

