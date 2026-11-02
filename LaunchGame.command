#!/bin/zsh
set -euo pipefail
COURIER_LAUNCH_ROOT="$(cd "$(dirname "${0:A}")" && pwd)"
# The completed local app is self-contained; no Editor or automatic rebuild.
if [[ -f "$COURIER_LAUNCH_ROOT/Runtime.nosync/CourierBuildMac.sparseimage" ]]; then
    COURIER_RUNTIME="$COURIER_LAUNCH_ROOT/Runtime.nosync"
else
    COURIER_RUNTIME="${COURIER_LAUNCH_ROOT:h}"
fi
COURIER_IMAGE="$COURIER_RUNTIME/CourierBuildMac.sparseimage"
COURIER_MOUNT="$COURIER_RUNTIME/CourierBuildVolume"
COURIER_APP="$COURIER_MOUNT/ReleaseMac/Mac/LastMinuteCourier.app"
if [[ ! -x "$COURIER_APP/Contents/MacOS/LastMinuteCourier" ]]; then
    if [[ ! -f "$COURIER_IMAGE" ]]; then
        print -u2 'The local standalone game image is missing. No build was started.'
        exit 2
    fi
    mkdir -p "$COURIER_MOUNT"
    /usr/bin/hdiutil attach -nobrowse -mountpoint "$COURIER_MOUNT" "$COURIER_IMAGE"
fi
exec /usr/bin/open "$COURIER_APP"
