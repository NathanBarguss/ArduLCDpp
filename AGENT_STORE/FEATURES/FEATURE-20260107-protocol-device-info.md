# Goal
Enable PC-side tools to query ArduLCDpp over the existing serial link and receive a compact, machine-parseable identity payload (firmware/protocol version + key build/runtime configuration) so benches can auto-detect board capabilities and avoid "wrong env / wrong geometry" confusion.

# Background
ArduLCDpp currently behaves like a write-only lcdproc `los-panel` target: the host streams `0xFE` commands, `0xFD` backlight bytes, and raw data. During bring-up (especially across HD44780/OLED/dual builds and Nano168 vs Nano328 variants), our tooling has no reliable way to confirm what firmware is actually running once the serial port resets the board. This slows diagnosis and increases the risk of false failures when the host assumes the wrong backend, geometry, or baud.

# Requirements
- Backward compatible with lcdproc `los-panel` traffic (no reinterpretation of existing bytes).
- Opt-in: the board only emits metadata in response to an explicit request (no unsolicited chatter during normal lcdproc sessions).
- Response must be stable, small, and parseable (single-line ASCII recommended; <= 128 bytes ideal).
- Response must include, at minimum:
  - `proto` (protocol feature version for this metadata reply)
  - `fw` (firmware version string) and/or `sha` (git short hash)
  - `backend` (`HD44780`, `OLED`, `DUAL`)
  - `geom` (`<LCDW>x<LCDH>`)
  - `mcu` (e.g., `atmega168`, `atmega328p`)
  - `baud` (current serial baud rate)
- Unknown/unsupported meta-subcommands must return a clear error reply (and not affect display state).

# Dependencies
- A defined "meta command" prefix byte that is not used by lcdproc `los-panel` (proposal below).
- A source of version strings in firmware (manual `ARDULCDPP_VERSION`, PlatformIO-injected build metadata, or a git hash via build flags/scripts).
- Documentation updates in `README.md` and `docs/lcdproc_display_mapping.md` once implemented.

# Implementation Plan
1. Reserve a new escape prefix for ArduLCDpp metadata messages (proposal: `0xFC`).
2. Define initial request/response:
   - Host sends: `FC 01` (`GET_INFO`).
   - Device replies (single line, newline-terminated):
     - `@ARDULCDPP proto=1 fw=... sha=... env=... backend=DUAL geom=20x4 mcu=atmega168 baud=57600\n`
   - If `fw/sha/env` are unavailable, return the keys with `unknown` values rather than omitting them.
3. (Optional future) Add `FC 02` (`GET_CAPS`) returning a small bitmask or key list for feature flags (CGRAM supported, dual parity, OLED translator version, etc.).
4. Add a tiny host-side probe snippet (Python/pyserial) to `scripts/` or docs to demonstrate the flow: open port, wait 2–3 seconds for bootloader reset, send `FC 01`, read one line.
5. Add a new smoke test case (T9) to `docs/display_smoke_tests.md`: "Identify probe".

# Acceptance Criteria
- After flashing any supported environment, a host can send `FC 01` and receive a single-line identity reply within 50–200 ms (excluding the standard 2–3 s post-open reset wait).
- Reply includes the required keys and correctly reflects the running build configuration.
- Sending `FC 01` during display-heavy bursts does not deadlock or corrupt display output; worst case, the reply may be delayed until the firmware returns to its main loop.
- Unknown subcommand (e.g., `FC FF`) yields an error reply (e.g., `@ARDULCDPP err=unknown_cmd\n`) and the display continues unaffected.

# Validation Notes
- Manual: use a pyserial one-liner to request `FC 01` and log the response alongside photos of T1–T4 results for the same firmware.
- Regression: confirm lcdproc sessions remain clean (no unsolicited metadata) and that the metadata request does not introduce a new "blackout / lock-up" vector in dual/OLED builds.
