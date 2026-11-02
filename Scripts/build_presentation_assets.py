"""Build only this project's original atmosphere and audio assets."""
from pathlib import Path
import runpy
import unreal
scripts = Path(unreal.Paths.project_dir()).resolve() / "Scripts"
runpy.run_path(str(scripts / "build_atmosphere.py"), run_name="__main__")
runpy.run_path(str(scripts / "create_audio.py"), run_name="__main__")
runpy.run_path(str(scripts / "create_music_layers.py"), run_name="__main__")
runpy.run_path(str(scripts / "import_audio.py"), run_name="__main__")
