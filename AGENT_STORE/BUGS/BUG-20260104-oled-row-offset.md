# Summary
OLED cursor renders one row lower than expected after HD44780 command translation.

# Environment
- Firmware: `feature/FEATURE-20251223-oled-backend` branch, `nano168_dual` PlatformIO env
- Hardware: Bench Arduino Nano ATmega168 with HD44780 LCD + SSD1306 128x32 OLED connected in parallel
- Host tooling: Manual los-panel byte scripts via Python/pyserial (COM6, 57,600 bps)

# Steps to Reproduce
1. Reset the Nano (opening the serial port resets it automatically). Wait ~2.5 seconds for the firmware to boot and draw the splash screen on both panels.
2. Send `FE 01`, `FE 02`, then ASCII `A` (clear, home, write character). Example snippet:
   ```python
   import serial, time
   with serial.Serial('COM6', 57600, timeout=1) as ser:
       time.sleep(2.5)
       ser.write(bytes([0xFE, 0x01]))
       ser.write(bytes([0xFE, 0x02]))
       ser.write(b'A')
       time.sleep(0.5)
   ```

# Expected Results
- Both displays clear and show a single `A` at row 0, column 0 (upper-left).

# Actual Results
- LCD behaves correctly.
- OLED prints `A` one character cell lower (appears on the "second line"), suggesting the translated DDRAM address maps to the wrong page/offset on 128x32 hardware.

# Fix Plan
- Investigate SSD1306 page-to-row mapping when LCDH=4 but OLED has only 4 pages (128x32). The translator may need a per-backend row offset or the OLED driver may need to adjust the vertical origin when `setCursor` is called.
- Verify after fix by rerunning T2/T3 from `docs/display_smoke_tests.md` and documenting the outcome in `FEATURE-20260104-dual-display-parity`.

# Verification Notes
- Capture before/after photos of both panels to confirm the cursors line up.
- Re-run the entire cursor sweep (T3) once the single-character case is fixed to ensure every DDRAM address maps to the correct OLED row/column.
