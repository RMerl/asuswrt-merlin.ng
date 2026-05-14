
#!/bin/bash

# --- Configuration ---
# Updated to point specifically to the u-boot-2019.07 subfolder
BASE_REF="$HOME/gpl/asuswrt.102-39995-be96/release/src-rt-5.04behnd.4916/bootloaders/u-boot-2019.07"
COMP_SRC="$HOME/gpl/asuswrt.102-39995-be92/release/src-rt-5.04behnd.4916/bootloaders/u-boot-2019.07"

TARGET_BASE="$HOME/tempo/bootloaders"
MODEL_NAME="rt-be92u"

# The development target directory
DEV_TARGET="$HOME/dev/amng/release/src-rt-5.04behnd.4916/bootloaders/obj.$MODEL_NAME"

# --- Safety checks ---
if [ ! -d "$BASE_REF" ]; then
    echo "Error: Reference directory not found: $BASE_REF"
    exit 1
fi

if [ ! -d "$COMP_SRC" ]; then
    echo "Error: Comparison source directory not found: $COMP_SRC"
    exit 1
fi

# --- Phase 1: De-duplication ---
COMMON_DIR="$TARGET_BASE/common"
SPECIFIC_DIR="$TARGET_BASE/model.$MODEL_NAME"

mkdir -p "$COMMON_DIR"
mkdir -p "$SPECIFIC_DIR"

echo "Comparing U-Boot 2019.07 for $MODEL_NAME against reference..."

# Walk through the comparison source (be88)
find "$COMP_SRC" -type f | while read -r file_b; do
    # Calculate path relative to the u-boot-2019.07 root
    rel_path="${file_b#$COMP_SRC/}"
    file_a="$BASE_REF/$rel_path"
    
    dest_common="$COMMON_DIR/$rel_path"
    dest_specific="$SPECIFIC_DIR/$rel_path"

    if [ -f "$file_a" ]; then
        # Fast bitwise comparison
        if cmp -s "$file_a" "$file_b"; then
            # Files are identical: ensure it exists in common
            if [ ! -f "$dest_common" ]; then
                mkdir -p "$(dirname "$dest_common")"
                cp -p "$file_b" "$dest_common"
            fi
        else
            # Files differ: copy to model-specific
            mkdir -p "$(dirname "$dest_specific")"
            cp -p "$file_b" "$dest_specific"
        fi
    else
        # File is unique to this model's GPL: copy to model-specific
        mkdir -p "$(dirname "$dest_specific")"
        cp -p "$file_b" "$dest_specific"
    fi
done

# --- Phase 2: Export to Dev Tree ---
echo "Cleaning and updating dev tree: $DEV_TARGET"

# Clear existing content to ensure no stale objects remain
rm -rf "$DEV_TARGET"
mkdir -p "$DEV_TARGET"

# Copy the unique files discovered in this run
if [ -d "$SPECIFIC_DIR" ] && [ "$(ls -A "$SPECIFIC_DIR")" ]; then
    cp -a "$SPECIFIC_DIR/." "$DEV_TARGET/"
    echo "Export complete."
else
    echo "No unique files found for $MODEL_NAME (all were identical to reference)."
fi

echo "Process finished."
