#!/bin/bash
#
# setup.sh - Install ARM GNU Toolchain on the Kakip board (aarch64)
#
# Installs arm-none-eabi-gcc 13.3.rel1 to /opt/arm-gnu-toolchain.
# Run once, then add the toolchain to PATH before building:
#
#   export PATH=/opt/arm-gnu-toolchain/bin:$PATH
#

set -e

# Must run on the Kakip board (aarch64 Linux)
ARCH=$(uname -m)
if [ "$ARCH" != "aarch64" ]; then
    echo "Error: this script is for the Kakip board (aarch64). Detected: $ARCH"
    exit 1
fi

# Install make if not present
if ! command -v make &>/dev/null; then
    echo "Installing make..."
    sudo apt install -y make
fi

TOOLCHAIN_VER="13.3.rel1"
TOOLCHAIN_DIR="/opt/arm-gnu-toolchain"
TARBALL="arm-gnu-toolchain-${TOOLCHAIN_VER}-aarch64-arm-none-eabi.tar.xz"
URL="https://developer.arm.com/-/media/Files/downloads/gnu/${TOOLCHAIN_VER}/binrel/${TARBALL}"

# Check if already installed
if [ -x "${TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc" ]; then
    INSTALLED_VER=$("${TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc" --version | head -1)
    echo "Toolchain already installed: ${INSTALLED_VER}"
    echo ""
    echo "To use: export PATH=${TOOLCHAIN_DIR}/bin:\$PATH"
    exit 0
fi

echo "Installing ARM GNU Toolchain ${TOOLCHAIN_VER}..."
echo ""

# Download
TMP="/tmp/${TARBALL}"
if [ ! -f "$TMP" ]; then
    echo "Downloading ${TARBALL}..."
    wget -O "$TMP" "$URL"
else
    echo "Using cached download: ${TMP}"
fi

# Install
echo "Extracting to ${TOOLCHAIN_DIR}..."
sudo mkdir -p "$TOOLCHAIN_DIR"
sudo tar xf "$TMP" -C "$TOOLCHAIN_DIR" --strip-components=1

# Verify
echo ""
"${TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc" --version | head -1
echo ""
echo "Done. To use:"
echo ""
echo "  export PATH=${TOOLCHAIN_DIR}/bin:\$PATH"
echo ""
