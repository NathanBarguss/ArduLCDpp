### Goal
Add runtime protocol auto-detection so ArduLCDpp can accept either LOS-PANEL (LCDproc) or Matrix Orbital (LCD Smartie) traffic over the same USB serial connection without reflashing or host-side configuration.

### Background
ArduLCDpp currently behaves as a LOS-PANEL bridge and must remain bit-for-bit compatible for existing users. Adding Matrix Orbital support introduces ambiguity at the byte-stream level (both protocols use `0xFE`), so we need a safe, session-scoped protocol selection that does not misclassify real LCDproc streams.

### Requirements
- Default state is `PROTO_UNKNOWN` at boot / connect.
- Buffer the first bytes of a session (8-16 bytes, or until a short timeout) before committing to a protocol.
- Apply heuristics to select a protocol:
  - Strong LOS-PANEL indicators:
    - `0xFD <byte>` backlight brightness.
    - `0xFE <byte>` where the next byte looks like a raw HD44780 opcode (e.g., `0x01`, `0x02`, `0x08-0x0F`, `0x40-0x7F`, `0x80-0xFF`).
  - Strong Matrix Orbital indicators (LCD Smartie 5.4.x):
    - `0xFE <cmd> <param> <param>` patterns early in the stream (structured command usage before much text).
    - Early cursor commands with small (x,y) values before text.
- Once selected, lock the protocol for the session (no flip-flopping mid-stream).
- Replay buffered bytes through the selected parser exactly once.
- Tie-breaking rule: if uncertain at timeout, prefer LOS-PANEL to protect backward compatibility.
- Session reset conditions return to `PROTO_UNKNOWN`:
  - Device reboot.
  - Serial disconnect/reconnect.
  - Extended serial inactivity timeout (target 10s; confirm final value with QA/bench).
- Treat `0xFE` as a protocol-specific introducer only after the protocol has been selected:
  - LOS-PANEL: `0xFE` prefixes a raw HD44780 opcode byte.
  - Matrix Orbital: `0xFE` prefixes a Matrix Orbital command byte (plus parameters as required).
- Provide a low-friction way to observe the selected protocol during bring-up (e.g., debug log flag, optional one-line boot banner hint).

### Dependencies
- Existing LOS-PANEL parser must remain stable and receive identical byte sequences after replay.
- Matrix Orbital parser MVP must exist as an alternative selection target.
- Host-side test harness (or at least `scripts/` replay tooling) to validate heuristics safely.

### Implementation Plan
1. Introduce a small protocol router layer in the serial RX pipeline:
   - `PROTO_UNKNOWN`: buffer + detect.
   - `PROTO_LOS_PANEL`: forward bytes directly to LOS parser.
   - `PROTO_MATRIX_ORBITAL`: forward bytes directly to MO parser.
2. Implement the buffer + replay mechanism with bounded memory use (Nano168 is a supported target).
3. Implement heuristics as a score-based decision (or strict match rules) with explicit tie-breaking (default to LOS-PANEL when ambiguous).
4. Add an inactivity watchdog to re-enter detection mode for the next session.
5. Add optional debug output guarded by compile-time flag(s) so production behavior is unchanged.

### Acceptance Criteria
- LCDproc/LOS-PANEL behavior is unchanged for existing smoke sequences and bench setups.
- LCD Smartie 5.4.x can connect and render text after protocol selection.
- Misclassification rate is effectively zero in practical use:
  - LOS-PANEL streams never route to Matrix Orbital during the initial buffer window under typical LCDproc behavior.
  - Matrix Orbital streams that start with structured commands route to Matrix Orbital reliably.
- A scripted replay can exercise both detection paths deterministically (fixtures committed under `tests/` or `scripts/`).

### Validation Notes
- Capture "first 16 bytes" traces from:
  - Known-good LCDproc session(s).
  - Known-good LCD Smartie 5.4.x session(s) configured for Matrix Orbital LCD.
- Verify detection + replay does not drop or reorder bytes (especially around `clear`, `home`, and `setCursor`).
