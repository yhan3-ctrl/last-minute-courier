#!/bin/zsh
set -euo pipefail

COURIER_PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
COURIER_ENGINE_ROOT="${1:-/Users/Shared/Epic Games/UE_5.8}"
COURIER_EDITOR="$COURIER_ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
if [[ ! -x "$COURIER_EDITOR" ]]; then
    print -u2 "Unreal Editor not found: $COURIER_EDITOR"
    exit 2
fi
if [[ ! -f "$COURIER_PROJECT_ROOT/Binaries/Mac/libUnrealEditor-LastMinuteCourier.dylib" || ! -f "$COURIER_PROJECT_ROOT/Content/Maps/L_Courier.umap" ]]; then
    print -u2 'Build the Editor module and generate the map before launching the prototype.'
    exit 2
fi
mkdir -p "$COURIER_PROJECT_ROOT/Saved/Logs" "$COURIER_PROJECT_ROOT/DerivedDataCache"
if [[ -z "${DEVELOPER_DIR:-}" && -d /Applications/Xcode.app/Contents/Developer ]]; then
    export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
fi
exec env "UE-LocalDataCachePath=$COURIER_PROJECT_ROOT/DerivedDataCache" \
    "$COURIER_EDITOR" "$COURIER_PROJECT_ROOT/LastMinuteCourier.uproject" \
    /Game/Maps/L_Courier -game -windowed -ResX=1280 -ResY=800 -nosplash -NoLiveCoding \
    "-LocalDataCachePath=$COURIER_PROJECT_ROOT/DerivedDataCache" \
    "-abslog=$COURIER_PROJECT_ROOT/Saved/Logs/FirstPlayable.log"
