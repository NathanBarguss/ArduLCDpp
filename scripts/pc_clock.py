#!/usr/bin/env python3
"""
PC-side "clock" demo for ArduLCDpp (los-panel over serial).

Layout (20x4 default):
- Row 1: Date ("January 8th") left-aligned
- Rows 2-3: Big HH:MM (24h) centered using custom chars + block
- Row 4: Year right-aligned
- Colon flashes once per second
"""

from __future__ import annotations

import argparse
import datetime as dt
import time
from typing import Iterable, List, Optional, Sequence, Tuple

try:
    from zoneinfo import ZoneInfo  # py3.9+
except Exception:  # pragma: no cover
    ZoneInfo = None  # type: ignore[assignment]

import serial


ESC = 0xFE
BACKLIGHT = 0xFD
META_PREFIX = 0xFC
META_SET_STREAMING_MODE = 0x10
STREAMING_MODE_IMMEDIATE = 0
STREAMING_MODE_SAFE = 1


# Custom chars (CGRAM slots 0..7), from the user-provided reference.
CUST_CHARS: List[List[int]] = [
    [31, 31, 31, 0, 0, 0, 0, 0],  # 0: small top line
    [0, 0, 0, 0, 0, 31, 31, 31],  # 1: small bottom line
    [31, 0, 0, 0, 0, 0, 0, 31],  # 2: small lines top and bottom
    [0, 0, 0, 0, 0, 0, 0, 31],  # 3: thin bottom line
    [31, 31, 31, 31, 31, 31, 15, 7],  # 4: left bottom chamfer full
    [28, 30, 31, 31, 31, 31, 31, 31],  # 5: right top chamfer full
    [31, 31, 31, 31, 31, 31, 30, 28],  # 6: right bottom chamfer full
    [7, 15, 31, 31, 31, 31, 31, 31],  # 7: left top chamfer full
]


# Big 3x2 digits: 6 "cells" per digit, row-major (top-left..top-right, then bottom-left..bottom-right).
# 0..7 are CGRAM slots, 254 => space, 255 => full-block (0xFF).
BIG_NUMS: List[List[int]] = [
    [7, 0, 5, 4, 1, 6],  # 0
    [0, 5, 254, 1, 255, 1],  # 1
    [0, 2, 5, 7, 3, 1],  # 2
    [0, 2, 5, 1, 3, 6],  # 3
    [7, 3, 255, 254, 254, 255],  # 4
    [7, 2, 0, 1, 3, 6],  # 5
    [7, 2, 0, 4, 3, 6],  # 6
    [0, 0, 5, 254, 7, 254],  # 7
    [7, 2, 5, 4, 3, 6],  # 8
    [7, 2, 5, 1, 3, 6],  # 9
]


def cgram_set_addr(addr: int) -> bytes:
    return bytes([ESC, 0x40 | (addr & 0x3F)])


def ddram_set_addr(addr: int) -> bytes:
    return bytes([ESC, 0x80 | (addr & 0x7F)])


def ddram_addr_for(row: int, col: int, width: int, height: int) -> int:
    if height == 4:
        # Common HD44780 address maps.
        if width == 20:
            bases = (0x00, 0x40, 0x14, 0x54)
        else:  # 16x4 and many compatibles
            bases = (0x00, 0x40, 0x10, 0x50)
    elif height == 2:
        bases = (0x00, 0x40)
    else:
        bases = tuple(i * width for i in range(height))
    return (bases[row] + col) & 0x7F


def ordinal_suffix(day: int) -> str:
    if 11 <= (day % 100) <= 13:
        return "th"
    return {1: "st", 2: "nd", 3: "rd"}.get(day % 10, "th")


def format_date(now: dt.datetime) -> str:
    day = now.day
    return f"{now.strftime('%B')} {day}{ordinal_suffix(day)}"


def to_cell_byte(value: int) -> int:
    if 0 <= value <= 7:
        return value
    if value == 254:
        return 0x20  # space
    if value == 255:
        return 0xFF  # full block
    return 0x20


def big_digit_row_bytes(digit: int, row: int) -> bytes:
    cells = BIG_NUMS[digit]
    start = 0 if row == 0 else 3
    return bytes(to_cell_byte(v) for v in cells[start : start + 3])


def render_big_time(hhmm: str, colon_on: bool) -> Tuple[bytes, bytes]:
    d0, d1, d2, d3 = (int(ch) for ch in hhmm)

    # Colon uses two existing custom chars for a "two-dot" feel without consuming extra CGRAM slots.
    colon_top = bytes([0]) if colon_on else b" "
    colon_bottom = bytes([1]) if colon_on else b" "
    sep = b" "  # readability spacing between big glyph groups

    top = b"".join(
        (
            big_digit_row_bytes(d0, 0),
            sep,
            big_digit_row_bytes(d1, 0),
            sep,
            colon_top,
            sep,
            big_digit_row_bytes(d2, 0),
            sep,
            big_digit_row_bytes(d3, 0),
        )
    )
    bottom = b"".join(
        (
            big_digit_row_bytes(d0, 1),
            sep,
            big_digit_row_bytes(d1, 1),
            sep,
            colon_bottom,
            sep,
            big_digit_row_bytes(d2, 1),
            sep,
            big_digit_row_bytes(d3, 1),
        )
    )
    return top, bottom


def pad_or_trim(text: str, width: int) -> bytes:
    trimmed = text[:width]
    return trimmed.encode("ascii", errors="replace").ljust(width, b" ")


def program_custom_chars(ser: serial.Serial, slot_delay_ms: float) -> None:
    for slot, glyph in enumerate(CUST_CHARS):
        payload = bytearray()
        payload += cgram_set_addr(slot << 3)
        payload += bytes((row & 0x1F) for row in glyph)
        ser.write(payload)
        ser.flush()
        if slot_delay_ms > 0:
            time.sleep(slot_delay_ms / 1000.0)


def build_write_at(row: int, col: int, width: int, height: int, data: bytes) -> bytes:
    addr = ddram_addr_for(row, col, width, height)
    return ddram_set_addr(addr) + data


def parse_tz(name: Optional[str]) -> Optional[dt.tzinfo]:
    if not name:
        return None
    if ZoneInfo is None:
        raise SystemExit("Python 3.9+ required for --tz (zoneinfo).")
    try:
        return ZoneInfo(name)
    except Exception as exc:
        raise SystemExit(f"Unknown timezone '{name}': {exc}") from exc


def main() -> int:
    parser = argparse.ArgumentParser(description="ArduLCDpp PC clock demo (los-panel over serial).")
    parser.add_argument("--port", default="COM6", help="Serial port (default: COM6)")
    parser.add_argument("--baud", type=int, default=57600, help="Baud rate (default: 57600)")
    parser.add_argument("--width", type=int, default=20, help="Display columns (default: 20)")
    parser.add_argument("--height", type=int, default=4, help="Display rows (default: 4)")
    parser.add_argument("--delay", type=float, default=3.0, help="Seconds to wait after opening port (auto-reset).")
    parser.add_argument("--backlight", type=int, default=255, help="Backlight byte 0-255 (default: 255).")
    parser.add_argument("--streaming", choices=("safe", "immediate"), default=None, help="Optional dual-build mode hint.")
    parser.add_argument("--slot-delay-ms", type=float, default=15.0, help="Delay between CGRAM slot uploads.")
    parser.add_argument("--after-clear-ms", type=float, default=10.0, help="Delay after clear/home before DDRAM writes.")
    parser.add_argument("--tz", default=None, help="Optional timezone name (e.g., Europe/London).")
    args = parser.parse_args()

    width = int(args.width)
    height = int(args.height)
    if width <= 0 or height <= 0:
        raise SystemExit("--width/--height must be positive.")
    if height < 4:
        raise SystemExit("This demo expects a 4-row display (--height 4).")

    time_block_width = 17  # 4*(3-wide digit) + 1 colon + 4 single-char gaps
    time_start_col = max(0, (width - time_block_width) // 2)

    tz = parse_tz(args.tz)

    last_date: Optional[str] = None
    last_year: Optional[str] = None
    last_hhmm: Optional[str] = None
    last_colon: Optional[bool] = None

    with serial.Serial(args.port, args.baud, timeout=0.1) as ser:
        time.sleep(args.delay)

        # Ensure first byte clears the power-on banner promptly.
        init = bytearray()

        if args.streaming:
            mode = STREAMING_MODE_SAFE if args.streaming == "safe" else STREAMING_MODE_IMMEDIATE
            init += bytes([META_PREFIX, META_SET_STREAMING_MODE, mode])

        level = max(0, min(255, int(args.backlight)))
        init += bytes([BACKLIGHT, level])
        init += bytes([ESC, 0x01, ESC, 0x02])  # clear + home
        ser.write(init)
        ser.flush()
        time.sleep(max(0.0, args.after_clear_ms) / 1000.0)

        program_custom_chars(ser, args.slot_delay_ms)
        ser.write(ddram_set_addr(0x00))
        ser.flush()

        while True:
            now = dt.datetime.now(tz=tz)
            now_floor = now.replace(microsecond=0)
            next_tick = now_floor + dt.timedelta(seconds=1)

            date_str = format_date(now)
            year_str = f"{now.year:04d}"
            hhmm = now.strftime("%H%M")
            colon_on = (now.second % 2) == 0

            buf = bytearray()

            if date_str != last_date:
                buf += build_write_at(0, 0, width, height, pad_or_trim(date_str, width))
                last_date = date_str

            if year_str != last_year:
                buf += build_write_at(3, max(0, width - 4), width, height, year_str.encode("ascii"))
                last_year = year_str

            if (hhmm != last_hhmm) or (colon_on != last_colon):
                top, bottom = render_big_time(hhmm, colon_on)
                buf += build_write_at(1, time_start_col, width, height, top)
                buf += build_write_at(2, time_start_col, width, height, bottom)
                last_hhmm = hhmm
                last_colon = colon_on

            if buf:
                ser.write(buf)
                ser.flush()

            sleep_s = (next_tick - dt.datetime.now(tz=tz)).total_seconds()
            time.sleep(max(0.01, sleep_s))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
