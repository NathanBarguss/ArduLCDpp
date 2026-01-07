#!/usr/bin/env python3
"""
T5 custom character parity probe.

Programs CGRAM slots 0..7 via los-panel bytes and then renders bytes 0x00..0x07.
Use this on LCD-only, OLED-only, and Dual builds to visually confirm parity.

Note: HD44780 clear (`FE 01`) is slow. If the host sends DDRAM writes immediately
after a clear/home, the firmware may still be blocked inside the LCD clear delay
loop and UART bytes can be dropped. Use `--after-clear-ms` (and optionally
`--slot-delay-ms`) when running on slower targets like ATmega168.
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
    parser.add_argument(
        "--backlight",
        type=int,
        default=None,
        help="Optional los-panel backlight/brightness byte to send first (0-255). Useful for OLED visibility.",
    )
    parser.add_argument(
        "--chunk-delay-ms",
        type=float,
        default=0.0,
        help="Delay between CGRAM slot writes and DDRAM phases (helps slow displays and avoids RX overruns).",
    )
    parser.add_argument(
        "--slot-delay-ms",
        type=float,
        default=0.0,
        help="Delay between each CGRAM slot programming chunk (10 bytes).",
    )
    parser.add_argument(
        "--after-clear-ms",
        type=float,
        default=5.0,
        help="Delay after sending clear/home before writing DDRAM (HD44780 clear can block long enough to drop following UART bytes).",
    )
    args = parser.parse_args()

    seq = build_sequence()
    print(
        f"Sending {len(seq)} bytes to {args.port} @ {args.baud} "
        f"(chunk_delay_ms={args.chunk_delay_ms}, slot_delay_ms={args.slot_delay_ms})..."
    )

    with serial.Serial(args.port, args.baud, timeout=1) as ser:
        time.sleep(args.delay)

        if args.backlight is not None:
            level = max(0, min(255, int(args.backlight)))
            ser.write(bytes([0xFD, level]))
            ser.flush()
            time.sleep(0.05)

        if args.chunk_delay_ms <= 0 and args.slot_delay_ms <= 0:
            ser.write(seq)
            ser.flush()
        else:
            # Send in phases to give the firmware time to program CGRAM without
            # dropping bytes (especially on slower builds / smaller MCUs).
            chunk_delay_s = args.chunk_delay_ms / 1000.0
            slot_delay_s = args.slot_delay_ms / 1000.0

            # Phase 1: program CGRAM slots 0..7 (each is 1 command + 8 data bytes).
            # The first 80 bytes are CGRAM programming:
            # 8 slots * (ESC + setCGRAM + 8 data bytes) = 8 * (2 + 8) = 80.
            cgram_len = 80
            if slot_delay_s > 0:
                for slot in range(8):
                    start = slot * 10
                    end = start + 10
                    ser.write(seq[start:end])
                    ser.flush()
                    time.sleep(slot_delay_s)
                time.sleep(chunk_delay_s)
            else:
                ser.write(seq[:cgram_len])
                ser.flush()
                time.sleep(chunk_delay_s)

            # Phase 2: clear/home, then wait so the HD44780 clear instruction
            # doesn't block long enough to overflow the UART RX buffer.
            clear_home = seq[cgram_len : cgram_len + 4]
            ser.write(clear_home)
            ser.flush()
            time.sleep(max(0.0, args.after_clear_ms) / 1000.0)

            # Phase 3: row-0 render (set addr + 8 glyph bytes = 10)
            row0 = seq[cgram_len + 4 : cgram_len + 14]
            ser.write(row0)
            ser.flush()
            time.sleep(chunk_delay_s)

            # Phase 4: row-1 render (set addr + 8 glyph bytes = 10)
            row1 = seq[cgram_len + 14 :]
            ser.write(row1)
            ser.flush()

        time.sleep(args.hold)

    print("Done. Expect 8 distinct glyphs for slots 0..7 on rows 0 and 1.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
