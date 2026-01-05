# Goal
Deliver a dual-display firmware mode that mirrors every lcdproc command to both the HD44780 and SSD1306 panels simultaneously so UX can compare output parity on the bench.

# Background
Until now we treated the LCD and OLED as mutually exclusive compile-time targets. That made sense while bringing the OLED online, but it blocks rapid UX evaluation because we have to reflash between builds to see rendering differences. The wiring already keeps both panels connected, so the missing piece is firmware support for driving them in parallel without regressing the existing single-backend builds.

# Requirements
- Introduce a `DUAL` display mode (name TBD) that instantiates both concrete backends and mirrors every `IDisplay` method call, including `write`, `command`, `createChar`, `clear`, and `setBacklight`.
- Ensure brightness commands map sensibly on both panels (e.g., LCD PWM vs OLED contrast) without diverging UX cues.
- Maintain the ability to ship LCD-only and OLED-only binaries; the dual path should be opt-in and compile-time selectable just like the dedicated builds.
- Document wiring, power considerations, and host workflow so UX/QA know when to use the dual mode versus the single-backend binaries.
- Update smoke-test guidance to include a parity spot-check (even if manual at first) so we catch any divergence between the two displays.

# Dependencies
- HD44780 backend (current default) and OLED backend (in-progress tickets) must both expose matching `IDisplay` behavior.
- Backlight calibration work should define mappings the dual mode can reuse.

# Implementation Plan
1. Add a composite implementation (e.g., `DualDisplay`) that wraps two `IDisplay` instances and forwards every call, handling any per-backend quirks internally. **(DONE 2026-01-04)** — `DualDisplay` now lives under `src/display/` and fans out all interface calls.
2. Extend `display_factory` to understand the new dual mode, instantiating both concrete displays once and wiring them into the composite. **(DONE 2026-01-04)** — `DISPLAY_BACKEND == DUAL` path returns one HD44780 + one OLED backend inside the composite.
3. Decide how `DISPLAY_BACKEND` (or successor flag) selects between LCD, OLED, and DUAL to avoid ad-hoc build flags. **(DONE)** — `include/DisplayConfig.h` defines `HD44780`, `OLED`, `DUAL`; `platformio.ini` now exposes `nano168_dual`.
4. Update docs/readme + AGENT_STORE guidance to explain when dual mode is useful and any hardware caveats (power draw, startup timing, etc.). **(IN PROGRESS)** — README lists the new environment, but wiring/power caveats plus smoke-test notes still need to be documented here and in `docs/display_smoke_tests.md`.
5. Exercise manual smoke tests with both panels connected; capture any timing or current issues before wider adoption. **(IN PROGRESS)** - Command translator now drives the OLED path, so parity runs can cover T1-T6; still need to log results + power notes in this ticket.
6. **2026-01-05 serial-overrun containment plan (tied to `BUG-20260104-serial-overrun`)**
   1. Rebuild/upload the `nano168_dual` env and rerun T4/T8 at full speed to confirm the current blank-row failure, recording the exact board/COM port plus photos/log snippets in the bug ticket before touching code.
   2. Add guarded debug helpers (behind `ENABLE_SERIAL_DEBUG`) that dump free SRAM, OLED init duration, and I²C clock right after `display.begin()` in `src/main.cpp`, then capture those baseline numbers inside the bug ticket’s constraint section.
   3. Introduce a reusable `SerialDebug` shim so every module can emit timestamped traces without sprinkling raw `Serial.print` calls; land this scaffolding by itself and verify T1 still passes with debug disabled.
   4. Instrument `DualDisplay::write`, `Hd44780CommandTranslator::handleData`, and `serial_read()` to log OLED blocking time, logical cursor positions, and UART starvation windows. Build with debug enabled and capture a single T4 log to prove the instrumentation works.
   5. Collect structured telemetry (serial logs, queue depth snapshots, photos) from fresh T4/T8 runs and summarize the data here plus in the bug ticket so future work starts from the same timing/memory baseline.
   6. Draft the OLED ring-buffer design in this ticket (target depth, SRAM cost, drain policy) before coding to ensure it fits the `nano168_dual` budget established above.
   7. Implement the buffer under a feature flag (`ENABLE_OLED_QUEUE`), keeping LCD writes synchronous while OLED writes enqueue `(addr,value)` pairs that drain only when `Serial.available()==0` or on a watchdog tick; add lightweight unit coverage for enqueue/dequeue correctness.
   8. Integrate the queue with the runtime loop, emit overflow warnings plus a brief backlight blink when drops occur, and validate each sub-step by re-running T4 immediately so we never need a rollback.
   9. Once the backlog drains cleanly, rerun the full smoke matrix on both `nano168_dual` and `nano_hd44780`, archive PASS evidence in this ticket, and only then move the bug file into `AGENT_STORE/BUGS/RESOLVED/`.

# Acceptance Criteria
- Building with the new dual mode flag results in both displays showing identical startup banners and host-driven content without recompiling between tests. **Startup banner parity confirmed 2026-01-04 after reordering `Serial.begin()` ahead of display init. Host-driven parity still pending command translator.**
- Brightness commands affect both panels predictably, with documented mapping. **LCD PWM + OLED contrast both respond, but the mapping hasn’t been calibrated or documented yet.**
- Existing single-backend builds continue to compile and run without regression. **All PlatformIO envs build; need a quick regression flash on `nano168_hd44780` after debug logging is removed.**
- README/AGENT instructions tell contributors how to enable the dual build and why they might use it. **Usage documented in README; still need workflow notes here plus smoke-test checklist updates.**

# Validation Notes
- Run the standard docs/display_smoke_tests checklist while watching both panels, noting any mismatches or ghosting in the ticket.
- Record supply-current observations (USB-powered vs. external) so future UX runs know if extra power is required.
- 2026-01-04 bench note: Dual build now brings up both panels when `Serial.begin()` runs before `display.begin()`. With `Hd44780CommandTranslator` in place, OLED mirrors DDRAM/CGRAM updates; document any cursor-shift gaps uncovered during testing.
