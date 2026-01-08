# Goal
Calibrate the HD44780 backlight PWM response so los-panel brightness commands (`FD 00`, `FD 80`, `FD FF`, etc.) produce visibly distinct levels on the Nano D11 + BC337 circuit.

# Background
During the 2026-01-02 smoke tests (T6), the LCD backlight only showed a subtle change between off/mid/high even though the PWM values varied from 0x00 to 0xFF. This suggests the current `analogWrite` range or transistor bias isn't aligned with the panel's optimal current. A calibrated curve (or updated resistor/driver recommendations) would make brightness control more useful and reproducible.

# Requirements
- Measure actual LED current/voltage for several PWM duty cycles and document the mapping.
- Update firmware (e.g., `setBacklight`) to apply a gamma/curve or clamp so `FD` values map to perceptually even steps.
- Document the calibration process and the expected voltage/current values in `docs/` so future hardware builders can match the behavior.
- Provide guidance for OLED backlight equivalents once implemented (even if it's a stub for now).

# Dependencies
- Existing HD44780 backlight driver (`src/display/HD44780Display.cpp`) and wiring documented in README/AGENTS.
- Access to the bench hardware (Nano168 + BC337 + LCD) and a multimeter or oscilloscope for measurement.

# Implementation Plan
1. Capture empirical readings for several PWM points (0, 32, 64, 128, 255).
2. Decide whether to adjust the hardware (resistor values) or apply a software curve; prototype the chosen approach.
3. Update firmware and re-run T6 to confirm clearly distinguishable brightness levels.
4. Update README/docs with the new calibration guidance and any required component changes.

# Acceptance Criteria
- At least three distinct brightness levels are obvious to the eye when issuing `FD 00`, `FD 80`, `FD FF`.
- Documentation captures the calibration table or formula.
- Smoke test T6 describes the calibrated expectations.

# Validation Notes
- Use a light meter or consistent photographic setup if available to show the difference between low/mid/high.
- If hardware changes are required, provide BOM updates and callouts in AGENTS.MD.
