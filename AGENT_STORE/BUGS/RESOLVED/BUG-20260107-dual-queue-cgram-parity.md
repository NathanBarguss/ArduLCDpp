# Summary
In dual-display builds with burst-safe queueing enabled (`ENABLE_DUAL_QUEUE=1`), custom character (CGRAM) updates are not mirrored to the OLED while queueing is active. This causes OLED custom glyphs (slots `0..7`) to remain stale/blank during normal LCDproc sessions, making the OLED appear "off" right when bargraphs/icons are rendered.

# Environment
- Firmware branch: `feature/FEATURE-20251223-oled-backend`
- Hardware: Bench Nano ATmega168 with HD44780 + SSD1306 connected (COM6, 57,600 8-N-1)
- PlatformIO env: `nano168_dual_serial` (queueing enabled) vs `nano168_oled` (single OLED)
- Host tooling:
  - `scripts/t5_custom_chars.py` (CGRAM programming + glyph render)
  - `scripts/t4_with_logs.py` (sanity: burst-safe streaming still passes)

# Steps to Reproduce
1. Flash `nano168_dual_serial` to a Nano168 with both LCD + OLED connected.
2. Run T5 (custom characters) with paced settings:
   - `python scripts/t5_custom_chars.py --port COM6 --delay 6 --backlight 255 --slot-delay-ms 20 --chunk-delay-ms 150 --after-clear-ms 8 --hold 12`
3. Observe the LCD and OLED during/after the run.

# Expected Results
- LCD and OLED both show the same 8 custom glyphs on row 0 and row 1 (bytes `0x00..0x07`), as they do in OLED-only mode.

# Actual Results
- LCD shows the expected custom glyphs.
- OLED shows missing/blank glyphs after CGRAM programming begins (often appearing "off" if the content is mostly glyph bytes).
- T4/T8 remain stable in dual mode (burst-safe queueing still works), and OLED can display normal ASCII/fills once refreshed, but custom glyph parity is broken.

# Root Cause Hypothesis
When `ENABLE_DUAL_QUEUE` is enabled and `queue_enabled_ == true`, `DualDisplay::createChar()` does not forward `createChar(slot, bitmap)` to the secondary display. CGRAM updates therefore never reach the OLED during host-active sessions (which is exactly when LCDproc programs glyphs for dashboards).

# Fix Plan
Do not disable queueing during CGRAM programming (that risks reintroducing UART overruns). Instead, make CGRAM updates queue-aware:

1. **Cache + dirty-mask CGRAM updates while queueing is active**
   - In `DualDisplay::createChar(slot, bitmap)`:
     - Always apply to the primary (LCD) immediately.
     - If `queue_enabled_` is true, copy `bitmap[8]` into a small cache `cgram_shadow_[8][8]` and set a dirty bit in `cgram_dirty_mask_`.
     - If `queue_enabled_` is false, keep current behavior (mirror to OLED immediately).

2. **Flush dirty CGRAM slots during idle servicing**
   - In `DualDisplay::pumpSecondary()` (or a helper it calls):
     - Before row refresh, flush a bounded number of dirty CGRAM slots to the OLED:
       - `secondary_.createChar(slot, cgram_shadow_[slot])`
     - Clear the dirty bit(s) for any slots flushed.
   - Keep the same idle/quiet gating used for row refresh so we avoid blocking UART during bursts.

3. **Validation**
   - Dual (`nano168_dual_serial`):
     - Run `scripts/t5_custom_chars.py` and confirm OLED catches up with correct glyphs once RX goes idle.
     - Re-run T4/T8 via `scripts/t4_with_logs.py` to confirm no regressions in the burst-safe guarantee.
   - OLED-only (`nano168_oled`): confirm unchanged behavior.

# Verification Notes
- This is a parity/UX issue, not a stability issue: it should be fixed without changing the burst-safe queueing policy.
- The CGRAM cache cost is small (64 bytes for 8 slots x 8 rows) and should fit within the Nano168 SRAM budget, especially after the recent TX/TWI trims.

## Resolution (2026-01-07)
- Fix landed: `DualDisplay` now caches CGRAM slot updates while queueing is active and flushes them to the OLED during idle (before row refresh), restoring glyph parity without reintroducing UART overruns.
- Implementation: commit `0725b83` on `feature/FEATURE-20251223-oled-backend`.

## Verification (Bench)
- T5 parity (slots 0..7) confirmed on all configurations using the paced probe:
  - `python scripts/t5_custom_chars.py --port COM6 --delay 6 --backlight 255 --slot-delay-ms 20 --chunk-delay-ms 150 --after-clear-ms 8 --hold 12`
  - `nano168_hd44780`: LCD renders 8 custom glyphs on rows 0 and 1.
  - `nano168_oled`: OLED renders the same glyphs.
  - `nano168_dual_serial`: LCD + OLED render the same glyphs; OLED may catch up after the idle window.
- Sanity: T8 still passes on all three configurations after the fix (no dropped bytes / no resets).
