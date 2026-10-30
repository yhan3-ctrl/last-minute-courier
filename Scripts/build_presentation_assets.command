#!/bin/zsh
set -euo pipefail
COURIER_PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$COURIER_PROJECT_ROOT/Scripts/local_runtime.zsh"
COURIER_ENGINE_ROOT="${1:-/Users/Shared/Epic Games/UE_5.8}"
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"
exec env "UE-LocalDataCachePath=$COURIER_PROJECT_ROOT/Runtime.nosync/DerivedDataCache" \
    "$COURIER_ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor-Cmd" \
    "$COURIER_PROJECT_ROOT/LastMinuteCourier.uproject" \
    -run=pythonscript "-script=$COURIER_PROJECT_ROOT/Scripts/build_presentation_assets.py" \
    -unattended -nullrhi -nosound -nosplash -NoLiveCoding \
    "-abslog=$COURIER_PROJECT_ROOT/Saved/Logs/PresentationAssets.log"
