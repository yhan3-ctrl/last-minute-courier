#!/bin/zsh
set -euo pipefail

COURIER_PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$COURIER_PROJECT_ROOT/Scripts/local_runtime.zsh"
COURIER_ENGINE_ROOT="${1:-/Users/Shared/Epic Games/UE_5.8}"
COURIER_SCENE="${2:-GardenCorner}"
if [[ "$COURIER_SCENE" != GardenCorner && "$COURIER_SCENE" != ProjectionCorner && "$COURIER_SCENE" != Cinema ]]; then
    print -u2 "Scene must be GardenCorner, ProjectionCorner or Cinema."
    exit 2
fi
COURIER_RES_X=1280
COURIER_RES_Y=800
if [[ "$COURIER_RES_X" != <-> || "$COURIER_RES_Y" != <-> || "$COURIER_RES_X" -lt 640 || "$COURIER_RES_Y" -lt 480 ]]; then
    print -u2 'Resolution must be integers of at least 640 by 480.'
    exit 2
fi
COURIER_EDITOR="$COURIER_ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
if [[ ! -x "$COURIER_EDITOR" ]]; then
    print -u2 "Unreal Editor not found: $COURIER_EDITOR"
    exit 2
fi
if [[ ! -f "$COURIER_PROJECT_ROOT/Binaries/Mac/libUnrealEditor-LastMinuteCourier.dylib" || ! -f "$COURIER_PROJECT_ROOT/Content/Maps/L_Courier.umap" ]]; then
    print -u2 'Build the Editor module and generate the map before launching the prototype.'
    exit 2
fi
mkdir -p "$COURIER_PROJECT_ROOT/Saved/Logs" "$COURIER_PROJECT_ROOT/Runtime.nosync/DerivedDataCache"
if [[ -z "${DEVELOPER_DIR:-}" && -d /Applications/Xcode.app/Contents/Developer ]]; then
    export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
fi
exec env "UE-LocalDataCachePath=$COURIER_PROJECT_ROOT/Runtime.nosync/DerivedDataCache" \
    "$COURIER_EDITOR" "$COURIER_PROJECT_ROOT/LastMinuteCourier.uproject" \
    /Game/Maps/L_Courier -game -windowed "-ResX=$COURIER_RES_X" "-ResY=$COURIER_RES_Y" -nosplash -NoLiveCoding "-ExecCmds=Automation RunTests Courier.Visual.$COURIER_SCENE" \
    "-LocalDataCachePath=$COURIER_PROJECT_ROOT/Runtime.nosync/DerivedDataCache" \
    "-abslog=$COURIER_PROJECT_ROOT/Saved/Logs/VisualInspection.log"
