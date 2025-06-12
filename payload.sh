#!/bin/bash

# Ensure bootloader exists
if [ ! -f "bootloader" ]; then
    echo "Error: 'bootloader' not found."
    exit 1
fi

# Ensure bootchain exists
if [ ! -d "bootchain" ]; then
    echo "Error: 'bootchain' folder not found."
    exit 1
fi

# File paths
I_BSS="bootchain/iBSS"
I_BEC="bootchain/iBEC.img4"
DIAG="bootchain/diag-N94.img3"

# Check files
for f in "$I_BSS" "$I_BEC" "$DIAG"; do
    if [ ! -f "$f" ]; then
        echo "Missing file: $f"
        exit 1
    fi
done

# Get sizes
I_BSS_SIZE=$(stat -f%z "$I_BSS")
I_BEC_SIZE=$(stat -f%z "$I_BEC")
DIAG_SIZE=$(stat -f%z "$DIAG")

# Get offsets
I_BSS_OFFSET=$(stat -f%z bootloader)
I_BEC_OFFSET=$((I_BSS_OFFSET + I_BSS_SIZE))
DIAG_OFFSET=$((I_BEC_OFFSET + I_BEC_SIZE))

# Append binaries
cat "$I_BSS" >> bootloader
cat "$I_BEC" >> bootloader
cat "$DIAG" >> bootloader

# Append MAGIC and binary struct using Python
python3 <<EOF
with open("bootloader", "ab") as f:
    f.write(b"PAYLOADTABLE")
    import struct
    f.write(struct.pack("llllll",
        $I_BSS_OFFSET, $I_BSS_SIZE,
        $I_BEC_OFFSET, $I_BEC_SIZE,
        $DIAG_OFFSET, $DIAG_SIZE
    ))
EOF
