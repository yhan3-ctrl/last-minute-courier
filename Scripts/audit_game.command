#!/bin/zsh
set -euo pipefail
COURIER_PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
COURIER_ENGINE_ROOT="${1:-/Users/Shared/Epic Games/UE_5.8}"
COURIER_AUDIT_NAME="${2:-Regression}"
if [[ "$COURIER_AUDIT_NAME" == *[^a-zA-Z0-9_-]* ]]; then
    print -u2 'Audit name must contain only letters, numbers, underscores or hyphens.'
    exit 2
fi
mkdir -p "$COURIER_PROJECT_ROOT/Saved/QA/$COURIER_AUDIT_NAME"
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"
exec env "UE-LocalDataCachePath=$COURIER_PROJECT_ROOT/DerivedDataCache" \
    "$COURIER_ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor-Cmd" \
    "$COURIER_PROJECT_ROOT/LastMinuteCourier.uproject" /Game/Maps/L_Courier -game \
    -unattended -nullrhi -nosound -nosplash -NoLiveCoding \
    '-ExecCmds=Automation RunTests Courier.Regression;Quit' \
    "-ReportExportPath=$COURIER_PROJECT_ROOT/Saved/QA/$COURIER_AUDIT_NAME" \
    "-abslog=$COURIER_PROJECT_ROOT/Saved/QA/$COURIER_AUDIT_NAME/Engine.log"
