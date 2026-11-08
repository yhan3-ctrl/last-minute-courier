#!/bin/zsh
# QA only: plays one full run at real speed with the in-game autopilot and
# writes events.log plus screenshots to Saved/AutoPlay/<timestamp>/.
set -euo pipefail
COURIER_PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$COURIER_PROJECT_ROOT/Scripts/local_runtime.zsh"
COURIER_ENGINE_ROOT="${1:-/Users/Shared/Epic Games/UE_5.8}"
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"
exec env "UE-LocalDataCachePath=$COURIER_PROJECT_ROOT/Runtime.nosync/DerivedDataCache" \
    "$COURIER_ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor" \
    "$COURIER_PROJECT_ROOT/LastMinuteCourier.uproject" /Game/Maps/L_Courier -game \
    -CourierAutoplay -CourierTelemetry ${COURIER_AUTOPLAY_EXTRA:-} -windowed -ResX=1280 -ResY=800 -nosplash -NoLiveCoding -nosound \
    "-LocalDataCachePath=$COURIER_PROJECT_ROOT/Runtime.nosync/DerivedDataCache" \
    "-abslog=$COURIER_PROJECT_ROOT/Saved/Logs/AutoPlay.log"
