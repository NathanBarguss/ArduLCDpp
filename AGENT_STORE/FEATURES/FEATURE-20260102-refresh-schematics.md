# Goal
Redraw the wiring schematic/PNG in `resources/` so they match the 2026 Nano-to-HD44780 pin map (RS=D12, E=D2, data bus on D3-D10, PWM backlight on D11) captured in `AGENTS.MD` and the README.

# Background
Firmware and docs now assume the refreshed pinout, but the only visual references (`wiring_schematic.sch` / `.png`) still show the legacy wiring. This causes confusion for hardware bring-up and for remote agents that rely on the published diagrams.

# Requirements
- Update `resources/wiring_schematic.sch` (and regenerate the PNG) to mirror the latest wiring table.
- Include the BC337 backlight circuit on D11 exactly as implemented on the bench prototype.
- Call out RW tied low and note the serial/power connectors exactly as built.

# Dependencies
- Finalized pin map lives in `AGENTS.MD` and README ("Current Nano ↔ HD44780 wiring" section).
- Any KiCad/gschem symbols used in the legacy file should remain available or be replaced with equivalent open-format symbols.

# Implementation Plan
1. Open the existing schematic source and adjust the net connections/pin labels to the new mapping.
2. Export an updated PNG (and optionally PDF) into `resources/` so non-EDA users can view it.
3. Verify references in README/AGENTS point to the refreshed diagrams (update wording if necessary).

# Acceptance Criteria
- Both the schematic source and PNG reflect the 2026 wiring with no stale labels.
- README no longer needs the warning about outdated diagrams once this is merged.

# Validation Notes
- Cross-check each pin against the `AGENTS.MD` wiring table before exporting.
- If the tools used are different from the original (e.g., KiCad vs gschem), document the new workflow in the resources folder.
