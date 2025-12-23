# AGENTS

## Interaction Protocol
- At the start of every chat, ask: "Which role should I operate as: DEV, QA, or UX?" and wait for the answer before proceeding.
- Once the role is selected, note it explicitly (e.g., "Active role: DEV") and keep that perspective until the user changes it.
- If the user doesn't respond with a valid role, prompt again with the available options.

## Workspace Bootstrap
1. Verify the `AGENT_STORE/` hierarchy exists at the repo root. This repo intentionally mimics a lightweight JIRA: the folder tree is the shared backlog that every agent can read without external services. If any folder is missing, create it using the list below so automation and humans have the same structure across machines:
   - `AGENT_STORE/README.md`
   - `AGENT_STORE/RESEARCH/README.md`
   - `AGENT_STORE/BUGS/README.md` plus `AGENT_STORE/BUGS/RESOLVED/README.md`
   - `AGENT_STORE/FEATURES/README.md` plus `AGENT_STORE/FEATURES/RESOLVED/README.md`
1. Populate each README with the purpose, naming pattern `TYPE-YYYYMMDD-short-slug.md`, and the markdown headers defined later in this document. That way, a brand-new contributor can open any README and immediately know how to author a compliant entry without rereading `AGENTS.MD`.
1. Confirm the workspace has write access so you can add or move bug, feature, or research files. The workflow requires creating new markdown artifacts and moving them into `RESOLVED/` archives; lack of write access blocks that process.
1. Sync git submodules after cloning: `git submodule update --init --recursive`. The `lcd2oled` display helper library lives in `lib/lcd2oled/` as a submodule (source: https://github.com/NathanBarguss/lcd2oled) and must be present for OLED work.
1. If you are unsure what to place inside a README, mirror the templates defined below:
   - `AGENT_STORE/README.md` should explain the overall backlog concept, list each subfolder, and describe the workflow checklist for adding and resolving items.
   - `AGENT_STORE/RESEARCH/README.md` must include the research template headers (Objective, Key Findings, Sources, Open Questions, Recommended Next Steps) so exploratory work remains actionable.
   - `AGENT_STORE/BUGS/README.md` must spell out the defect template (Summary, Environment, Steps to Reproduce, Expected/Actual Results, Fix Plan, Verification Notes) and remind agents to move closed items to `BUGS/RESOLVED/`.
   - `AGENT_STORE/FEATURES/README.md` must document the feature template (Goal, Background, Requirements, Dependencies, Implementation Plan, Acceptance Criteria, Validation Notes) and direct agents to archive finished specs under `FEATURES/RESOLVED/`.
   - Both `RESOLVED/README.md` files only need a short note explaining that they are archival stores for completed work; no additional template content is required.

## Role Glossary
- **DEV** - Implementation-focused engineer handling code, integration, and refactors.
- **QA** - Tester responsible for verification, regression checks, and quality gates.
- **UX** - Designer concentrating on user flows, visual decisions, and interaction polish.
