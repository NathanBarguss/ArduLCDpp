# Display Smoke Test Plan

This checklist keeps HD44780 and OLED backends protocol-compatible as we add hardware. Each row is runnable with nothing more than the ArduLCDpp firmware, a USB cable, and a terminal capable of writing raw los-panel bytes.

## Prerequisites
- Build + upload the firmware for the backend under test (`& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -t upload -e <env>`).
- Note the serial port name (`COMx` on Windows, `/dev/ttyUSBx` on Linux/macOS).
- Host machine needs Python 3 plus `pyserial` (`pip install pyserial`) to replay the command snippets below.

## Suggested Environments
- HD44780 only: `nano168_hd44780` (Nano ATmega168) or `nano_hd44780` (Nano ATmega328P).
- OLED only: `nano168_oled` (Nano ATmega168).
- Dual parity (quiet): `nano168_dual` (both panels, serial debug off).
- Dual + serial debug + burst-safe streaming: `nano168_dual_serial` (use this for T4/T8 on Nano168; display updates may lag during the burst and then catch up during idle).

## Streaming Mode (optional)
In dual builds, the firmware supports an explicit streaming UX toggle:
- `FC 10 01` = StreamingSafe (defer display work during bursts, catch up on idle)
- `FC 10 00` = Immediate (write-through; may require host pacing to avoid drops on small MCUs)

## Sending Test Sequences
Use the helper script to emit arbitrary los-panel bytes. **Always wait at least 2-3 seconds after opening the serial port before sending data**-the Nano auto-resets when DTR toggles, and anything sent while the bootloader is running gets dropped. Likewise, keep the port open for a few seconds after sending so humans can verify the display state.

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

### Burst Tests (T4/T8)
For unpaced burst tests (and for logs that coexist with the firmware's gated diagnostics), prefer the harness script:

```powershell
python scripts/t4_with_logs.py --port COM6 --delay 3 --capture 8 --fill 0x5A --test t4
python scripts/t4_with_logs.py --port COM6 --delay 3 --capture 8 --fill 0x5A --test t8
```

## Test Matrix

| ID | Scenario | Procedure | Expected (HD44780) | Expected (OLED) | Expected (Dual) | Notes |
|----|----------|-----------|--------------------|-----------------|-----------------|-------|
| T1 | Power-on banner | Reset board; observe boot text before host starts streaming. | `"ArduLCDpp Ready"` (row 0), `"Waiting for host..."` (row 1 when available), and `bit.ly/4plthUv` (row 2 when the panel exposes a third row) centered with backlight set to `STARTUP_BRIGHTNESS`. Banner clears automatically after the first serial byte. | Same banner intensity/position; OLED may emulate backlight via contrast or stay static if not supported. | Both panels show the same banner content and clear at first host byte. | Ensures factory backlight + `display.begin()` run before serial traffic. |
| T2 | Clear/Home bounds | Send `FE 01` (clear) then `FE 02` (home). Follow with ASCII `A`. | Display blanked; `A` appears at row 0 col 0. | Exact cursor behavior match; no stale pixels. | Both panels show `A` at row 0 col 0. | Confirms los-panel command passthrough. |
| T3 | Cursor sweep | Loop `y=0..LCDH-1`, `x=0..LCDW-1`: send `FE 80|addr` for DDRAM, write marker char. | Cursor honors geometry; characters land in a perfect grid. | Identical layout (e.g., 20x4) with no wrapping artifacts; translator converts DDRAM addresses into OLED cursor positions. | Both panels track the same sweep positions with no mismatch. | Use script to emit the full sweep to catch addressing math errors. |
| T4 | Full-screen fill (unpaced) | Stream exactly `LCDW*LCDH` bytes without escape (`41` repeated, etc.). | Panel fills sequentially left-to-right, top-to-bottom; no truncation. | Same behavior; OLED text buffer must mimic HD44780 wrapping. | On `nano168_dual_serial`, display updates can lag during the burst, then both panels fill to parity once RX goes idle. | Detects buffering/throughput regressions; prefer `scripts/t4_with_logs.py` for burst runs. |
| T5 | Custom characters | For slots 0-7: send `FE 40|(slot<<3)` followed by 8 pattern bytes, then issue `FE 80` (home) before writing the slot indices (`00-07`). | HD44780 renders uploaded glyphs; bytes persist until next `createChar`. | OLED backend now mirrors the same glyphs by translating CGRAM writes into `createChar` calls. | Both panels show matching glyphs for slots 0-7. | Use `docs/lcdproc_display_mapping.md` for glyph references. DDRAM must be reselected after CGRAM writes or the glyph bytes keep programming CGRAM instead of appearing on-screen. If you send `FE 01` (clear) before writing the glyph indices, add a short delay after clear/home (HD44780 clear can block long enough to drop subsequent UART bytes). |
| T6 | Backlight/brightness | Send `FD 00`, `FD 80`, `FD FF` with 500 ms between. | PWM brightness visibly changes; `analogWrite` values map linearly. | OLED should map to contrast/dimming. If hardware lacks backlight, note "N/A" but keep command a no-op. | Both panels respond (LCD PWM + OLED contrast) without desync. | Confirms `setBacklight` wiring per backend. |
| T7 | USB reconnect | While streaming data (T4), unplug USB for 5 seconds, reconnect, resend data. | Firmware resumes stream after host reopens port; no freeze in `serial_read`. | Same expectation; OLED buffers must re-init if needed. | Both panels return to parity after reconnect and resend. | **Skip when USB is the only power source** (board resets). Needs external supply or future automation hook; otherwise log as N/A. Helpful to watch host logs for serial errors. |
| T8 | Stress burst (unpaced) | Send 1 KB of mixed bytes (commands + data) without delay. | No dropped bytes; LiquidCrystal keeps pace even if characters scroll offscreen. | OLED translation layer must avoid watchdog resets; display may briefly lag but should recover without corruption. | On `nano168_dual_serial`, display updates can lag during the burst, then catch up to parity once RX goes idle; should not reset. | Prefer `scripts/t4_with_logs.py --test t8` for repeatability and logs. |

## Execution Notes
- Record PASS/FAIL per backend and attach photos where visuals matter (custom chars, fills).
- If testing a backend that isn't wired or supported on the bench, mark it as "N/A" and capture the reason in the relevant ticket.
- Before merging display-affecting PRs, mention in the PR description: `Smoke: T1-T8 on HD44780 (PASS), OLED (PASS), Dual (PASS)` (or call out any partial coverage).
