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

## v0.2 — First playable visual preview — 2026-10-29

Implementation commit: `7eb78c6` (campus preview, seven-language UI and camera comfort controls).

This is agent-operated technical validation and user-requested refinement, not a first-time human playtest or a claim of assignment-compliant playtest iteration.

### Implemented and checked

- Installed UE 5.8.4, Xcode 27.1 and Metal Toolchain with the user's authorization. The Editor module builds successfully; the map and two project-owned materials were generated through Unreal Python.
- Replaced Canvas text with native Slate menus and bundled licensed fonts. English remains the default on every fresh run.
- At the user's request, added one language dropdown containing English, Simplified Chinese, Japanese, Korean, Spanish, French and German. Verified opening dropdown selection with keyboard, and visually inspected Japanese, Korean, Spanish, French and German opening pages plus Chinese/German pause pages. The 46 current runtime strings have six complete optional translations; a validation script detects missing copy, fonts and licenses. No native-speaker review is claimed.
- Built an initial campus scene with buildings, trees, benches, a parcel depot, cafe pavilion, clocktower, courier and clock-marked carts.
- Added adjustable FOV/sensitivity and optional camera-direction lock. No head bob, camera shake, speed zoom, camera roll or motion blur. Verified slider adjustment and camera-lock checkbox with keyboard. Comfort still requires human feedback.
- Ran opening → practice movement/jump → Q/recharge → E pickup → timed route → E successful delivery → R retry to pickup. Collisions triggered checkpoint recovery and the route remained completable. This short route does not prove the required 5–10 minute play time.

### Problems observed and responses

- Default engine material did not provide the intended project color control: created a matte material with an explicit Color parameter.
- Large central world labels and dark background obscured readability: moved labels aside, added a light sky and broader campus landmarks; gave the cafe sign a light panel.
- Card header/counter wrapping and truncated energy hints: used explicit text widths and shorter localized HUD labels.
- Native Mac captures intermittently omitted static UI text while engine-exported screenshots showed complete labels. Added unified HUD repainting and simpler text rendering for single-line labels; retain real-player visual verification as an open check.
- Retry while paused during practice could skip the lesson: disabled that action and guarded the gameplay state.

### Still unverified or unfinished

Human playtests and feedback-driven revisions; camera comfort; frame-time stability; timeout and pause edge cases; pointer clicks (native automation reports noWindowsAvailable); smaller window layouts; complete per-language playthroughs; orders two/three, full level pacing, audio and final ending; standalone Mac/Windows/Linux releases; lecture-specific document structures and graduate requirements.
