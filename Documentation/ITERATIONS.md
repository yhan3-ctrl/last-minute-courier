# Iterations

## v0.1 — Design baseline — 2026-10-29

This is a planning baseline, not a playtested build.

The user selected the campus courier concept and explicitly requested opening cards that explain the story. The design now connects the three orders to one campus film screening and specifies three bilingual opening cards, a safe practice yard, timed deliveries, selective slow motion, failure recovery and a final scene.

The initial technical inspection found no Unreal Engine or full Xcode installation. Disk cleanup was completed with user authorization. Epic launcher setup is in progress. Existing lecture decks could not be read successfully; their document structures and graduate lenses remain unverified.

### Evidence status

- Implemented gameplay: none verified yet. Native C++ source for the first-order greybox is prepared, but has not been compiled or run.
- First-time player feedback: none yet.
- Completed feedback-driven revisions: none yet.
- Timing and fun claims: targets only, not measured results.

Future entries must identify the tested build, observed problem, decision, implementation commit and follow-up result. Feature additions alone do not satisfy the assignment's iteration requirement.

## First-order source preparation — 2026-10-29

Added a native Unreal project descriptor, module/targets, input and rendering configuration, bilingual opening cards, third-person greybox character, practice state, selective cart slowdown, first-order pickup/delivery, retries and HUD. Added an editor map creation script and a local build wrapper. This is implementation preparation, not a feedback-driven iteration. No runtime or playtest results are claimed.

The user restricted file operations to the game development folder, then explicitly allowed necessary Epic/Unreal/developer-tool runtime data at default locations. The user created an Epic account and signed in. Native UI automation can observe the launcher but clicks currently fail with noWindowsAvailable; engine selection requires resolving this UI issue or a user click.
