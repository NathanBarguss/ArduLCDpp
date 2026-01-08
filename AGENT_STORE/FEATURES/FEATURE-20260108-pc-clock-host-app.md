# FEATURE-20260108-pc-clock-host-app

### Goal
Provide a simple PC-side Python “clock” app that exercises ArduLCDpp’s los-panel serial protocol and showcases big custom-character digits on both LCD and OLED in the dual-mirror build.

### Background
ArduLCDpp exposes an HD44780-compatible command/data stream over USB serial (`FE` = command passthrough, `FD` = backlight/brightness). A lightweight host demo helps validate CGRAM parity (custom chars), DDRAM addressing, and sustained UX updates without relying on a full LCDproc stack.

### Requirements
- Runs on Windows (primary bench), configurable serial port (default `COM6`).
- Uses the existing los-panel byte stream (no firmware changes, no protocol fork).
- Screen layout on `20x4`:
  - Row 1: full month name + day ordinal (e.g., `January 8th`), left aligned.
  - Rows 2-3: centered “big” `HH:MM` (24h) using the provided custom chars and block fills.
  - Row 4: current year (e.g., `2026`) right aligned.
- Colon flashes once per second.
- Works on LCD-only, OLED-only, and dual builds (protocol-identical).

### Dependencies
- Python 3 + `pyserial`.
- ArduLCDpp firmware running at `57600` baud.

### Implementation Plan
1. Add `scripts/pc_clock.py` using `pyserial` to open the port, set backlight, clear/home, upload CGRAM slots 0..7, then render updates at 1 Hz.
2. Keep updates minimal: redraw big time each second (for colon blink), update date/year only when they change.
3. Center the big time block on `20x4` using the standard HD44780 DDRAM address map.

### Acceptance Criteria
- With `nano168_dual` wired to both panels, both displays show identical clock layout and remain stable for a 10+ minute run.
- Custom character big digits render without corruption; colon reliably blinks at 1 Hz.
- Date updates at midnight; year updates on Jan 1 without manual restart.

### Validation Notes
- Run: `python scripts/pc_clock.py --port COM6 --backlight 255`
- Verify on a real `20x4` glass LCD first (contrast trimmer), then confirm OLED parity.

