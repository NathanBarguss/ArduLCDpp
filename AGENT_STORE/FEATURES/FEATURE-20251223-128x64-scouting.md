# Goal
Plan and document the follow-on work required to add SSD1306 128x64 OLED support (emulating 16x8 or 20x4 layouts) once the 128x32 MVP ships.

# Background
Stakeholders want a path to taller OLED modules, but this is explicitly a later phase. Capturing the constraints now prevents architectural rework when we revisit the backlog.

# Requirements
- Define the desired logical geometry for 128x64 (e.g., 20x4 or 16x8) and note how it affects LCDproc configuration.
- Evaluate whether lcd2oled needs additional patches (page addressing, clear() length, buffer sizing) for 64-pixel panels.
- Outline any PlatformIO/Arduino memory impacts of larger buffers.
- Mark the ticket as "deprioritized / post-MVP" so it does not block the 128x32 release.

# Dependencies
- Completion of the 16x4 OLED MVP (command translator, CGRAM shim, docs, smoke tests).

# Implementation Plan
1. Research 128x64 SSD1306 wiring/initialization differences and record them in a short design note.
2. Prototype lcd2oled with a 64-pixel panel to validate clear(), setCursor(), and performance characteristics.
3. Update backlog tickets (geometry, docs, smoke tests) with variants covering 128x64 once confidence is high.

# Acceptance Criteria
- Written analysis exists outlining geometry, firmware changes, and risks for 128x64 panels.
- Ticket is labeled/communicated as "low priority" so planning boards know it is not part of the current milestone.

# Validation Notes
- When reprioritized, rerun the smoke-test matrix expanded for the taller layout.
