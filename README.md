# Last Minute Courier / 最后一刻快递员

Project 3: a small 3D delivery game in Unreal Engine. Theme: Time.

## Current state

Design and bilingual game copy are prepared. The Epic launcher installer is downloaded and verified. Unreal Engine, Xcode and a playable build are not yet installed/validated. Nothing here should be described as a tested game.

- [Design and production plan](Documentation/Design.zh.md)
- [Playable copy](DesignData/game_content.json)
- [Rubric and evidence checklist](Documentation/Acceptance.zh.md)

## Working arrangement

Documents and source live in this iCloud Desktop folder. Engine installation and generated caches need resident storage. iCloud synchronization is not version control. Use Git for source iterations; do not put generated engine caches or build intermediates into Git.

## First implementation milestone

Use Unreal's third-person template for movement and camera. Build a safe starting area, opening cards, package pickup, one moving cart, selective slow motion, delivery interaction and order retry. Run it in Unreal before expanding the level.

## Pending dependencies

1. Epic account sign-in and any required license acceptance by the user.
2. Verify engine/Xcode compatibility and actual storage requirements before engine download.
3. Read lecture decks 3 and 4; previous attempts timed out on cloud-backed files.
4. Confirm graduate requirements and intended release architectures.
5. Windows and Linux releases require suitable build/test environments; a Mac build alone does not satisfy the assignment.
