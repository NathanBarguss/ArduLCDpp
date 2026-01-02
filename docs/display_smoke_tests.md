# Display Smoke Test Plan

This checklist keeps HD44780 and OLED backends protocol-compatible as we add hardware. Each row is runnable with nothing more than the ArduLCDpp firmware, a USB cable, and a terminal capable of writing raw los-panel bytes.

## Prerequisites
- Build + upload the firmware for the backend under test (`& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -t upload -e <env>`).
- Note the serial port name (`COMx` on Windows, `/dev/ttyUSBx` on Linux/macOS).
- Host machine needs Python 3 plus `pyserial` (`pip install pyserial`) to replay the command snippets below.

## Sending Test Sequences
Use the helper script to emit arbitrary los-panel bytes:

```powershell
python - <<'PY'
import sys, time
import serial
PORT = r"COM5"  # update to your board's port
seq = bytes.fromhex(
    "FE 01"        # clear
    "FE 02"        # home
    "FD 00"        # backlight off
    "FD FF"        # backlight full
    "41 42 43"     # data bytes (ABC)
)
with serial.Serial(PORT, 57600, timeout=1) as ser:
    ser.write(seq)
    ser.flush()
    time.sleep(0.5)
PY
```

Substitute the `seq` bytes per the matrix below. You can also drive LCDproc itself (see `resources/LCDd.conf`), but the raw-byte method removes the daemon as a variable.

## Test Matrix

| ID | Scenario | Procedure | Expected (HD44780) | Expected (OLED) | Notes |
|----|----------|-----------|--------------------|-----------------|-------|
| T1 | Power-on banner | Reset board; observe boot text before host starts streaming. | `%dx%d Ready` centered at (0,0) with backlight set to `STARTUP_BRIGHTNESS`. | Same banner intensity/position; OLED may emulate backlight via contrast or stay static if not supported. | Ensures factory backlight + `display.begin()` run before serial traffic. |
| T2 | Clear/Home bounds | Send `FE 01` (clear) then `FE 02` (home). Follow with ASCII `A`. | Display blanked; `A` appears at row 0 col 0. | Exact cursor behavior match; no stale pixels. | Confirms los-panel command passthrough. |
| T3 | Cursor sweep | Loop `y=0..LCDH-1`, `x=0..LCDW-1`: send `FE 80|addr` for DDRAM, write marker char. | Cursor honors geometry; characters land in a perfect grid. | Identical layout (e.g., 20×4) with no wrapping artifacts. | Use script to emit the full sweep to catch addressing math errors. |
| T4 | Full-screen fill | Stream exactly `LCDW*LCDH` bytes without escape (`41` repeated, etc.). | Panel fills sequentially left-to-right, top-to-bottom; no truncation. | Same behavior; OLED text buffer must mimic HD44780 wrapping. | Detects write() buffering regressions. |
| T5 | Custom characters | For slots 0–7: send `FE 40|(slot<<3)` followed by 8 pattern bytes, then write the slot indices (`00–07`). | HD44780 renders uploaded glyphs; bytes persist until next `createChar`. | OLED backend must either translate to its API or, if unsupported, document limitation (expected FAIL until implemented). | Use `docs/lcdproc_display_mapping.md` for glyph references. |
| T6 | Backlight/brightness | Send `FD 00`, `FD 80`, `FD FF` with 500 ms between. | PWM brightness visibly changes; `analogWrite` values map linearly. | OLED should map to contrast/dimming. If hardware lacks backlight, note “N/A” but keep command a no-op. | Confirms `setBacklight` wiring per backend. |
| T7 | USB reconnect | While streaming data (T4), unplug USB for 5 seconds, reconnect, resend data. | Firmware resumes stream after host reopens port; no freeze in `serial_read`. | Same expectation; OLED buffers must re-init if needed. | Helpful to watch host logs for serial errors. |
| T8 | Stress burst | Send 1 KB of mixed bytes (commands + data) without delay. | No dropped bytes; LiquidCrystal keeps pace even if characters scroll offscreen. | OLED translation layer must avoid watchdog resets; display may briefly lag but should recover without corruption. | Helps catch future OLED command translators. |

## Execution Notes
- Record PASS/FAIL per backend and attach photos where visuals matter (custom chars, fills).
- If testing OLED before its backend lands, mark the column as “blocked” and capture the missing feature in the relevant ticket.
- Before merging display-affecting PRs, mention in the PR description: `Smoke: T1–T8 on HD44780 (PASS), OLED (BLOCKED: backend WIP)`.
