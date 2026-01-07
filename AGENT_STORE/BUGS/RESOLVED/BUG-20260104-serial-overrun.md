# Summary
Resolved (2026-01-07): Unpaced host bursts (T4/T8 los-panel traffic) could drop tail bytes on translator-driven dual/OLED builds because display work (especially OLED I2C) blocked `serial_read()` long enough to overrun the UART RX buffer.

# Environment
- Firmware branch: `feature/FEATURE-20251223-oled-backend`
- Hardware: Bench Nano ATmega168 with HD44780 + SSD1306 connected (COM6, 57,600 8-N-1)
- Host tooling: `scripts/t4_with_logs.py` (T4/T8 los-panel playback)

# Steps to Reproduce
1. Reset the Nano (opening the serial port resets automatically). Wait 2-3 seconds for the splash screen.
2. Send `FE 01`, `FE 02`, followed immediately by 80 data bytes (e.g., `0x5A` repeated) with no delay between bytes.
3. Close the serial port as soon as the bytes are transmitted.

# Expected Results
- All four rows fill with the streamed character (rows 0,1,2,3) on both LCD and OLED, matching HD44780 behavior.

# Actual Results
- Under fast bursts, the last row (or tail of the payload) can be missing or incomplete, indicating the MCU did not receive/consume the final bytes in time.

# Attempts So Far
- Translator and serial telemetry: `SerialDebug` + gated logging once `host_active` is detected; `scripts/t4_with_logs.py` coexists with the firmware and captures the gated diagnostics.
- OLED performance work: `lcd2oled::Draw()` already batches a character into a single I2C transmission, but at 100 kHz the per-character cost is still too high for bursty UART workloads.

# 2026-01-07 Telemetry Capture (COM6, Nano168 dual)
- Firmware env: `nano168_dual_serial`
- Key boot metrics observed:
  - `debug: i2c.clock.hz=100000`
  - `debug: free_sram.after_banner=134`
- T4 (84 bytes) and T8 (1024 bytes) replayed successfully with `scripts/t4_with_logs.py`:
  - Firmware reaches `raw: host active` and stays stable (no boot loops / resets).
  - With mitigation enabled, display work is deferred during the burst and performed during idle.
  - When idle (`Serial.available()==0`), OLED catch-up happens as row refreshes:
    - `dual.refresh.row_us ~= 17900` per row (20 columns), repeated for rows 0-3.

# Fix Implemented (defer display work while host streaming)
- Goal: prevent OLED I2C (and other per-byte display work) from blocking `serial_read()` during unpaced bursts (root cause of missing tail bytes).
- Build-time flag: `ENABLE_DUAL_QUEUE=1` (enabled for `nano168_dual_serial` only).
- Runtime behavior:
  - Before `host_active`, mirroring stays synchronous (boot banner and init remain visible on both panels).
  - After `host_active`, display updates are deferred: `DualDisplay::write()` updates a small 20x4 shadow buffer instead of issuing slow display calls.
  - During idle time, `serviceDisplayIdleWork()` refreshes up to one dirty row per call, so bursts are not blocked by I2C.
- SRAM reclamation for ATmega168:
  - `nano168_dual_serial` builds `lcd2oled` with `-DLCD2OLED_ENABLE_TEXT_BUFFER=0` to free RAM for the dual 20x4 shadow buffer.

- Supporting change: `Hd44780CommandTranslator` avoids redundant `setCursor()` calls (less per-byte overhead during bursts).

# Runtime Constraints
- ATmega168 headroom is tight: `nano168_dual_serial` reports 885/1024 bytes RAM used, with `free_sram.after_banner=134`.
- Default TWI/I2C speed remains 100 kHz; OLED row refresh costs ~18 ms per row at that speed.

# Fix Plan
1. Add a DOR0/FE0 error counter in `serial_read()` so we can assert "no UART overruns" under T8 (rather than inferring it from backlog timing).
2. Decide whether to bump I2C to 400 kHz in `OLEDDisplay::begin()` (if the module is stable at 400 kHz) to shrink `dual.refresh.row_us` without impacting burst processing.
3. Bench-confirm UX: during a burst the OLED will lag, then snap to parity once the host goes idle.

# Verification Notes
- PASS (2026-01-07) T4: `python scripts/t4_with_logs.py --port COM6 --delay 3 --capture 8 --fill 0x5A --test t4`
  - `rx.bytes_total=84`, no resets, both displays end in 4 full rows (OLED fills after idle refresh).
- PASS (2026-01-07) T8: `python scripts/t4_with_logs.py --port COM6 --delay 3 --capture 8 --fill 0x5A --test t8`
  - `rx.bytes_total=1024`, no resets, both displays end in parity.
