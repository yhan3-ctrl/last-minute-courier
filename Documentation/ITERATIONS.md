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


## v0.5 — Clear actions and a livelier campus — 2026-10-30

The user requested understandable, human-logical play and authorized implementation. Coffee traffic teaches a single gap before a paired crossing, with contextual wait/Q/ride/release instructions. The lesson, goal and hints share one side card; ability numbers have their own column. ARRIVED takes precedence over a backward direction when E can interact. The next delivery opens the untimed brief directly at the same counter.

Original NPCs, a striped cafe, roof cup landmark, steaming service cups, flower arches and cinema audience support visible consequences. Two original synchronized 48-second music layers fade in with progress. Recenter follows the shortest angle, accepts mouse interruption and resolves immediately under pause. The finale uses a wider 2.4-second camera transition and a side ticket; a fresh run restores the regular camera.

Actual input found a jump prompt/acceptance mismatch, a backward lesson target, clipped Q numbers, misleading arrival guidance and chair-heavy ending framing. These led to final corrections. CampusV05 first failed the paused-camera expectation; subsequent Final, Aligned, Arrival and Finale reports pass all three suites. The final report has zero errors/warnings and eight capsule routes. Coverage checks pass 101 strings across six optional languages. The nine named original audio assets imported with zero errors/warnings.

Native verification is partial: fresh opening/tutorial input, controlled Coffee start then walking/E delivery/Enter next brief, and controlled Cinema start then aisle walk/E ending. These do not establish a complete playthrough or 5–10 minute timing. Human playtests, comfort/listening assessment, native-speaker review and platform release packages remain outstanding.

The original .git was preserved. Bulk restoration was rejected and did not execute. An independent checkout inside Project3/Runtime.nosync/CampusV05Work retains history, with 72e02a2 checkpointing prior staged v0.4 and this iteration saved as a normal local commit. Authored changes are mirrored into outer Project3; nosync is a local working area, not a cloud backup.


## v0.5.1 - November 1, 2026: native journey, rubric, packaged startup

- Ordinary input exposed an awning crossing the coffee camera, continuous decorative curbs at intersections and side-ramp recovery/guidance losing progress. Shortened the awning, omitted interior curb/rail seams at supported merges, saved side landings and rewound guidance with recovery. Capsule and checkpoint regressions passed. Gameplay checkpoint: **b2578df**.
- A fresh ordinary-input run completed opening, practice, all three orders and the ending. It used pause/camera lock and read-only telemetry; no fixture, position, phase or clock mutation. Evidence: Saved/QA/FullInputV05/Attempt3.log and Previews/CinemaV051-FullInput.png. Successful-order clock total 4:07; not a first-time human wall-clock duration, and not proof of every ability transfer.
- German result review exposed awkward stamp wrapping; spacing/type and separate time/retry lines were revised. Pause now clears held-key/pending movement state; native immediate pause/resume stayed still. Finale spring-arm collision is disabled only during the concluding pullback so chairs cannot collapse the view.
- Re-read Project 3.pdf all pages and lecture decks 3/4. Rewrote High Concept as current implementation, made shell/gameplay mode transitions and complete operational rules explicit, and added graduate seven-category mapping and complete answers to Reward, Punishment and Simplicity/Complexity questions. Two English PDF exports were rendered and all ten pages visually checked.
- The first Mac standalone build compiled/signed but crashed at startup. The user also reported repeated failure and supplied a crash screenshot. Crash stack points to SCourierOverlay::Rebuild during HUD BeginPlay reading empty card arrays. Guarding data readiness and retrying before caching UI state fixed the independent application's startup. **fd018a2** records the fix and finale camera change. Native independent launch then showed the English opening and Chinese/Japanese/Korean fonts. Full packaged input playthrough and other hardware remain pending; signing/build success alone was explicitly insufficient.
- Build evidence: final engine regression RubricV051Final has 3 successful suites, 0 failed, 0 errors/warnings (before the final UI-only readiness guard). Editor and Game modules compiled after that guard, Xcode app assembly succeeded, and strict/deep codesign passed. Several earlier packaging attempts failed from iCloud metadata, path resolution and temporary capacity; those are not counted as successful release tests.
- Disk became critically low. User chose to keep Unreal and pause large builds. Removed only generated project duplicates/caches, compacted the project sparse image, and uninstalled explicitly authorized unused Godot/Unity Hub and Unity Hub templates. Kept all project source/content/documents and current UE/Xcode. Launcher opens the existing standalone app without building; wrappers refuse Editor builds below 8 GiB and packaging below 12 GiB free. These thresholds are project safeguards, not measured engine minimum requirements.
- Human first-time learning/fun/comfort/listening evidence, first-time 5-10 minutes, Windows/Linux packages, Mac full packaged journey and GitHub/Box/Blackboard delivery remain open. Local commits and a history bundle are not GitHub pushes.


## v0.5.2 - November 4, 2026: actual Q crossing and staged lift guidance

Standalone review found that a short Q tap before the practice lane satisfied the old flag even when the courier crossed after releasing it. Practice now records Q active at the approach and far edge of the lane, clears unfinished entry on release, recovery or walking outside the lane, and preserves a correctly completed crossing after release. The untimed lesson explicitly explains holding Q through the lane, release after crossing and recharge. This is a response to an observed technical playtest issue, not invented human feedback.

Lift prompts now explicitly teach waiting for a raised lift to come down with Q released, using Q plus Space to board, releasing Q while riding and walking forward at the far landing. Contextual boarding instructions stay visible while slowdown is active. English remains the default and all new strings include the six optional languages.

Nov04-Onboarding passes FirstOrderBoundaries, PlatformRide and ThreeOrderJourney with zero test errors/warnings. New checks cover a short tap followed by normal crossing, early release, a side bypass, recovery, valid held crossing, release after clearing, boarding while Q is active, a raised lift and landing prompts. The standalone executable runs the same three suites successfully and exits 0. Its sandbox blocked JSON/abslog export to the project; process stdout and a parsed summary preserve this limit. These tests use controlled positions and engine frames, not a complete ordinary-input or human playthrough.

Editor and Game modules compiled. The ordinary Xcode generated app shell failed signing. Compiling in Unreal's Xcode mode, reusing unchanged cooked assets, staging the new program and two runtime JSON files into a copy of the previous complete app, clearing copied Finder metadata and freeing temporary APFS build space produced a strictly verified local signature. Binary UUID 35141D10-4E64-3D42-8A5F-4910E72F1535. v0.5.1 remains as a rollback. The existing launcher now targets v0.5.2. A compressed Mac DMG includes the app, controls/limits, credits and both design PDFs.

Native independent opening, four-card navigation, actual jump, updated Q lesson layout and the specific retry instruction after bypassing the lesson were observed. The tool cannot hold Q and W simultaneously; actual held-key crossing and a full normal-input packaged journey still need a person. The report PDFs were updated, rendered and all ten pages visually inspected. No first-time human playtest, Windows/Linux release, GitHub push/invitation or Box/Blackboard submission is claimed.

Distribution verification: hdiutil checksum, mounted read-only app strict/deep signature and actual English opening from that image passed. SHA-256 and the exact scope are stored in BuildStatus.json. This still does not establish cross-Mac compatibility or a full ordinary-input playthrough.

Final wording follow-up: visual comparison found the new recharge hint referenced READY although the actual HUD displays an ability bar. All seven languages now refer to a full bar. Editor/Game rebuilt, final standalone regression again passed all three suites, and the Final DMG has a verified checksum/signature. The earlier mounted DMG remains untouched while available for user play. Final binary UUID D5573103-C998-3A4B-8F89-BFBD5D4DF15D.


## v0.5.3 - November 5, 2026: accept natural timing and make routes visible

Actual partial-practice user feedback contradicted the forced-Q lesson: the natural strategy was to wait for a cart to move aside and cross quickly, yet the lesson respawned safe crossings. The user also found the route unclear. The x1900 teaching reset is removed. With/without Q, early release and a natural gap all progress normally; crossing saves a forward checkpoint. Actual contact is still checked and the recovery text explicitly names a cart hit. These are observed user-reported practice problems, not a fabricated full playthrough or duration.

Sparse tiny dashes are replaced with a continuous orange line, broad outlined directional arrows about every five metres, a stationary next-waypoint ring/downward pointer and a larger direction HUD. Only the active practice/order group is visible; alternative walkways remain marked. The guide uses instanced meshes, no collision, no flashing/bobbing and no camera movement. It cannot create new invisible walls. The existing eight capsule sweeps still pass.

Nov05-LogicalCrossing passes all three suites with zero test errors/warnings. New cases accept a safe crossing with no Q and early release, distinguish a real hit from a visible gap and check current-route visibility and non-collision. The signed standalone candidate also passes all three suites with no missing-object/package load errors in captured stdout. An ordinary-keyboard Editor preview crossed the lane without Q, showing Safe crossing, the next lesson and a full 3.0-second ability charge. The new line, arrows, marker and HUD were visually inspected. The tool then reported the user changed the playtest window; stopped sending input.

Versioned rules, concept, graduate Punishment/Simplicity analyses and partial-feedback record were updated. Both PDF exports were rendered and all ten pages visually inspected. Editor and Game modules compiled; unchanged cooked assets were reused. Candidate binary UUID D4071D65-5B65-3B14-B0B3-8743EA9BD13F, local signature strict/deep verification passed. Windows/Linux, a full normal-input standalone run, complete human pacing/comfort/listening and online submissions remain open.

Distribution: compressed v0.5.3 DMG checksum and strict/deep app signature after read-only mounting pass. Three read-only-image suites pass, exit 0. The final Read Me clarifies that no Q does not cause reset while the low step still teaches jumping. App program/runtime data hashes match the tested image. Current launcher path now contains v0.5.3; no active user window was closed or relaunched during publishing.
