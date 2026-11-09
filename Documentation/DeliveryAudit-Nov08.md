# Delivery audit - November 8, 2026

## Assignment and evidence

Project 3.pdf: all three pages read and visually checked. Graduate status confirmed. The owner approved a concise rubric-aligned report and separate High Concept / Game Script PDFs, then requested Arial and the supplied academic paper's plain layout. Formal documents focus on design and actual feedback; internal measurements remain here.

The owner reports four v0.9/v0.10 players with similar concerns and immediate feedback-driven revisions. Individual durations, quotations and completion counts are not recorded. PlaytestFeedback-Nov08.md reconstructs only the aggregate feedback-to-change trace.

## Rules and language audit

Orders 45/120/130 s; Q starts 3/4/5 uses; at most 3 real seconds, devices at 0.25 speed, 1.5 s recharge. The courier and deadline do not slow. Early release still spends the use. One practice hourglass and 4/5/7 delivery hourglasses; each +1 Q, every 25 sparks +1 Q. Turnstile periods 10/8.5/7.5 s; all gates compulsory. C primary slide, Ctrl alternate, V recenter.

159 runtime strings and six optional translations pass the coverage/font/license validator. Native opening card 1 was checked in en/zh/ja/ko/es/fr/de using keyboard language cycling, then restored to English. This verifies opening rendering/layout, not every translated screen or native-language quality. A separate controlled menu tour visits en/zh/de, not Japanese as older notes claimed.

## Runtime checks and fix

Original v0.10 packaged ordinary-input journey: 143.0 s total; orders 8.1/42.4/56.3 s; zero recoveries/timeouts/retries/stuck events; 9/9 stars. The driver knows the route, so this is an expert technical lower bound rather than human pacing. The controlled UI tour directly forces a timeout and three lens knocks to inspect failure screens; exclude its timing/failures from normal play evidence.

Screenshots found the projection-booth shutter could fill the chase camera with blue after delivery 2. v0.10.1 gives its visible face camera-only collision, with Pawn ignored; the separate Pawn blocker remains unchanged. A regression trace checks that distinction. The corrected candidate completed the same ordinary-input journey, again 143.0 s, zero recoveries/timeouts/retries/stuck events and 9/9 stars. Screenshots 017-019 now show the booth, route and readable overlays.

Four suites pass in the fixed Editor build and final candidate: FirstOrderBoundaries, Parkour, PlatformRide, ThreeOrderJourney. Tests use controlled states and real engine frames; they do not measure human learning or comfort. Test-event errors/warnings: zero. Ordinary engine startup diagnostics are retained in raw logs.

Comfort text in all seven languages now acknowledges the existing sprint/slide field-of-view change. No fixed nine-degree maximum is claimed. DefaultGame.ini and Info.plist version metadata change from stale 0.5.3 to 0.10.1.

Evidence: Saved/QA/HandoffNov08 (builds, ordinary journey, controlled tour, screenshots, packaged regression and remote metadata). Fixed Editor report: Saved/QA/HandoffNov08Fixed/index.json. Final candidate: PackagedFinalRegression.log; FixedPlaythrough.log and FixedPlaythrough/events.log.

## Source control and release

Remote main matched 0456b1b47cfae48564dc2e077427e23657fcb415. All fourteen annotated tag refs/target commits match local history and iteration headings. Six releases (v0.5.4 and v0.6-v0.10) contain Mac DMG/checksum assets. RemoteTags.json, RemoteReleases.json, RemoteMain.json and RemoteCommits.jsonl retain the read-only checks. Git authors/committers are Yu Han only.

Published v0.10 DMG: 515586900 bytes, SHA-256 3f9d0d90277eac065aaf44855339ffff7153b1a72d09e648adfad7954178b46c, matching the remote asset. Existing published tags/assets are preserved. v0.10.1 is a new local revision; no public push/release was made during this audit.

Windows: user says a build already exists and asks to verify it later. Linux/Box contents were not independently checked. Teacher invitation and Blackboard upload remain author actions; no invitation or message was sent. Do not reuse the Project 2 links visible in the instructor-feedback screenshot.

## Remaining acceptance observations

First-time full duration, final fun, independent learning transfer, comfort/listening and native-language quality require actual player observations; modifications cannot reconstruct measurements. Teacher-hardware checks and submission receipts require the real packages/destinations. No guarantee of full marks is made.

Xcode 27.2 lacks the Editor Metal toolchain; NullRHI Editor tests and packaged Metal rendering work. No component downloads or cleanup/deletion were undertaken. Prior builds and Unreal are preserved. Disk free space was checked before the targeted compile.

## Final local distribution and documents

v0.10.1 DMG: 515723065 bytes; SHA-256 fe1ddfa6fc747fd6f9046746d52954178b063d6ce5973917e8e5e89bffee7abf. hdiutil checksum is valid. The read-only mounted app passes strict/deep signature verification, reports version 0.10.1 and matches the tested launcher app's executable and two design-data hashes. All four mounted regression suites succeeded; process exit 0 (DMGRegression.log). The launcher points to the corrected app; v0.10 is retained in ReleaseMac/PreviousV010.

Three independent PDFs: High Concept 2 pages; Game Script and Graduate Analysis 8 pages; Report 10 pages. All final pages were rendered and visually inspected. Actual glyph fonts are embedded ArialMT and Arial-BoldMT; author/creator Yu Han. Formal text contains no tool branding or internal expert-duration discussion. FinalPDFManifest.json records the output hashes. Format follows the owner's supplied plain academic reference with the explicitly requested Arial font.

## Authorized publication

The owner authorized publishing v0.10.1 to the existing public repository on November 8, 2026, including code, the new tag, Mac DMG/checksum and all three PDFs. The report and repository links were synchronized to the new release before publishing. The existing published tags and assets are preserved. Remote verification records are retained in Saved/QA/HandoffNov08/Publication.
