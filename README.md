# Last Minute Courier / 最后一刻快递员

Project 3: a small 3D delivery game in Unreal Engine. Theme: Time.

## Current state

Design, bilingual game copy and the native Unreal C++ first-order prototype source are prepared. Epic launcher is installed, updated and signed in by the user. Unreal Engine, Xcode and a playable build are not yet installed/validated. Nothing here should be described as a tested game.

- [Design and production plan](Documentation/Design.zh.md)
- [Playable copy](DesignData/game_content.json)
- [Rubric and evidence checklist](Documentation/Acceptance.zh.md)
- [Prototype source status and setup](Documentation/Prototype.zh.md)

## Working arrangement

Documents and source live in this iCloud Desktop folder. Engine installation and generated caches need resident storage. iCloud synchronization is not version control. Use Git for source iterations; do not put generated engine caches or build intermediates into Git.

User boundary: only operate in the game development folder. The user explicitly allows necessary Epic/Unreal/developer-tool runtime data at default locations. Keep authored game files, assets, builds and manually configured caches inside Project3; do not modify unrelated personal data.

## First implementation milestone

Compile the native third-person greybox, generate its map with Scripts/bootstrap_map.py, then verify the opening cards, practice yard, package pickup, moving carts, selective slow motion, delivery interaction and order retry in Unreal before expanding the level.

## Pending dependencies

1. Any required Unreal/Xcode license acceptance by the user; Epic sign-in is complete.
2. Verify engine/Xcode compatibility and actual storage requirements before engine download.
3. Read lecture decks 3 and 4; previous attempts timed out on cloud-backed files.
4. Confirm graduate requirements and intended release architectures.
5. Windows and Linux releases require suitable build/test environments; a Mac build alone does not satisfy the assignment.
