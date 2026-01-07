#!/usr/bin/env python3
"""
Replay the los-panel smoke-test bursts (T4 fullscreen fill or T8 stress burst)
while capturing firmware debug logs. Keeps the serial monitor attached so the
Nano's gated instrumentation actually emits telemetry.
"""

import argparse
import sys
import threading
import time
from typing import List, Optional

try:
	import serial  # type: ignore
except ImportError as exc:  # pragma: no cover - runtime dependency
	sys.stderr.write("pyserial is required: pip install pyserial\n")
	raise

BAUDRATE = 57600
PRINTABLE_BASE = 0x20
PRINTABLE_RANGE = 0x5F  # 0x20-0x7E inclusive
META_PREFIX = 0xFC
META_SET_STREAMING_MODE = 0x10
STREAMING_MODE_IMMEDIATE = 0
STREAMING_MODE_SAFE = 1


def build_t4_payload(fill_byte: int = 0x5A) -> bytes:
	"""Return the standard T4 payload (clear + home + 80 data bytes)."""
	header = bytes([0xFE, 0x01, 0xFE, 0x02])
	body = bytes([fill_byte & 0xFF] * 80)
	return header + body


def build_t8_payload(fill_byte: int = 0x5A) -> bytes:
	"""
	Return the 1 KB stress burst for T8: start with clear/home, then stream
	repeating command+data chunks that touch DDRAM and backlight settings.
	"""
	payload = bytearray()
	payload.extend((0xFE, 0x01, 0xFE, 0x02))  # clear + home
	brightness = 0
	addr = 0
	data_seed = fill_byte & 0xFF
	while len(payload) < 1024:
		payload.extend((0xFD, brightness & 0xFF))  # backlight tweak
		payload.extend((0xFE, 0x80 | (addr & 0x7F)))  # set DDRAM address
		for i in range(6):  # write a short burst of ASCII characters
			value = PRINTABLE_BASE + ((data_seed + i) % PRINTABLE_RANGE)
			payload.append(value)
		brightness = (brightness + 17) & 0xFF
		addr = (addr + 5) & 0x7F
		data_seed = (data_seed + 9) & 0xFF
	if len(payload) > 1024:
		return bytes(payload[:1024])
	if len(payload) < 1024:
		payload.extend(bytes([0x30]) * (1024 - len(payload)))
	return bytes(payload)


def build_streaming_mode_payload(mode: str) -> bytes:
	mode_lower = mode.strip().lower()
	if mode_lower == "immediate":
		mode_value = STREAMING_MODE_IMMEDIATE
	elif mode_lower == "safe":
		mode_value = STREAMING_MODE_SAFE
	else:
		raise ValueError(f"Unknown streaming mode: {mode}")
	return bytes([META_PREFIX, META_SET_STREAMING_MODE, mode_value])


def reader_thread(test_name: str, ser: serial.Serial, stop_event: threading.Event,
                  sink: List[str]) -> None:
	"""Continuously read lines from Serial and stash them in sink."""
	prefix = f"[{test_name}]"
	while not stop_event.is_set():
		line = ser.readline()
		if line:
			decoded = line.decode("utf-8", errors="replace").rstrip()
			print(decoded)
			sink.append(decoded)
		else:
			time.sleep(0.01)


def run(test_name: str, port: str, delay_before_send: float,
        capture_duration: float, fill_byte: int, streaming_mode: Optional[str]) -> int:
	builder = build_t4_payload if test_name.upper() == "T4" else build_t8_payload
	payload = builder(fill_byte)
	prefix = f"[{test_name.upper()}]"
	print(f"{prefix} opening {port} @ {BAUDRATE} baud")
	with serial.Serial(port, BAUDRATE, timeout=0.01) as ser:
		ser.reset_input_buffer()
		print(f"{prefix} waiting {delay_before_send:.1f}s for board reset...")
		time.sleep(delay_before_send)
		stop_event = threading.Event()
		logs: List[str] = []
		reader = threading.Thread(
		    target=reader_thread, args=(test_name.upper(), ser, stop_event, logs), daemon=True
		)
		reader.start()
		try:
			if streaming_mode:
				mode_payload = build_streaming_mode_payload(streaming_mode)
				print(f"{prefix} setting streaming mode: {streaming_mode}")
				ser.write(mode_payload)
				ser.flush()
				time.sleep(0.05)

			print(f"{prefix} sending {len(payload)} bytes (fill=0x{fill_byte:02X})")
			ser.write(payload)
			ser.flush()
			print(f"{prefix} capture window {capture_duration:.1f}s...")
			time.sleep(capture_duration)
		finally:
			stop_event.set()
			reader.join(timeout=1.0)

	if any("raw: host active" in line for line in logs):
		print(f"{prefix} firmware acknowledged host activity.")
	else:
		print(f"{prefix} WARNING: no 'raw: host active' log observed; "
		      "ensure the los-panel burst reached the device.")

	boot_markers = sum("instrumentation armed" in line for line in logs)
	if boot_markers > 1:
		print(f"{prefix} WARNING: detected {boot_markers} boot banners during capture "
		      "(possible reset).")
	else:
		print(f"{prefix} no resets detected in capture window.")

	return 0


def main() -> int:
	parser = argparse.ArgumentParser(description="Run los-panel bursts (T4 or T8) with log capture.")
	parser.add_argument("--port", required=True, help="Serial port (e.g. COM6 or /dev/ttyUSB0)")
	parser.add_argument("--delay", type=float, default=3.0,
	                    help="Seconds to wait after opening the port before sending (default: 3)")
	parser.add_argument("--capture", type=float, default=5.0,
	                    help="Seconds to keep capturing logs after sending (default: 5)")
	parser.add_argument("--fill", type=lambda s: int(s, 0), default=0x5A,
	                    help="Data byte used for generated payloads (default: 0x5A)")
	parser.add_argument("--test", choices=("t4", "t8"), default="t4",
	                    help="Which smoke-test payload to send (default: t4)")
	parser.add_argument("--streaming", choices=("safe", "immediate"),
	                    help="Optional: send FC 10 <mode> before the payload")
	args = parser.parse_args()
	try:
		return run(args.test, args.port, args.delay, args.capture, args.fill, args.streaming)
	except serial.SerialException as exc:
		prefix = f"[{args.test.upper()}]"
		sys.stderr.write(f"{prefix} Serial error: {exc}\n")
		return 1


if __name__ == "__main__":
	raise SystemExit(main())
