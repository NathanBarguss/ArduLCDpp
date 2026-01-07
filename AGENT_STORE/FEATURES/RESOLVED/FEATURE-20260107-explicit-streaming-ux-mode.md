# Goal
Make the "defer display updates while host streaming" behavior an explicit, user-visible mode that can be enabled/disabled intentionally (instead of being an implicit mitigation).

# Background
The ATmega168 can overrun its UART RX buffer during unpaced host bursts (e.g., los-panel T4/T8). The MVP mitigation that fixed this was to defer both LCD+OLED updates while the host is actively streaming, then "catch up" by refreshing dirty rows once the RX stream goes idle.

This solves data loss, but it introduces a UX tradeoff: the OLED (and now the LCD) can lag during streaming bursts and then snap to parity afterward.

We want to expose this tradeoff as an explicit UX mode so bench users can choose:
- "Low-latency parity" (best visual immediacy, may require host pacing / may drop bytes under unpaced bursts), or
- "Streaming safe" (no byte loss under bursts, displays refresh after idle).

# Requirements
- Provide a clear, explicit "streaming mode" switch that controls whether display writes are deferred during host bursts.
- Default should remain safe for the constrained Nano168 dual build (no lost bytes in T4/T8 without host pacing).
- Must work for LCD-only, OLED-only, and dual backends (no-op where not applicable).
- Must be configurable:
  - Compile-time default via build flags (`platformio.ini`) per environment, and
  - Runtime toggle via a simple host command (so test harnesses can switch modes without reflashing).
- Must not reintroduce boot loops or serial logging regressions (no verbose logging emitted during active bursts unless explicitly requested and proven safe).

# Dependencies
- Current mitigation implementation in `DualDisplay` and host-active gating in `serial_read()`.
- Test harness: `scripts/t4_with_logs.py` for T4/T8 burst verification.

# Implementation Plan
1. Add a small "streaming UX mode" enum with two states:
   - `StreamingSafe` (defer writes during bursts; refresh on idle)
   - `Immediate` (write-through to LCD/OLED as bytes arrive)
2. Add a compile-time default for each PlatformIO env (e.g., `-DSTREAMING_MODE_DEFAULT=StreamingSafe`).
3. Add a runtime control command:
   - Choose an unused opcode (or reuse an existing escape/control slot) and define:
     - `SET_STREAMING_MODE <0|1>` where 0 = Immediate, 1 = StreamingSafe.
   - Ensure the command is parsed before display data is interpreted.
4. Wire the mode into `DualDisplay` and translator paths:
   - In `StreamingSafe`, defer writes during bursts as today.
   - In `Immediate`, always write-through (no deferral); still allow OLED-only deferral if needed.
5. Add minimal UX feedback:
   - Optional one-line serial banner after idle: `mode.streaming=...` when changed.
   - Optional on-screen indicator in debug builds (e.g., corner glyph or brief text) behind a flag.
6. Validate:
   - T4/T8 with `StreamingSafe` must show full 4-row fill and `rx.bytes_total` == expected.
   - With `Immediate`, document expected behavior: may require host pacing; confirm parity when paced.

# Acceptance Criteria
- `StreamingSafe` mode:
  - T4: both displays end in 4 full rows of Z; `rx.bytes_total=84`.
  - T8: no resets; `rx.bytes_total=1024`.
- `Immediate` mode:
  - With paced host (introduce delay), both displays update in real time and end in parity.
  - With unpaced host, behavior is documented (allowed to drop tail bytes, but should not crash/reset).
- Mode can be toggled at runtime without reflashing and takes effect immediately for subsequent bytes.

# Validation Notes
- Bench device: Nano168 dual on COM6.
- Scripts: `python scripts/t4_with_logs.py --port COM6 --test t4|t8`.
- Capture photos/video of mode differences for future UX review.

## Notes (2026-01-07)
- Implemented a first cut of runtime switching via a reserved meta prefix:
  - Host sends: `FC 10 <mode>` where `<mode>` is `0` (Immediate) or `1` (StreamingSafe).
  - Compile-time default: `STREAMING_MODE_DEFAULT` (defaults to `STREAMING_MODE_SAFE`).
- When serial debug is enabled, the firmware emits an idle-gated banner after changes: `mode.streaming=safe|immediate`.

## Verification (2026-01-07)
- `StreamingSafe` mode:
  - T4 passes: both displays fill 4 rows with Z and `rx.bytes_total` matches expected.
  - T8 passes: no resets on `nano168_dual_serial`; OLED/LCD parity preserved (including custom glyph flows).
- `Immediate` mode:
  - Intended for UX comparison under paced host traffic; unpaced bursts may drop bytes (documented/acceptable), but must not reset.
  - Bench validation still required for the unpaced case (ensure “no reset” holds even if bytes drop).
