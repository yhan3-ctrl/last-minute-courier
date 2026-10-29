# Last Minute Courier / 最后一刻快递员

Project 3: a small 3D delivery game in Unreal Engine. Theme: Time.

## Current state

Unreal Engine 5.8.4, Xcode 27.1 and Metal Toolchain are installed. The first-order prototype builds and runs on this Mac. It includes an initial campus scene, bespoke Slate menus, camera comfort controls and a language selector: English (default), Simplified Chinese, Japanese, Korean, Spanish, French and German. Agent-operated runtime checks covered the opening, tutorial, pickup, successful delivery, retry, pause and keyboard settings. This is an early playable preview, not a finished or packaged release.

- [Design and production plan](Documentation/Design.zh.md)
- [Playable copy](DesignData/game_content.json)
- [Rubric and evidence checklist](Documentation/Acceptance.zh.md)
- [Full prototype audit and remaining gaps](Documentation/QA.zh.md)
- [Prototype source status and setup](Documentation/Prototype.zh.md)
- [Visual direction and camera comfort](Documentation/VisualDirection.zh.md)
- [Third-party font credits](Documentation/ASSET_CREDITS.md)

To preview the current build on the development Mac, double-click `Scripts/play_prototype.command`. Enter advances the story, WASD moves, Space jumps, Q slows marked devices, E collects/delivers, Esc pauses, C recenters the camera, and L switches language. Camera settings are in the pause menu. Use this script so Xcode is selected for this process without changing the Mac's global developer directory.

For a smaller-window check: `./Scripts/play_prototype.command '/Users/Shared/Epic Games/UE_5.8' 1024 640`. After building, `./Scripts/audit_game.command` runs the first-order boundary regression and writes the Unreal report under `Saved/QA/Regression`. This deliberately controls actor positions and clock ticks; it does not measure level play time or replace player feedback.

## Working arrangement

Documents and source live in this iCloud Desktop folder. Engine installation and generated caches need resident storage. iCloud synchronization is not version control. Use Git for source iterations; do not put generated engine caches or build intermediates into Git.

User boundary: only operate in the game development folder. The user explicitly allows necessary Epic/Unreal/developer-tool runtime data at default locations. Keep authored game files, assets, builds and manually configured caches inside Project3; do not modify unrelated personal data.

## First implementation milestone

Compile the native third-person greybox, generate its map with Scripts/bootstrap_map.py, then verify the opening cards, practice yard, package pickup, moving carts, selective slow motion, delivery interaction and order retry in Unreal before expanding the level.

## Pending dependencies

1. Finish orders two and three, the final scene, level pacing, audio and the complete gameplay checks.
2. Read lecture decks 3 and 4; previous attempts timed out on cloud-backed files.
3. Confirm graduate requirements and intended release architectures.
4. Conduct real first-time player tests and record feedback-driven revisions. Agent-operated checks do not meet that requirement.
5. Windows and Linux releases require suitable build/test environments; a Mac Editor preview alone does not satisfy the assignment.
