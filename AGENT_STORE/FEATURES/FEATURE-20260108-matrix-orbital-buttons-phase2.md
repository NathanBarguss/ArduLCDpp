### Goal
Add optional button/key event support for Matrix Orbital mode so LCD Smartie can receive input events, without impacting LOS-PANEL behavior.

### Background
Matrix Orbital character LCDs often support keypad input. The hardware prototype plan includes up to 4 momentary buttons on pull-up inputs. This feature is explicitly Phase 2 so the display bridge MVP can ship first.

### Requirements
- Buttons are active only when the session is in Matrix Orbital mode.
- Support 4 buttons (configurable pin map at compile time; default mapping documented).
- Debounce and repeat behavior must be predictable and documented (initial press + optional repeat).
- Send Matrix Orbital key events over serial TX in the format LCD Smartie expects.
- No LOS-PANEL side effects:
  - No extra bytes transmitted.
  - No mode toggles.
  - No timing regressions in burst-safe RX handling.

### Dependencies
- `FEATURE-20260108-protocol-autodetection` and `FEATURE-20260108-matrix-orbital-mvp`.
- Hardware wiring decision for button pins (and confirmation of pull-up strategy).
- Optional: a host-side script that validates key events (extend the automated harness).

### Implementation Plan
1. Confirm the exact LCD Smartie / Matrix Orbital key event encoding via traffic capture or documentation.
2. Implement a small GPIO/button module:
   - Polling loop and debounce.
   - Event queue that emits key events without blocking RX parsing.
3. Gate the module on active protocol == Matrix Orbital.
4. Add tests (or scripted validation) for press/release/repeat.

### Acceptance Criteria
- In Matrix Orbital mode, LCD Smartie receives distinct key events for each button.
- In LOS-PANEL mode, there is no serial TX output and no visible performance change.
- Button handling does not introduce missed bytes or display stutter under T4/T8 burst scenarios.

### Validation Notes
- Bench test with serial console open (remember DTR resets; wait ~2-3s after opening the port).
