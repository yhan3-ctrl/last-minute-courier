# Last Minute Courier / 最后一刻快递员

An Unreal Engine 5.8.4 campus delivery game about borrowed seconds and a film night.

The current development version contains a safe practice yard, three connected orders, moving lifts and a shuttle, alternative paths, a three-stamp cinema ending, seven selectable languages (English by default), camera comfort controls, original music and sound effects. This is a Mac Editor preview. It is not a packaged course submission; first-time player timing and feedback remain outstanding.

Read [current implementation and checks](Documentation/Prototype.zh.md), the [game script](Documentation/GameScript.md), [rubric checklist](Documentation/Acceptance.zh.md), [test evidence](Documentation/QA-Evidence.json), and [iteration history](Documentation/ITERATIONS.md). Older observations in QA.zh.md are dated historical evidence.

Double-click `Scripts/play_prototype.command`. Four opening cards explain the story, goal, ability and controls. Practice has no delivery timer. WASD moves, mouse looks, Space jumps, holding Q slows blue clock-tagged devices, E reads a brief or delivers, Enter accepts the brief and starts its clock, C recenters, H shows route help, Esc pauses, and R retries the current unfinished order. Completed deliveries remain completed. Language is available in the opening and pause menus; sound settings are in the pause menu.

Build: `./Scripts/build_editor.command '/Users/Shared/Epic Games/UE_5.8'`.
Regression: `./Scripts/audit_game.command '/Users/Shared/Epic Games/UE_5.8' Regression`.
Asset generation: run `Scripts/create_audio.py` with Python, then `Scripts/build_presentation_assets.command` with Unreal installed. The generator replaces only named project-authored presentation assets. Engine regression deliberately controls actor locations and simulation time; it cannot establish fun, comfort or a 5–10 minute play time.

Source, documents and assets stay in Project3. Rebuildable Saved, Intermediate and DDC data stay in Project3/Runtime.nosync to avoid cloud-transfer stalls; Saved and Intermediate are relative symlinks. These are local caches, not backups. Necessary Epic/Xcode runtime data use their normal locations with the user's authorization. No unrelated personal files are in scope.

Before submission: complete genuine first-time player tests and measured pacing; verify lecture 3/4 document format and any graduate requirements; package/test Mac, Windows x86 and Linux x86 releases in suitable environments; then arrange the required GitHub, Box and Blackboard materials. Do not treat this preview as submission-ready.
