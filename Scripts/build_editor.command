#!/bin/zsh
set -euo pipefail

# Only run after engine/Xcode installation and license setup are complete.
COURIER_PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if [[ $# -ne 1 ]]; then
    print -u2 'Usage: build_editor.command /absolute/path/to/UE_version'
    exit 2
fi
COURIER_ENGINE_ROOT="$1"
COURIER_BUILD_SCRIPT="$COURIER_ENGINE_ROOT/Engine/Build/BatchFiles/Mac/Build.sh"
if [[ ! -f "$COURIER_BUILD_SCRIPT" ]]; then
    print -u2 "Missing Unreal build script: $COURIER_BUILD_SCRIPT"
    exit 2
fi
# Select the full toolchain for this process without changing the machine-wide xcode-select setting.
if [[ -z "${DEVELOPER_DIR:-}" && -d /Applications/Xcode.app/Contents/Developer ]]; then
    export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
fi
mkdir -p "$COURIER_PROJECT_ROOT/Saved/Logs" "$COURIER_PROJECT_ROOT/DerivedDataCache"
env "UE-LocalDataCachePath=$COURIER_PROJECT_ROOT/DerivedDataCache" \
    /bin/bash "$COURIER_BUILD_SCRIPT" LastMinuteCourierEditor Mac Development \
    "-Project=$COURIER_PROJECT_ROOT/LastMinuteCourier.uproject" \
    -WaitMutex -NoHotReloadFromIDE \
    "-log=$COURIER_PROJECT_ROOT/Saved/Logs/BuildEditor.log"
