#!/usr/bin/env python3
"""
Replay the T4 los-panel burst and capture firmware debug logs simultaneously.

Usage:
    python scripts/t4_with_logs.py --port COM6
"""

import argparse
import sys
import threading
import time
from typing import List

try:
	import serial  # type: ignore
except ImportError as exc:  # pragma: no cover - runtime dependency
	sys.stderr.write("pyserial is required: pip install pyserial\n")
	raise

BAUDRATE = 57600


def build_t4_payload(fill_byte: int = 0x5A) -> bytes:
	"""Return the standard T4 payload (clear + home + 80 data bytes)."""
	header = bytes([0xFE, 0x01, 0xFE, 0x02])
	body = bytes([fill_byte & 0xFF] * 80)
	return header + body


def reader_thread(ser: serial.Serial, stop_event: threading.Event, sink: List[str]) -> None:
	"""Continuously read lines from Serial and stash them in sink."""
	while not stop_event.is_set():
		line = ser.readline()
		if line:
			decoded = line.decode("utf-8", errors="replace").rstrip()
			print(decoded)
			sink.append(decoded)
		else:
			time.sleep(0.01)


def run(port: str, delay_before_send: float, capture_duration: float, fill_byte: int) -> int:
	payload = build_t4_payload(fill_byte)
	print(f"[T4] opening {port} @ {BAUDRATE} baud")
	with serial.Serial(port, BAUDRATE, timeout=0.01) as ser:
		ser.reset_input_buffer()
		print(f"[T4] waiting {delay_before_send:.1f}s for board reset...")
		time.sleep(delay_before_send)
		stop_event = threading.Event()
		logs: List[str] = []
		reader = threading.Thread(target=reader_thread, args=(ser, stop_event, logs), daemon=True)
		reader.start()
		try:
			print(f"[T4] sending {len(payload)} bytes (fill=0x{fill_byte:02X})")
			ser.write(payload)
			ser.flush()
			print(f"[T4] capture window {capture_duration:.1f}s...")
			time.sleep(capture_duration)
		finally:
			stop_event.set()
			reader.join(timeout=1.0)

	if any("raw: host active" in line for line in logs):
		print("[T4] firmware acknowledged host activity.")
	else:
		print("[T4] WARNING: no 'raw: host active' log observed; "
		      "ensure the los-panel burst reached the device.")

	return 0


def main() -> int:
	parser = argparse.ArgumentParser(description="Run los-panel T4 burst with log capture.")
	parser.add_argument("--port", required=True, help="Serial port (e.g. COM6 or /dev/ttyUSB0)")
	parser.add_argument("--delay", type=float, default=3.0,
	                    help="Seconds to wait after opening the port before sending (default: 3)")
	parser.add_argument("--capture", type=float, default=5.0,
	                    help="Seconds to keep capturing logs after sending (default: 5)")
	parser.add_argument("--fill", type=lambda s: int(s, 0), default=0x5A,
	                    help="Data byte used for the 80-character fill (default: 0x5A)")
	args = parser.parse_args()
	try:
		return run(args.port, args.delay, args.capture, args.fill)
	except serial.SerialException as exc:
		sys.stderr.write(f"[T4] Serial error: {exc}\n")
		return 1


if __name__ == "__main__":
	raise SystemExit(main())
