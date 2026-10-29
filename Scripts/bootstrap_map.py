"""Run inside Unreal Editor after the C++ module compiles. Never overwrites a map."""
from pathlib import Path

import unreal

project_dir = Path(unreal.Paths.project_dir()).resolve()
map_file = project_dir / "Content" / "Maps" / "L_Courier.umap"
if map_file.exists():
    unreal.log("Courier map already exists; preserving it.")
else:
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.new_level("/Game/Maps/L_Courier"):
        raise RuntimeError("Could not create /Game/Maps/L_Courier")
    start = actors.spawn_actor_from_class(
        unreal.PlayerStart, unreal.Vector(100, 0, 120), unreal.Rotator(0, 0, 0)
    )
    if start is None:
        raise RuntimeError("Could not create player start")
    start.set_actor_label("Courier Start")
    if not levels.save_current_level():
        raise RuntimeError("Could not save the courier map")
    unreal.log("Courier map created. Play to generate the first-order greybox.")
