# Last Minute Courier / 最后一刻快递员

An Unreal Engine 5.8.4 campus delivery game about borrowed seconds and a film night.

The current v0.5.2 development version contains a safe practice yard, three connected orders, moving lifts and a shuttle, alternative paths, a three-stamp cinema ending, seven selectable languages (English by default), camera comfort controls, a lively cafe and cinema audience, three synchronized original music layers and six sound effects. A Mac arm64 Development DMG is available at Builds/Mac-v0.5.2/LastMinuteCourier-0.5.2-Mac-arm64-Final.dmg. Three Editor and three standalone engine regression suites pass. Native opening, practice instructions and retry guidance were checked; CJK font checks were completed in v0.5.1. Full packaged playthrough, other hardware and first-time player timing/feedback remain outstanding. It is not submission-ready.

Read [current implementation and checks](Documentation/Prototype.zh.md), the [game script](Documentation/GameScript.md), [rubric checklist](Documentation/Acceptance.zh.md), [test evidence](Documentation/QA-Evidence.json), and [iteration history](Documentation/ITERATIONS.md). Older observations in QA.zh.md are dated historical evidence.

From outer Project3, double-click `启动游戏.command`; it mounts the existing project-contained app image when necessary and opens the standalone game, without compiling or launching Unreal Editor. The stable editable working copy remains in `Runtime.nosync/CampusV05Work`. Inside it, `LaunchGame.command` opens the same standalone app. Four opening cards explain the story, goal, ability and controls. Practice has no delivery timer. WASD moves, mouse looks, Space jumps, holding Q slows blue clock-tagged devices, E reads a brief or delivers, Enter accepts the brief and starts its clock, C recenters, H shows route help, Esc pauses, and R retries the current unfinished order. Completed deliveries remain completed. Language is available in the opening and pause menus; sound settings are in the pause menu.

Build: `./Scripts/build_editor.command '/Users/Shared/Epic Games/UE_5.8'`.
Regression: `./Scripts/audit_game.command '/Users/Shared/Epic Games/UE_5.8' Regression`.
Asset generation: run `Scripts/create_audio.py` with Python, then `Scripts/build_presentation_assets.command` with Unreal installed. The generator replaces only named project-authored presentation assets. Engine regression deliberately controls actor locations and simulation time; it cannot establish fun, comfort or a 5–10 minute play time.

Source, documents and assets stay in Project3. Rebuildable Saved, Intermediate and DDC data stay in Project3/Runtime.nosync to avoid cloud-transfer stalls; Saved and Intermediate are relative symlinks. These are local caches, not backups. Necessary Epic/Xcode runtime data use their normal locations with the user's authorization. No unrelated personal files are in scope.

Before submission: complete genuine first-time player tests and measured pacing; verify lecture 3/4 document format and any graduate requirements; package/test Mac, Windows x86 and Linux x86 releases in suitable environments; then arrange the required GitHub, Box and Blackboard materials. Do not treat this preview as submission-ready.

See Documentation/Improvement-v05.zh.md for the latest changes and validation limits. Original Project3/.git remains preserved; active local commit history lives in the stable working copy.

Graduate status is confirmed. Documentation/GraduateAnalysis.md maps all seven course mechanics and answers three implemented balance lenses. English design PDFs are in output/pdf. Large builds are paused at the user's request because disk space is low. UE and Xcode remain installed; rebuilding still requires them.
