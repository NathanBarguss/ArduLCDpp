# Feature Delivery Priorities

This queue focuses on landing a stable OLED MVP (currently `LCDW=20`, `LCDH=4`) while keeping the existing HD44780 path untouched and adding a dual-display parity build for live comparisons. Each item lists the ticket ID, why it must happen in this slot, and the most important prerequisites. When you touch specs created before 2026-01-04, update them if they still assume "only one display per build."

## Phase 1 - Core Abstraction *(Completed)*
- (Done) **FEATURE-20251223-display-backend-interface** - Resolved (see `FEATURES/RESOLVED/`). Rendering now goes through `IDisplay`, so downstream tickets can assume the abstraction exists.

## Phase 2 - OLED Bring-up
1. **FEATURE-20251223-oled-backend** - Vendor `lcd2oled` into `lib/` and stand up `OLEDDisplay` using the new interface. No los-panel logic changes yet; this just proves we can talk to the panel.
2. **FEATURE-20251223-oled-command-translator** - lcd2oled lacks `command()`, so intercept `0xFE` traffic and emit equivalent interface calls. Without this layer LCDproc cannot control cursors, clears, or CGRAM.
3. **FEATURE-20251223-cgram-shim** - Extend the translator so CGRAM writes become `createChar()` calls; prerequisite for bargraph/icon fidelity.
4. **FEATURE-20251223-custom-char-parity** - Exercise the shim with real LCDproc glyphs and add validation assets so regressions are caught early.
5. **FEATURE-20251223-oled-geometry** - Lock the logical window to the firmware geometry (`LCDW` x `LCDH`) and clamp coordinates so LCDproc layouts behave the same on LCD and OLED.
6. **FEATURE-20251223-oled-backlight** - Map los-panel brightness commands to OLED brightness settings once basic rendering is proven.
7. **FEATURE-20251223-oled-performance** - Patch lcd2oled `clear()` for 128x32 panels and address any flicker from rapid updates after the core features work.

## Phase 3 - Build Selection & Validation
1. **FEATURE-20251223-backend-config-switch** - With both backends functional, add the compile-time selector plus default macros so we can create LCD-only, OLED-only, and dual builds deterministically.
2. **FEATURE-20260104-dual-display-parity** - Stand up the mirrored output mode so UX/QA can evaluate LCD vs OLED parity without reflashing between tests.
3. **FEATURE-20251223-smoke-test-matrix** - Resolved; see `FEATURES/RESOLVED/FEATURE-20251223-smoke-test-matrix.md`. Captures the dual-backend (and dual-mode) checklist and references `docs/display_smoke_tests.md` + `scripts/t4_with_logs.py`.
4. **FEATURE-20251223-oled-docs** - Resolved; see `FEATURES/RESOLVED/FEATURE-20251223-oled-docs.md`. Adds `docs/oled_i2c_setup.md` and README links for SSD1306 bring-up.

## Phase 3.5 - Burst-Safe Streaming (Recently Landed)
1. **BUG-20260104-serial-overrun** - Resolved (see `BUGS/RESOLVED/BUG-20260104-serial-overrun.md`). Unpaced T4/T8 pass on `nano168_dual_serial` by deferring display work during host bursts and refreshing during idle.
2. **FEATURE-20260107-sram-budget-and-trims** - Active. Captures the SRAM headroom targets and low-risk trim knobs validated on Nano168.
3. **FEATURE-20260107-explicit-streaming-ux-mode** - Active. Captures a UX toggle to make "Streaming safe" vs "Immediate" an explicit operator choice.

## Deferred / Post-MVP
1. **FEATURE-20251223-128x64-scouting** - Research path for SSD1306 128x64 panels (potential 16x8 or 20x4 targets). Leave this deprioritized until the current MVP ships and stabilizes.

## Phase 4 - Infrastructure & Documentation
1. **FEATURE-20260102-automated-test-harness** - Build a host-side tool that replays the smoke-test byte sequences and records structured pass/fail results so we can automate regressions.
2. **FEATURE-20260102-backlight-calibration** - Calibrate the Nano D11 PWM curve (or document hardware tweaks) so `0xFD` brightness levels produce distinct, predictable intensity steps.
3. **FEATURE-20260102-refresh-schematics** - Update `resources/wiring_schematic.*` to match the 2026 wiring table (RS=D12, bus on D3–D10, PWM on D11) so docs stop diverging from hardware.

## Recently Resolved / Archived References
- **FEATURE-20251223-platformio-migration** - PlatformIO build + multi-env workflow landed; see `FEATURES/RESOLVED/`.
- **FEATURE-20251223-startup-screen** - Original waiting-for-host banner implemented; superseded by the power-on message update.
- **FEATURE-20260102-power-on-message** - Current boot banner with shortened URL is live.
- **FEATURE-20251223-smoke-test-matrix** - Manual smoke tests documented and aligned with OLED + Dual expectations; see `FEATURES/RESOLVED/FEATURE-20251223-smoke-test-matrix.md`.
- **FEATURE-20251223-oled-docs** - SSD1306 wiring + environment/config walkthrough; see `FEATURES/RESOLVED/FEATURE-20251223-oled-docs.md`.
