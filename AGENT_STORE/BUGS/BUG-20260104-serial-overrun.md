# Summary
Translator-driven dual/OLED builds miss the last row during bulk writes because the UART RX buffer overruns when 80 bytes arrive without pacing.

# Environment
- Firmware: `feature/FEATURE-20251223-oled-backend` (nano168_dual env)
- Hardware: Bench Nano ATmega168 with HD44780 + SSD1306 connected
- Host tooling: Manual los-panel smoke scripts via Python/pyserial on COM6 @ 57,600 bps

# Steps to Reproduce
1. Reset the Nano (opening the serial port resets automatically). Wait 2-3 seconds for the splash screen.
2. Send `FE 01`, `FE 02`, followed immediately by 80 data bytes (e.g., `0x5A` repeated) without any delay between bytes.
3. Close the serial port as soon as the bytes are transmitted.

# Expected Results
- All four rows fill with the streamed character (row order 0,1,2,3) on both LCD and OLED, matching HD44780 behavior.

# Actual Results
- Rows 0-2 fill correctly, but the fourth row remains blank on both displays.
- If the same payload is throttled with ~10 ms between bytes—or the host keeps the port open a few seconds after writing—the full screen renders, indicating the device never received the final bytes during the fast burst.

# Attempts So Far
- **2026-01-04, build 857e59d:** Baseline translator landed; T4 still drops the fourth row on both displays.
- **Cursor-cache optimization:** Added logical/physical cursor tracking in `Hd44780CommandTranslator`. Helped OLED alignment but bulk writes still stalled once OLED I²C writes kicked in.
- **OLED buffering prototype:** Tried rewriting `OLEDDisplay` to shadow DDRAM and flush rows lazily. Result: startup banner/backlight never rendered because OLED init blocked the main loop; reverted.
- **Serial ring buffer experiments:** Added 80–160 byte software FIFOs in `main.cpp`. Every attempt either consumed too much RAM (OLED begin hung) or still left the OLED path too slow, so we reverted to the stable build.

# 2026-01-05 Bench Verification
- Firmware: `nano168_dual` (commit `0756ebad`, PlatformIO upload via COM6 at 57,600 bps).
- T4 (80-byte fill) replayed with `python`/`pyserial` burst: rows 0–2 mirrored correctly, row 3 stayed blank on both LCD and OLED while the host closed the port immediately after flushing.
- T8 (1 KB stress mix) replayed the same session: after the initial splash the LCD/OLED stalled during the burst—per bench observation the tail of the payload never appeared until the host resent with pacing, matching the original failure description.
- Artifacts: command snippets live in this session log; photos/serial captures pending UX upload so we can archive them alongside the ticket.

# Runtime Constraints (captured 2026-01-05 with `ENABLE_SERIAL_DEBUG=1`)
- `display.begin()` total latency: ~200,460 µs on the Nano168 dual build (includes HD44780 + OLED init).
- Free SRAM reported immediately after init: 145 bytes remaining, so any buffering solution must stay well under this delta or reclaim memory elsewhere.
- I²C clock derived from TWI registers: 100 kHz (`compute_i2c_clock_hz()`), matching the stock Wire default; this value should be referenced when modeling OLED drain rates or deciding whether to bump TWI speed.

# Fix Plan (next steps)
1. **Document hard constraints:** Measure and log the current free SRAM on `nano168_dual`, OLED refresh latency, and configured I²C clock. Confirm `lib/lcd2oled` matches the upstream tip. Record these numbers in this ticket so every iteration starts from the same timing/memory baseline.
2. **Instrumentation-only build:** Add `ENABLE_SERIAL_DEBUG` hooks in `setup()`, `DualDisplay::write`, translator entry/exit points, and the OLED drain loop. Emit timestamps, queue depth, and serial occupancy without changing runtime behavior. Flash this build and rerun T1–T4 to capture a fresh trace plus photos/video of the blank-row failure for comparison.
3. **Serial pacing telemetry:** Extend the debug build to sample UART backlog each loop (bytes pending, longest stall) and log how long OLED transfers keep interrupts disabled. This data lets us size the queue scientifically instead of guessing.
4. **OLED queue architecture spike:** Prototype a ~96-entry `(addr,value)` ring buffer owned by the dual backend while keeping LCD writes synchronous. Only drain the queue when `Serial.available()==0` or on a watchdog tick so OLED updates never block bursts. Micro-benchmark enqueue/dequeue to stay under ~20 µs per operation on ATmega168.
5. **Overflow handling + UX cues:** Track queue drops, emit a debug warning, and (behind a debug flag) briefly blink row 3 or toggle the backlight so bench testers notice when parity is compromised mid-demo.
6. **Stepwise validation:** After each iteration, rerun T4 (full-screen fill) and T8 (stress burst) from `docs/display_smoke_tests.md` at full speed, logging queue depth over time and ensuring zero drops. Capture artifacts for `FEATURE-20260104-dual-display-parity.md`.
7. **Regression sweep + env notes:** Before closing the bug, flash both `nano_hd44780` and `nano168_dual` to ensure RAM usage stays safe on the tighter boards. Update this ticket with which PlatformIO env was tested so hardware swaps remain frictionless.

# Verification Notes
- Re-run T4 from `docs/display_smoke_tests.md` at full speed after the translator change; confirm all four rows fill without adding host delays.
- Capture the same test on both displays (photos acceptable) and note the PASS result in `FEATURE-20260104-dual-display-parity.md`.
