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

Implementation commit: `e1dd7f1` (campus preview, seven-language UI and camera comfort controls).

This is automated technical validation and user-requested refinement, not a first-time human playtest or a claim of assignment-compliant playtest iteration.

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

Source baseline: `040b95d`. Implementation is the commit titled `Fix courier tutorial and interaction boundaries; audit first-order flow`.

This remains developer and automated technical validation, not human playtest feedback.

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

The original .git was preserved. Bulk restoration was rejected and did not execute. An independent checkout inside Project3/Runtime.nosync/CampusV05Work retains history, with 2488abe checkpointing prior staged v0.4 and this iteration saved as a normal local commit. Authored changes are mirrored into outer Project3; nosync is a local working area, not a cloud backup.


## v0.5.1 - November 1, 2026: native journey, rubric, packaged startup

- Ordinary input exposed an awning crossing the coffee camera, continuous decorative curbs at intersections and side-ramp recovery/guidance losing progress. Shortened the awning, omitted interior curb/rail seams at supported merges, saved side landings and rewound guidance with recovery. Capsule and checkpoint regressions passed. Gameplay checkpoint: **48d651a**.
- A fresh ordinary-input run completed opening, practice, all three orders and the ending. It used pause/camera lock and read-only telemetry; no fixture, position, phase or clock mutation. Evidence: Saved/QA/FullInputV05/Attempt3.log and Previews/CinemaV051-FullInput.png. Successful-order clock total 4:07; not a first-time human wall-clock duration, and not proof of every ability transfer.
- German result review exposed awkward stamp wrapping; spacing/type and separate time/retry lines were revised. Pause now clears held-key/pending movement state; native immediate pause/resume stayed still. Finale spring-arm collision is disabled only during the concluding pullback so chairs cannot collapse the view.
- Re-read Project 3.pdf all pages and lecture decks 3/4. Rewrote High Concept as current implementation, made shell/gameplay mode transitions and complete operational rules explicit, and added graduate seven-category mapping and complete answers to Reward, Punishment and Simplicity/Complexity questions. Two English PDF exports were rendered and all ten pages visually checked.
- The first Mac standalone build compiled/signed but crashed at startup. The user also reported repeated failure and supplied a crash screenshot. Crash stack points to SCourierOverlay::Rebuild during HUD BeginPlay reading empty card arrays. Guarding data readiness and retrying before caching UI state fixed the independent application's startup. **d754ed5** records the fix and finale camera change. Native independent launch then showed the English opening and Chinese/Japanese/Korean fonts. Full packaged input playthrough and other hardware remain pending; signing/build success alone was explicitly insufficient.
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


## v0.5.4 - November 7, 2026: full ordinary-input autoplay finds two blocked crossings

Earlier tools could not hold movement and Q at the same time, so no complete ordinary-input run existed. A QA-only autopilot (`-CourierAutoplay`, `Scripts/autoplay.command`) now plays at real speed with only player verbs: move toward the game's own next marker, look, jump, hold/release Q, E and Enter. It never teleports or edits state; it logs events and saves in-engine screenshots to `Saved/AutoPlay/<time>/`. Normal launches never construct it.

Run 1 (Nov 7, 23:12) completed coffee and lens, then timed out twice on the final reel: at the horizontal shuttle the courier walked into a teal rail. The rail belonged to the southern market side path, whose junction sits on the shuttle's boarding edge; the rail generator saw the empty pond beside it and fenced it. Only a player who happened to jump would pass. The same rule also fenced half of the stepping-stone takeoff. Geometry review found a second fault on the same route: the raised final lift's far edge stopped 2.25 m short of the terrace although the hint says "Walk forward to step off".

Fix: rails are never generated inside any moving platform's full travel footprint (plus margin) or along the stepping-stone line; a 2.5 m walkable lip joins the final lift's top position to the terrace. Regression adds three capsule sweeps (both shuttle halves, stepping-stone takeoff) and a landing-floor trace. All three suites pass.

Run 2 completed all three orders with 0 recoveries, 0 timeouts, 0 stuck events, 7 Q holds and 7 jumps: wall time 3:00 from first card to final stamp, clock used 12.3 / 62.8 / 63.0 s of 140 / 170 / 200 s. The autopilot reads each card for 3 s and each brief for 5 s and already knows when to cross, so this is a lower bound for an expert, not a first-time player's time; human timing remains open.

Pacing follow-up (same day): with 140/170/200 s the autoplay finished every order with 47-91% of its clock unused, so the deadline and the decision of when to spend Q carried little weight. Budgets are now 60/150/160 s (about 5x, 2.4x and 2.5x the expert time). Two route nodes were added at painted corners (lens x10500, reel x28900): the waypoint had pointed diagonally off the reel's descending ramp edge, and the autopilot had to jump down there. The lift prompt now says walk aboard (Space also works), because docked platforms are flush with the deck; the planter jump hint also appears on the final terrace. Seven-language coverage passes. Run 3: complete, 3:00 wall, 0 recoveries, 0 timeouts, clock used 12.3 / 62.8 / 63.0 of 60 / 150 / 160 s. First-time human timing is still required before these budgets can be called balanced.

Interface and intuition pass (Nov 7-8). The project owner judged the interface generic and template-like: five identical cream cards of full sentences on screen during play, an accessibility reading face whose slashed zero turned the clock into "Ø3:Ø1", and landing-page menus. The redesign keeps the game's palette but gives each surface a job. Menus are courier slips (coloured header band, paper body, perforation, key-cap buttons). The play HUD is four small dark plates anchored to the real viewport corners (a fixed 1280x800 frame had pushed them toward the centre and made the hint overlap the Q meter): order/destination with progress pips; a large clock that turns red and pulses under 20 s with a rotating compass arrow; a Q key cap and meter; a single contextual line plus an E key prompt. The always-on "follow the markers" sentence now shows only for the first seconds of an order. Delivered shows a rotated stamp; the ending is a two-part movie ticket with stamped stub. Fonts: Lilita One (display) and Nunito Bold/ExtraBold/Black (static instances made from the official variable font); CJK keeps Noto. Opening cards drop from four to three, with controls drawn as key caps.

Controls were reviewed against first instinct. E at an open brief now accepts (players pressed E again and nothing happened); Esc closes the brief; L only switches language on menu screens; R mid-delivery asks "Restart this order?" (Enter confirms, Esc keeps going) instead of silently discarding progress; pressing Q without charge flashes the meter red; a confirmed retry or timeout reopens the brief at the desk; a practice notice no longer carries into order 1. Waiting-pad footprints, two plain ovals that read as a giant face under the chase camera, are now sole-and-heel prints. The autopilot gained `-CourierUITour`, which screenshots the Chinese, Japanese and German opening, pause, help, restart confirmation, timeout, retried brief and Chinese HUD; German, the longest copy, fits after the pause buttons became a wrapping row. Seven-language coverage passes (115 strings). Regression adds checks for each control change; all three suites pass. Full autoplay: complete, 0 recoveries.


## v0.6 - November 8, 2026: first first-time playtest of the packaged game

The project owner played the packaged v0.5.4 build from the launcher, without coaching, and stopped during the first delivery. Observations, with screenshots:

1. After crossing the training cart they kept following the orange line past the parcel desk to the coffee stand, never holding an order. Cause: reaching the desk only switched to a waiting state, the orange route already pointed onward to the coffee stand, and nothing stopped the walk.
2. "None of the carts move." Cause: carts and lifts only ran during their own delivery, so everything past practice stood still until an order was accepted.
3. "Pressing E at the counter does nothing." Two causes: the counter they reached was the coffee stand, the destination; and at the parcel desk the counter itself sat 3.4 m from its interaction point.
4. Turning the camera at the stand put the view inside the barista: people and props had no collision, so the camera probe passed through them.
5. They asked for Q to show how many times it can be used per level.

Changes. Walking up to a desk opens its brief; after closing it, stepping away and back (or E) reopens it. A striped yard gate after the desk stays closed until the first order is accepted, with a sign saying so; walking around it returns the courier to the desk with that message. While a parcel waits, only the path to the desk is shown. Carts, lifts and the shuttle run whenever the courier can move, and collide everywhere. The parcel counter moved within reach and the interaction radius is 3 m. E away from a target explains where to go. People have solid bodies and large props stop the camera probe (thin posts and floor markings do not). Q now has 3/4/5 uses per delivery (unlimited in practice) with a 1.5 s recharge; the count and pips sit beside the Q key, the brief lists "Q uses", an empty count turns red and explains gaps and side paths, and retrying restores the uses. The second opening card and the route hints state the counts; the last route has six devices but five uses, making use a choice.

The investigation also found a defect present since v0.5: the next-waypoint marker's cone kept full collision after its rotation, an invisible obstacle just above head height at every marker. It is now disabled at actor level and tested.

Verification: all three suites pass with new checks (desk counter opens the brief, a closed brief does not reopen in place, the gate returns a skipper with a reason, carts move in practice, E explains itself, three uses then refusal with message, short recharge, retry restores uses, the camera stops at a person, the sky never blocks the camera, marker parts never collide). The packaged app passes the suites and a complete real-speed autoplay with the screenshot tour (0 recoveries). Editor rendering was unavailable because an Xcode update to 27.2 removed its Metal Toolchain; packaged runs do not need it. The owner's next full playthrough is the real check of these changes.


## v0.7 - November 8, 2026: reasons to play well and to play again

After v0.6 the project owner judged the game still less fun than commercial games. Diagnosis against that comparison: finishing on time was the only goal, so nothing said whether a run was good; the risky short line and the safe long line paid the same, so route choice carried no weight; holding Q, the core verb, had almost no audiovisual feel; and hits, the final seconds and deliveries had little feedback.

Changes. Golden clocks: 16 floating clocks (1 practice, then 4/5/6 per route) each add 5 seconds to the running order. Most sit on the precise line: in a cart lane, between paired lanes, at the top of each lift, over the shuttle's water, on a planter and on a stepping stone, so taking an ability's harder use now earns time. The practice clock teaches them before any clock runs. Stars: each delivery is rated 1-3 by time left (at least 45% / 20% / on time); the delivered card shows stars, time left and clocks collected, and the ending ticket shows stars per stamp, a total out of 9 and a rank, giving a finished run a reason to be replayed. Feel: holding Q grades the camera cool blue with a vignette and a faint cyan wash and drops the music's pitch; a cart hit or fall flashes red with a low thud; the last ten seconds tick; deliveries burst into paper confetti, larger at the finale; the clock panel flashes gold on a pickup. A finale lamp post that blocked the pulled-back ending camera was removed. All of this uses engine shapes and the existing sounds, so the cooked content and packaging path are unchanged.

Rubric review at the same time: Windows and Linux releases cannot be built with Unreal on macOS. `Documentation/BuildOtherPlatforms.md` and `Scripts/package_windows.bat` / `Scripts/package_linux.bat` give a one-command build on a Windows PC; those releases are still to be produced.

Verification: new checks (practice clock exists, every route has clocks, +5 s once per clock, retry restores clocks, star thresholds) and all earlier checks pass in all three suites; the packaged app passes them too and completes a real-speed scripted playthrough collecting 11 of 15 delivery clocks for 9/9 stars, with screenshots of the slow-time grade, the rated delivery card and the finale ticket. Whether these changes make the game more fun is for the owner's next playthrough to say.


## v0.8 - November 8, 2026: make the world make sense, and give each parcel its own rule

The project owner found v0.7 still monotonous and said many things did not match common sense; while playing they also asked why, at the lifts, the orange line led somewhere other than where they had to go. Reviewing the game as a first-time player found these causes:

- Carts swung back and forth on a short track like pendulums. They are now traffic: each lane has a depot shed at both ends, and carts drive out of one, across and into the other, one way. Q slows a cart's clock, so it waits in its depot longer.
- A cart hit faded to black and teleported the courier. It now knocks them aside with a 0.7 s stagger.
- "Serve warm" and "Handle with care" were only labels, and all three deliveries played the same way. The coffee now cools (the HUD shows its warmth and the stars measure it); the lens case shows three condition marks, each cart knock or fall cracks one, and the third fails the order; the reel is rated by time before the screening.
- A golden clock adding 5 seconds to a film's start time made no sense. It is now a Borrower cell: a spare battery for the Q device worth one more use.
- Leaving the area or falling hit an invisible reset. Hedges now mark the campus edge, and every reset names its cause (pond, dropped off a walkway, outside this delivery's area).
- Nobody explained why the lens waited at the coffee stand or the reel at the booth. The briefs now say who left each parcel there.
- The orange paint covered every walkway, so at the lift it ran off along the side ramp while the marker pointed to the lift; nothing was painted across the lift itself; and on the main ramp down the marker jumped to the side garden walk. Now only the recommended route is painted, chevrons ride on each lift and on the shuttle, and the marker follows a side path only when the courier is clearly on it.

Placing depot sheds also showed that one shed would have stood on the garden ramp's landing; those lanes moved, and capsule sweeps now check that no shed blocks a side path.

Verification: all three suites pass, with new checks for one-way traffic, the knock-back, Borrower cells, lens knocks and failure, per-parcel stars, depot clearance, the painted line staying off the side ramp, the marker staying on the line on the main ramp, and every painted point lying on walkable ground. The packaged app passes them and completes a real-speed scripted playthrough that now follows the painted line across the stepping stones (four jumps), with 0 recoveries.

## v0.9 - November 8, 2026: parkour movement, readable rewards and a consistency pass

The project owner asked for one more full check of v0.8, then reported three things from watching people play: the Borrower cell "looked strange and not like a reward; people were even afraid to touch it", the game was still boring, and it was too easy to finish. They suggested taking cues from parkour games because those match players' instincts. Causes found:

- The reward was the same blue as the hazards. The whole game teaches "blue clock-tagged things are dangerous", so a blue battery floating at head height read as a hazard. It is now a spinning gold hourglass with a warm glow that drifts toward the courier when they come close and pops "+1 borrowed time".
- Most of the playing time was waiting (for carts, for lifts) and the only verbs were walk and jump. Parkour games feel good because input answers at once and there are several ways to move. The courier now sprints (Shift), slides (Ctrl), climbs any ledge a jump meets, and gets forgiving jump timing (a short grace after leaving an edge, and a jump pressed just before landing).
- Q had one use: making something slower to wait less. Two new blue devices combine it with the new moves. Rolling shutters close on a cycle: slide under one while it is coming down, wait for it to reopen, or hold Q so the slide fits. Turnstiles have a low arm to jump and a high arm to slide under; Q slows them enough to read. A springboard (not a Q device) launches a sprinting courier onto the final terrace as an alternative to the last lift, and crate walls and a crate staircase to a market roof use the climb.
- Rewards were rare: 16 pickups in the whole game. Lines of gold sparks now run along each recommended line (and along the roof and the springboard's flight), with a rising chime for a run of pickups; 25 sparks give one more Q use. Close calls (sliding under a closing shutter, clearing a turnstile arm, passing just behind a cart) pop "Close call!" and add sparks, so bold play is rewarded instead of waiting.
- The practice yard gained a slide lesson: a striped beam from hedge to hedge, too low to walk under and too tall to hop, so the lesson cannot be skipped. Sprinting is taught on the first marker; climbing on the first crate wall, with a hint.

The full re-check of v0.8 also found leftovers from the old pendulum carts and old timing. Four hints still said to cross "when the cart moves aside"; one-way traffic is crossed after the cart has passed, so they say that in all seven languages. 42 unused strings were removed from the translation table, some stating rules that no longer exist. The design data listed 1.0 s of hit protection where the game uses 1.3 s plus a 0.7 s stagger. The Game Script still described a cart hit as a return to a checkpoint. On buttons, the key hint drew over the end of the caption at the 1280x800 UI scale; the key hint now leads the caption, and the UI fonts render unhinted. The automated UI tour never reached the lens order's own failure screen; it now captures it.

Building the automated playthrough for the new devices showed that a turnstile cannot be crossed by waiting alone (its two arms always cross a straight path somewhere), which is the intended lesson. The playthrough driver now holds Q, jumps the low arm and slides under the high one, as the hint says. It also showed that the first practice beam could be hopped onto; it was made taller.

Verification: a new engine suite (Courier.Regression.Parkour) runs real character frames for the beam (standing is blocked, a slide passes and halves the capsule), the crate climb, the shutter (shut is solid and harmless, open lets you through, closing knocks a standing courier back while a sliding one fits), both turnstile arms, the springboard, spark counting and reset, and checks every spark floats over standing ground. All four suites pass in the editor and in the app inside the v0.9 DMG. The packaged real-speed scripted playthrough completes with 0 recoveries and 0 stuck points in 2:22 (orders 8.0 / 42.4 / 56.3 s), using 4 slides, 2 climbs and 7 Q uses; with spark rewards at one use per 15 it ended the run holding 10 uses, so the rate became one per 25. Time limits went from 60/150/160 to 40/120/130 s because sprinting shortened every route. The expert time is a lower bound; a first-time human run of v0.9 is still needed.

## v0.10 - November 8, 2026: the owner's v0.9 playthrough

The project owner played v0.9 and reported: the turnstile spun so fast they could not get past it; it should be designed so it has to be passed (not walked around); and they were not sure the new keys did anything, on Mac or (later) Windows. Causes and changes:

- The turnstiles turned once every 3.4-4.6 s, so an arm tip moved at about 7 m/s: too fast to read and react to. They now turn once every 10 s (coffee), 8.5 s (garden) and 7.5 s (reel), about 2.5x slower; Q still slows them a further 4x. The hit zones are gentler: the low arm only counts below 62 cm (it spans 35-75 cm), the high arm was raised to 120-160 cm and only counts above 128 cm, and the side tolerance shrank from 48 to 40 cm.
- Every turnstile now stands in a wall that runs from its arms to the hedges, like a ticket gate. The coffee arcade and the reel's side path were re-routed to join before their gates, so every route passes through one. A regression check sweeps a body around each gate and must be blocked.
- Keys: the game had bound Command as a slide key, but on a Mac Command+Q quits the game, and Ctrl+Space switches the input source (which can make later keys seem dead). Command is no longer bound; C is the main slide key and Ctrl still works; a slide pressed in mid-air now fires on landing instead of being ignored; camera recentering moved from C to V. A key plate in the corner of the HUD lights each key while its move is active, so every press is visibly answered. The keys are bound by key code in the game itself, so they behave the same on the Windows build.
- Also found while reviewing screenshots: on the upper deck after a lift the hint still said to board the lift (the check ignored height), and the gold "Close call!" pop-up was hard to read against the pale sky (it now has a dark outline). The lens brief and star rule now say that any hit or fall knocks the case, not only carts. The coffee limit went from 40 to 45 s because the gate is now unavoidable.

Verification: all four engine suites pass, with new checks that each turnstile is walled in and turns no faster than once per 7 s. The packaged real-speed scripted playthrough completes with 0 recoveries, crossing all three gates (two slides under the high arm, one jump over the low arm) and earning 3/3/3 stars. Whether the slower turnstile now feels fair to a first-time player is for the owner's next playthrough.


## v0.10.1 - November 8, 2026: camera visibility and submission documents

A final packaged screenshot review found that the projection-booth shutter could cover the chase camera after delivery 2 and through the next brief. Its visible moving face now blocks the camera only and ignores Pawn; the independent courier blocker still controls passage. A regression trace checks both collision responses. The normal-input packaged journey was rerun from the first card to the final ticket; all three orders completed with no recovery, timeout, retry or stuck event. Four regression suites pass in the fixed Editor build and the final packaged candidate.

The comfort description now acknowledges the existing sprint/slide field-of-view effect in all seven languages. Stale 0.5.3 application metadata is corrected to 0.10.1. All 159 runtime strings have six optional translations and required font/license files; opening card 1 was visually checked in all seven languages and restored to English.

The owner confirmed four players with similar v0.9/v0.10 feedback and immediate feedback-driven revisions. PlaytestFeedback-Nov08.md records the aggregate reconstruction without inventing individual times, quotations or completion counts. The course submission now has an independent High Concept, an independent Game Script including graduate analysis, and a concise report. All use Arial, a plain academic layout and Yu Han authorship. This directly addresses the previous instructor comment about missing separate design documents and too many persistent text boxes.

Remote v0.1-v0.10 tags and release metadata were checked against local history. The new revision and local Mac DMG are prepared separately; the published v0.10 release is preserved. Detailed technical evidence is in DeliveryAudit-Nov08.md. Publication and course uploads remain separate actions.
