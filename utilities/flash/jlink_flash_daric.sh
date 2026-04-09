#!/bin/bash
# ==============================================================================
# JLink Firmware Download Script for Daric NTO under Linux-like environment
# ==============================================================================
# Description: Downloads firmware to Daric NTO chip via JLink V7.92
# Author: robin.wan@crossbar-inc.com
# ==============================================================================

set -e  # Exit on error

# Configuration
if [ -z "$JLINK_EXE" ]; then
    if command -v JLink.exe >/dev/null 2>&1; then
        JLINK_EXE="JLink.exe"
    elif command -v JLinkExe >/dev/null 2>&1; then
        JLINK_EXE="JLinkExe"
    else
        JLINK_EXE="/c/Program Files/SEGGER/JLink_V792h/JLink.exe"
    fi
fi

PROJ_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEVICE_CONFIG_DIR="${PROJ_DIR}"
JLINK_DEVICES_XML="${DEVICE_CONFIG_DIR}/JLinkDevices.xml"
DEVICE="DARIC_NTO"
DEFAULT_BIN="${PROJ_DIR}/bin/BootROM.bin"
DEFAULT_FLASH_ADDR="0x60000000"
JLINK_SCRIPT="/tmp/jlink_flash_$$.jlink"

# Parse command line arguments
BIN_FILE="$DEFAULT_BIN"
FLASH_ADDR="$DEFAULT_FLASH_ADDR"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -b) BIN_FILE="$2"; shift 2 ;;
        -a) FLASH_ADDR="$2"; shift 2 ;;
        -j) JLINK_EXE="$2"; shift 2 ;;
        0x*) FLASH_ADDR="$1"; shift ;;
        *.bin) BIN_FILE="$1"; shift ;;
        *) echo "Unknown argument: $1"; exit 1 ;;
    esac
done

# Smart defaulting for uart_demo address
if [[ "$BIN_FILE" == *"uart_demo"* ]] && [[ "$FLASH_ADDR" == "0x60000000" ]]; then
    FLASH_ADDR="0x60020000"
    echo "[INFO] Detected uart_demo, using default flash address 0x60020000"
fi

# Expand tilde and convert to absolute path
BIN_FILE=$(eval echo "$BIN_FILE")
BIN_FILE=$(realpath "$BIN_FILE" 2>/dev/null || echo "$BIN_FILE")

# Convert to Windows path for JLink
BIN_FILE_WIN=$(cygpath -w "$BIN_FILE" 2>/dev/null || echo "$BIN_FILE")
JLINK_DEVICES_XML_WIN=$(cygpath -w "$JLINK_DEVICES_XML" 2>/dev/null || echo "$JLINK_DEVICES_XML")

# Validation
echo "=========================================="
echo "JLink Firmware Download for Daric"
echo "=========================================="
echo "Binary File  : $BIN_FILE"
echo "Flash Address: $FLASH_ADDR"
echo "Device       : $DEVICE"
echo "JLink Version: V7.92"
echo "=========================================="

# Check JLink executable
if ! command -v "$JLINK_EXE" >/dev/null 2>&1 && [ ! -f "$JLINK_EXE" ]; then
    echo "[ERROR] JLink executable not found: $JLINK_EXE"
    echo "Please update JLINK_EXE path in the script or add it to PATH."
    exit 1
fi

# Check binary file
if [ ! -f "$BIN_FILE" ]; then
    echo "[ERROR] Binary file not found: $BIN_FILE"
    echo "Please build the project first or provide correct path."
    exit 1
fi

# Check device configuration
if [ ! -f "$JLINK_DEVICES_XML" ]; then
    echo "[WARN] JLinkDevices.xml not found at: $JLINK_DEVICES_XML"
    echo "Will attempt to use default device configuration."
fi

#Exec SetConnectMode = 1
#Exec SetResetStrategy = 1
# Create JLink command script
echo "[INFO] Generating JLink Script..."
cat > "$JLINK_SCRIPT" <<EOF
Exec JLinkDevicesXMLPath="$JLINK_DEVICES_XML_WIN"
ConnectUnderReset
connect
halt
r
h
exec SetCompareMode=0
loadbin "$BIN_FILE_WIN", $FLASH_ADDR
verifybin $BIN_FILE_WIN", $FLASH_ADDR
r
g
exit
EOF

# Display script content for debugging
if [ "${DEBUG}" = "1" ]; then
    echo "[DEBUG] JLink Script:"
    cat "$JLINK_SCRIPT"
    echo "=========================================="
fi

# Execute JLink
echo "[INFO] Flashing firmware to $DEVICE at $FLASH_ADDR..."
"$JLINK_EXE" -device "$DEVICE" -if SWD -speed 6000 -autoconnect 1 -commandfile "$JLINK_SCRIPT"
JLINK_RESULT=$?

# Cleanup
rm -f "$JLINK_SCRIPT"

# Report result
echo "=========================================="
if [ $JLINK_RESULT -eq 0 ]; then
    echo "[SUCCESS] Firmware flashed successfully!"
else
    echo "[ERROR] Firmware flashing failed with exit code: $JLINK_RESULT"
    exit $JLINK_RESULT
fi
echo "=========================================="
