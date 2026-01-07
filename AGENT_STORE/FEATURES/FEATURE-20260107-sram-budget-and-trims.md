# Goal
Create a measured, low-risk SRAM reduction plan for the Nano168 dual + serial build so we have predictable headroom (target: ≥160 bytes free after boot banner) without regressing the T4/T8 “no dropped bytes” guarantee.

# Background
The current `nano168_dual_serial` MVP is stable and passes unpaced T4/T8, but SRAM is very tight:
- PlatformIO reports `Data: 900 bytes (87.9% Full)` for ATmega168 (1 KB SRAM).
- Runtime probes during T4/T8 have shown ~`119–125` bytes free after the boot banner.

We need a cleanup pass to reclaim SRAM so future features and instrumentation don’t push us back into unstable territory.

# Requirements
- Maintain behavior: T4 and T8 must still PASS with `rx.bytes_total` matching expected (`84` and `1024`).
- Maintain stability: no boot loops/resets with serial debug enabled.
- Improve headroom: `free_sram.after_banner >= 160` on `nano168_dual_serial`.
- Keep changes isolated and reversible via build flags where possible.
- Make the SRAM budget and knobs discoverable/documented for future contributors.

# Dependencies
- Current MVP mitigation branch: `feature/FEATURE-20251223-oled-backend`.
- Test harness: `scripts/t4_with_logs.py` for T4/T8.
- Submodule: `lib/lcd2oled` now supports `LCD2OLED_ENABLE_TEXT_BUFFER` for SRAM reclamation.

# Implementation Plan
1. **Baseline measurement (no functional changes)**
   - Record PlatformIO size output for `nano168_dual_serial` and `nano168_dual`.
   - Record runtime `free_sram.after_banner` during T4 and T8.
   - Capture top SRAM consumers using `avr-nm --size-sort` on `firmware.elf`.

2. **Low-risk compile-time wins (apply one at a time)**
   - Reduce UART TX buffer: `-DSERIAL_TX_BUFFER_SIZE=32` (keep RX unchanged initially).
   - Reduce Wire/TWI buffers: `-DTWI_BUFFER_LENGTH=16` (saves ~48 bytes across 3 buffers).
   - Rebuild + re-run T4 after each change; proceed only if PASS.

3. **Medium-risk optional trims (only if needed)**
   - Make HD44780 CGRAM cache optional behind a flag (saves 64 bytes) if the product does not rely on custom glyphs.
   - Reduce/disable verbose debug counters/structs in non-debug builds (ensure no behavior change in `nano168_dual`).

4. **Verification**
   - Run T4 and T8 on COM6 and confirm:
     - `rx.bytes_total=84` for T4 and `rx.bytes_total=1024` for T8
     - `free_sram.after_banner >= 160`
     - No resets and displays still end in parity

5. **Documentation**
   - Add a short “SRAM budget” section to the project docs (or AGENT_STORE) listing:
     - Current baseline numbers
     - Target headroom
     - Available build flags and their SRAM deltas
     - Any UX/behavior tradeoffs of each trim

# Acceptance Criteria
- `nano168_dual_serial`:
  - `free_sram.after_banner >= 160`
  - T4 PASS with `rx.bytes_total=84`
  - T8 PASS with `rx.bytes_total=1024`
- `nano168_dual` still builds and behaves normally (no serial debug required).
- Documentation exists and is easy to follow for a new contributor.

# Validation Notes
- Bench device: Nano168 dual on COM6.
- Commands:
  - `pio run -e nano168_dual_serial -t size`
  - `python scripts/t4_with_logs.py --port COM6 --test t4`
  - `python scripts/t4_with_logs.py --port COM6 --test t8`
