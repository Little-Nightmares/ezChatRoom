#!/bin/bash
# Cross-compile ChatRoomClient for Windows using MinGW-w64
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-windows"

echo "=== Cross-compiling ChatRoomClient for Windows ==="

# Clean and configure
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_DIR/cmake/toolchain-mingw-w64.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="/opt/qt6-windows/6.2.4/mingw_64;/opt/openssl-windows;/usr/lib/x86_64-linux-gnu" \
    -DOPENSSL_ROOT_DIR=/opt/openssl-windows \
    -DQt6HostInfo_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6HostInfo

# Build all object files (link will fail, that's expected)
cmake --build . --target ChatRoomClient -j$(nproc) 2>&1 | tail -3 || true

echo ""
echo "=== Re-linking with --start-group/--end-group ==="

# Manual re-link with linker groups to resolve circular dependencies
cd "$BUILD_DIR/client"
x86_64-w64-mingw32-g++ -O3 -DNDEBUG \
    -Wl,--whole-archive CMakeFiles/ChatRoomClient.dir/objects.a -Wl,--no-whole-archive \
    -o ChatRoomClient.exe \
    -Wl,--start-group \
    @CMakeFiles/ChatRoomClient.dir/linklibs.rsp \
    -Wl,--end-group \
    -lws2_32 -lcrypt32 -lsecur32 -lbcrypt -ladvapi32

echo ""
echo "=== Build complete ==="
file ChatRoomClient.exe
ls -lh ChatRoomClient.exe
