# Feature Delivery Priorities

This queue focuses on landing a stable 16x4 OLED MVP while keeping the existing HD44780 path untouched. Each item lists the ticket ID, why it must happen in this slot, and the most important prerequisites.

## Phase 1 – Core Abstraction
1. **FEATURE-20251223-display-backend-interface** – Everything else depends on routing rendering through `IDisplay`. Finish this refactor and keep HD44780 as the default implementation before touching OLED work.

## Phase 2 – OLED Bring-up
2. **FEATURE-20251223-oled-backend** – Vendor `lcd2oled` into `lib/` and stand up `OLEDDisplay` using the new interface. No los-panel logic changes yet; this just proves we can talk to the panel.
3. **FEATURE-20251223-oled-command-translator** – lcd2oled lacks `command()`, so intercept `0xFE` traffic and emit equivalent interface calls. Without this layer LCDproc cannot control cursors, clears, or CGRAM.
4. **FEATURE-20251223-cgram-shim** – Extend the translator so CGRAM writes become `createChar()` calls; prerequisite for bargraph/icon fidelity.
5. **FEATURE-20251223-custom-char-parity** – Exercise the shim with real LCDproc glyphs and add validation assets so regressions are caught early.
6. **FEATURE-20251223-oled-geometry** – Lock the logical window to 16x4 (Width=16, Height=4) and clamp coordinates so LCDproc layouts behave the same on LCD and OLED.
7. **FEATURE-20251223-oled-backlight** – Map los-panel brightness commands to OLED brightness settings once basic rendering is proven.
8. **FEATURE-20251223-oled-performance** – Patch lcd2oled `clear()` for 128x32 panels and address any flicker from rapid updates after the core features work.

## Phase 3 – Build Selection & Validation
9. **FEATURE-20251223-backend-config-switch** – With both backends functional, add the compile-time selector plus default macros so we can create “LCD” vs “OLED” builds deterministically.
10. **FEATURE-20251223-smoke-test-matrix** – Publish the dual-backend test checklist and demo scripts, capturing the scenarios already exercised during bring-up.
11. **FEATURE-20251223-oled-docs** – Finalize `/docs/oled_i2c_setup.md`, compile flag instructions, and LCDd.conf guidance so UX/devs have a single onboarding reference.

## Deferred / Post-MVP
12. **FEATURE-20251223-128x64-scouting** – Research path for SSD1306 128x64 panels (potential 16x8 or 20x4 targets). Leave this deprioritized until the 16x4 MVP ships and stabilizes.
