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


## v0.3 — First-order boundary audit — 2026-10-29

Source baseline: `f97bdbf`. Implementation is the commit titled `Fix courier tutorial and interaction boundaries; audit first-order flow`.

This remains developer/agent technical validation, not human playtest feedback.

- Wrote and ran a native Unreal regression against the real map, game mode, character and carts. Before fixing behavior it reported five failures: jump-lesson bypass, distant-Q lesson credit, underground pickup, airborne pickup and airborne delivery.
- Required an actual jump near the lesson, and Q use near the training cart. Shared grounded/height/distance validation between interaction hints and interaction execution. The same suite then passed; a final run including paused/locked camera recenter also passed with zero errors and zero test warnings.
- Checked normal-clock versus quarter-speed device updates, charge/cooldown, pause freeze, releasing Q during pause, checkpoint protection, timeout, retry resets, grounded success and success-clock freeze through controlled locations and explicit engine ticks. This does not certify real input combinations, physics, pacing or performance.
- Added contextual recovery messages, cleared residual motion on checkpoint restore, labeled pickup-ready state correctly, made menu buttons keyboard-focusable, added a Quit button, and made each language-menu row use its own regional font. Runtime copy validation now covers 54 strings and six optional translations.
- Added explicit C / menu recenter after observing how a turned-away camera could be locked in an unhelpful direction. Recenter points along this preview's +X route; future curved routes must replace that assumption. Added dark world-sign backgrounds and fitted the gameplay HUD to a design canvas.
- In the actual 1024×640 window, checked opening → movement/jump/Q tutorials → E pickup → pause/resume → cart collision/recovery → successful E delivery → R retry. During a substantial pause, displayed time remained approximately 02:04 and resumed at 02:03. Menu controls and HUD fit within the window. This is evidence of this test run only.
- Also let the full 140-second countdown expire naturally in the graphical build, observed the failure menu, and pressed R to restore pickup-ready state with no running clock. The encouraging failure heading could still state the timeout cause more clearly.
- Keyboard focus and Space on Quit closed the prior graphically running preview normally; log confirmed shutdown. Pointer automation remains blocked by noWindowsAvailable, so mouse-click testing is still open.
- Preserved sanitized before/after/final report summaries in `Documentation/QA-Evidence.json`, detailed findings and remaining scope in `Documentation/QA.zh.md`, and updated the stale rubric checklist. Raw logs/reports remain ignored in Saved.

Open: full three-order level, new uses for all abilities, 5–10 minute first-time-player timing, human comfort and feedback-driven revisions, performance, all-language full playthroughs, standalone platform releases and assignment publication. Local iCloud cache reads sometimes time out; no global storage settings were changed.

## v0.4 — Three orders, clear onboarding and atmosphere — 2026-10-29

The user requested stronger gameplay, explicit starting instructions, a prettier sky, enjoyable music and comfortable 3D presentation. The first linear delivery has become three connected deliveries with a garden, lifts, stepping stones, a horizontal shuttle, alternative routes and a cinema reveal. Briefs can be read before accepting and starting the clock; completed orders stay completed on retry. Four focused opening cards and untimed practice explain the full goal and every input. English remains the startup default with six optional languages.

Geometry inspection and controlled engine regression identified ramp lips and side-route navigation pointing across ponds. Level junctions now leave level runouts; navigation follows the side route when the courier is on it. Ground is walkable outside pond cutouts; recovery detaches any platform base, clears pending input, and freezes play for its short fade. A real engine-frame platform test checks riding and slowdown without launching the courier. Instanced terrain uses a supported material and movable lighting; the visible default grid and unbuilt-lighting warning prompted these corrections.

Original oscillator/noise audio supplies a 48-second cue and six sound effects, with a pause-menu mute switch. A gradient sunset, stationary soft clouds, sun, haze and distant hills replace the flat sky. Initial Metal rendering exposed an unconnected Saturate input despite successful null-RHI asset generation; the material generator was corrected to check every expression connection.

Evidence: JourneyPhysicsV04 and PresentationV04 each pass all three engine suites with zero test errors or warnings. PresentationV04 includes four-card opening navigation. Localization coverage passes 95 active strings across six optional languages, with fonts/licenses present. These are technical checks using controlled locations and simulated time, not genuine first-time player feedback. Complete graphical review and measured 5–10 minute pacing remain independent acceptance work.

Build caches and intermediate data were preserved under Project3/Runtime.nosync to prevent iCloud transfer stalls. Source and assets remain in the original project. UnrealBuildTool runs with NoUBA after its accelerated writes failed through the generated-directory symlink. No global developer-directory change was made.

Graphical follow-up found a separate collision problem: independent continuous guardrails crossed the garden ramp at its turn and merge. At the observed capsule position near (14446, 1634, 410), both sideways and forward movement were obstructed. Guardrails are now assembled as instances after the finished walkways are evaluated; segments over connected walkable surfaces are omitted. Six capsule sweeps cover garden, projection and final-terrace corners/merges. Developer-only Courier.Visual scenarios provide explicit controlled inspection starts; they must never be counted as complete input playthroughs.

Final follow-up: FinalJourneyV04 passes all three suites with zero test errors/warnings, including six capsule routes. Diagnostic hits at the ramp-to-flat transition were walkable floor normals (0.982–0.986 Z), not guardrails; the fixture now uses CharacterMovement's walkability threshold instead of rejecting all floor contact. Native W/A movement from a controlled start traversed the previously blocked garden corner and merge. The cinema ticket was rendered from a controlled developer start, not a full-level playthrough. Actual English/Chinese/German fourth-card layouts, English practice and first-order arcade delivery, brief-before-clock behavior, music checkbox off/on, and Play Again were checked. Screenshot captions distinguish these scopes.
