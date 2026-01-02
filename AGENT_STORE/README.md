# AGENT_STORE Backlog

The `AGENT_STORE/` tree is our lightweight shared backlog so every role can add specs, bugs, and research notes without external tools. Keep this directory readable and versioned so work stays transparent.

## Structure
- `RESEARCH/` — discovery notes and experiments. Closed studies move to `RESEARCH/RESOLVED/`.
- `BUGS/` — open defects written as reproducible tickets. Fixed issues move to `BUGS/RESOLVED/`.
- `FEATURES/` — feature briefs and specs. Finished specs move to `FEATURES/RESOLVED/`.

## File naming
Create new entries using `TYPE-YYYYMMDD-short-slug.md` (e.g., `BUG-20251223-flicker.md`). Pick a descriptive slug so tickets are easy to search.

## Workflow checklist
1. Capture the request in the correct subfolder using the template noted below.
2. Keep the entry updated as work progresses (notes, owners, links, commits).
3. When the work is complete and verified, move the file into the matching `RESOLVED/` archive.
4. Never delete history; archives provide traceability for future audits.
