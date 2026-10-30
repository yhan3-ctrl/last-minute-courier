# Rebuildable development files remain inside Project3, outside iCloud transfer.
# Preserve existing generated files; never merge or discard a user's directories.
mkdir -p "$COURIER_PROJECT_ROOT/Runtime.nosync"
for COURIER_GENERATED_NAME in Intermediate Saved; do
    COURIER_GENERATED_PATH="$COURIER_PROJECT_ROOT/$COURIER_GENERATED_NAME"
    COURIER_LOCAL_PATH="$COURIER_PROJECT_ROOT/Runtime.nosync/$COURIER_GENERATED_NAME"
    if [[ -e "$COURIER_GENERATED_PATH" && ! -L "$COURIER_GENERATED_PATH" ]]; then
        if [[ -e "$COURIER_LOCAL_PATH" ]]; then
            print -u2 "Two generated folders exist; resolve them before running: $COURIER_GENERATED_NAME"
            return 2
        fi
        mv "$COURIER_GENERATED_PATH" "$COURIER_LOCAL_PATH"
    fi
    mkdir -p "$COURIER_LOCAL_PATH"
    if [[ ! -e "$COURIER_GENERATED_PATH" && ! -L "$COURIER_GENERATED_PATH" ]]; then
        ln -s "Runtime.nosync/$COURIER_GENERATED_NAME" "$COURIER_GENERATED_PATH"
    fi
done
