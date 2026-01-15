### Goal
Support the minimal Matrix Orbital character-LCD command subset required for LCD Smartie compatibility while keeping LOS-PANEL behavior intact.

### Background
LCD Smartie 5.4.x can drive character LCDs using a Matrix Orbital-compatible serial protocol. ArduLCDpp already has a unified character-grid backend for HD44780 and SSD1306 (via emulated semantics). Implementing a Matrix Orbital parser that maps into the backend enables Windows host compatibility without changing display drivers.

### Requirements
- Target host: LCD Smartie 5.4.x (expected compatible with any 5.x character LCD output).
- Protocol framing:
  - ASCII bytes are printable characters.
  - `0xFE <command> [parameters...]` are Matrix Orbital control commands.
  - LCD Smartie does not send raw HD44780 opcodes in this mode.
  - Treat `0xFE` as a Matrix Orbital command introducer only after protocol detection selects Matrix Orbital.
- Implement only the subset LCD Smartie uses for a fixed 20x4 character LCD workflow:
  - Print text.
  - Clear screen.
  - Home.
  - Set cursor (x,y).
  - Custom characters (CGRAM) for indices 0-7.
  - Backlight brightness.
- Geometry is fixed at 20x4 for v1 Matrix Orbital mode (no 16x2/16x4, no runtime switching, no auto-detect).
- Unknown/unsupported Matrix Orbital commands are ignored safely (no crashes, no protocol router reset).
- Matrix Orbital parsing must not interfere with LOS-PANEL parsing (selection handled by protocol auto-detection).

### Dependencies
- `FEATURE-20260108-protocol-autodetection` (runtime selection and routing).
- Existing `IDisplay` backend abstraction (LCD/OLED/Dual builds).
- A traffic capture or byte-sequence fixtures from LCD Smartie to confirm the exact commands used.

### Implementation Plan
1. Capture LCD Smartie 5.4.x byte sequences (serial TX capture or log via Python proxy).
2. Implement a Matrix Orbital command dispatcher:
   - Distinguish command bytes from printable bytes.
   - Map required commands into `IDisplay` calls.
3. Implement cursor mapping rules for 20x4, including bounds clamping.
4. Implement CGRAM writes for indices 0-7 using existing parity/shim logic.
5. Implement backlight mapping to the backend's `setBacklight(level)` semantics.
6. Add minimal observability for bring-up (optional debug logs, behind compile-time flags).

### Acceptance Criteria
- LCD Smartie can:
  - Clear the screen and print text across multiple rows.
  - Move the cursor and overwrite in-place.
  - Define and display custom characters reliably.
  - Adjust backlight brightness.
- LOS-PANEL smoke tests continue to pass unchanged.
- Behavior is consistent across LCD-only, OLED-only, and Dual builds.

### Validation Notes
- Validate against a recorded LCD Smartie session fixture to prevent regressions.
- Verify that CGRAM parity and burst-safety features still hold under Smartie update rates.
