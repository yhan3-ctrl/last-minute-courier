#!/bin/zsh
set -euo pipefail
COURIER_PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# Packaging is intentionally refused while this disk has little free space.
COURIER_FREE_KB=$(/bin/df -k "$COURIER_PROJECT_ROOT" | /usr/bin/awk 'NR==2 {print $4}')
if [[ "$COURIER_FREE_KB" -lt 12582912 ]]; then
    print -u2 'Packaging paused: at least 12 GiB of free disk space is required by this project wrapper.'
    exit 3
fi
source "$COURIER_PROJECT_ROOT/Scripts/local_runtime.zsh"
COURIER_ENGINE_ROOT="${1:-/Users/Shared/Epic Games/UE_5.8}"
# Xcode resolves ProjectFilesMac through the physical Intermediate symlink.
# Keep its generated ../../Build resource path connected to this project.
COURIER_INTERMEDIATE_REAL="$(cd "$COURIER_PROJECT_ROOT/Intermediate" && pwd -P)"
COURIER_BUILD_ALIAS="${COURIER_INTERMEDIATE_REAL:h}/Build"
if [[ "$COURIER_BUILD_ALIAS" != "$COURIER_PROJECT_ROOT/Build" && ! -e "$COURIER_BUILD_ALIAS" && ! -L "$COURIER_BUILD_ALIAS" ]]; then
    mkdir -p "$COURIER_PROJECT_ROOT/Build"
    ln -s "$COURIER_PROJECT_ROOT/Build" "$COURIER_BUILD_ALIAS"
fi
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"
mkdir -p "$COURIER_PROJECT_ROOT/Saved/Logs" "$COURIER_PROJECT_ROOT/Builds/Mac"
COURIER_BUILD_ARGS=(-build)
if [[ "${COURIER_REUSE_BUILT_APP:-0}" == 1 ]]; then
    if [[ ! -x "$COURIER_PROJECT_ROOT/Binaries/Mac/LastMinuteCourier.app/Contents/MacOS/LastMinuteCourier" ]]; then
        print -u2 'No previously compiled game app exists.'
        exit 2
    fi
    COURIER_BUILD_ARGS=(-skipbuild -skippackage)
fi
exec env "UE-LocalDataCachePath=$COURIER_PROJECT_ROOT/Runtime.nosync/DerivedDataCache" \
    /bin/bash "$COURIER_ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
    "-project=$COURIER_PROJECT_ROOT/LastMinuteCourier.uproject" -noP4 -unattended -utf8output \
    -platform=Mac -clientconfig=Development -ubtargs=-NoUBA "${COURIER_BUILD_ARGS[@]}" -cook -stage -pak -nodebuginfo -nocompileeditor \
    "-stagingdirectory=${COURIER_STAGE_ROOT:-$COURIER_PROJECT_ROOT/Builds/Mac}"
