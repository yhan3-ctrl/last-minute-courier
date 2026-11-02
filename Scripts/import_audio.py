"""Run in Unreal Editor to import/reimport only the project's own named original audio."""
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve()
unreal.EditorAssetLibrary.make_directory("/Game/Audio")
tasks = []
for source in sorted((root / "SourceAssets/Audio").glob("*.wav")):
    destination = "/Game/Audio/" + source.stem
    if source.stem not in {"CampusLoop","Pickup","Paper","Stamp","Borrow","Finale","Footstep","CampusStrings","CampusBells"}:
        continue
    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = "/Game/Audio"
    task.destination_name = source.stem
    task.automated = True
    task.replace_existing = True
    task.save = True
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    if not task.imported_object_paths:
        raise RuntimeError("Audio import failed: " + task.filename)
for name in ("CampusLoop", "CampusStrings", "CampusBells"):
    track = unreal.EditorAssetLibrary.load_asset("/Game/Audio/" + name)
    if not track:
        raise RuntimeError("Missing music loop: " + name)
    track.set_editor_property("looping", True)
    track.set_editor_property("virtualization_mode", unreal.VirtualizationMode.PLAY_WHEN_SILENT)
    unreal.EditorAssetLibrary.save_loaded_asset(track)
unreal.log("Courier original audio imported with synchronized silent playback configured.")
