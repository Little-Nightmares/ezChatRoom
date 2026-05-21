#!/usr/bin/env bash
#
# package_linux.sh - ChatRoom Linux Release Packaging Script
#
# This script builds the ChatRoom project in Release mode, strips debug symbols,
# collects all shared library dependencies, Qt plugins, and QML modules,
# and packages everything into a distributable tar.gz archive.
#
# Usage:
#   ./scripts/package_linux.sh [BUILD_DIR]
#
# Arguments:
#   BUILD_DIR  - Optional path to the build directory (default: build)
#
# The resulting archive will be created in the project root:
#   ChatRoom-linux-x64-v1.0.0.tar.gz
#

set -euo pipefail

# =============================================================================
# Configuration
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${1:-${PROJECT_DIR}/build}"

VERSION="1.0.0"
ARCH="x64"
PLATFORM="linux"
PACKAGE_NAME="ChatRoom-${PLATFORM}-${ARCH}-v${VERSION}"
RELEASE_DIR="${PROJECT_DIR}/${PACKAGE_NAME}"

# Binary paths (relative to build directory)
SERVER_BIN="${BUILD_DIR}/server/ChatRoomServer"
CLIENT_BIN="${BUILD_DIR}/client/ChatRoomClient"

# Qt paths
QT_PLUGINS_DIR="/usr/lib/x86_64-linux-gnu/qt6/plugins"
QT_QML_DIR="/usr/lib/x86_64-linux-gnu/qt6/qml"

# Qt QML modules to copy
QML_MODULES=(
    "QtQuick"
    "QtQuick.Controls"
    "QtQuick.Layouts"
    "QtQuick.Window"
    "QtQuick.Dialogs"
    "QtQml"
    "QtQml.Models"
    "QtQml.WorkerScript"
    "QtTest"
)

# Qt plugins to copy
QT_PLUGIN_SUBDIRS=(
    "platforms"
    "sqldrivers"
    "imageformats"
    "styles"
)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# =============================================================================
# Utility Functions
# =============================================================================

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[OK]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

die() {
    log_error "$1"
    exit 1
}

check_command() {
    if ! command -v "$1" &>/dev/null; then
        die "Required command '$1' not found. Please install it first."
    fi
}

# =============================================================================
# Pre-flight Checks
# =============================================================================

log_info "ChatRoom Linux Release Packager v${VERSION}"
log_info "============================================"

check_command "cmake"
check_command "make"
check_command "strip"
check_command "ldd"
check_command "tar"

# Validate Qt paths
if [[ ! -d "${QT_PLUGINS_DIR}" ]]; then
    die "Qt plugins directory not found: ${QT_PLUGINS_DIR}"
fi

if [[ ! -d "${QT_QML_DIR}" ]]; then
    die "Qt QML directory not found: ${QT_QML_DIR}"
fi

# =============================================================================
# Step 1: Build in Release Mode
# =============================================================================

log_info "Step 1: Building in Release mode..."

if [[ ! -f "${BUILD_DIR}/Makefile" ]]; then
    log_info "Configuring CMake in Release mode..."
    cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="${RELEASE_DIR}" \
        || die "CMake configuration failed."
else
    log_warn "Build directory already configured. Skipping CMake configuration."
    log_warn "If you want a clean Release build, remove ${BUILD_DIR} and re-run."
fi

log_info "Running make..."
cmake --build "${BUILD_DIR}" --config Release -j"$(nproc)" \
    || die "Build failed."

# Verify binaries exist
if [[ ! -f "${SERVER_BIN}" ]]; then
    die "Server binary not found: ${SERVER_BIN}"
fi

if [[ ! -f "${CLIENT_BIN}" ]]; then
    die "Client binary not found: ${CLIENT_BIN}"
fi

log_success "Build completed successfully."

# =============================================================================
# Step 2: Strip Debug Symbols
# =============================================================================

log_info "Step 2: Stripping debug symbols from binaries..."

strip --strip-debug "${SERVER_BIN}" \
    || die "Failed to strip server binary."

strip --strip-debug "${CLIENT_BIN}" \
    || die "Failed to strip client binary."

log_success "Debug symbols stripped."

# =============================================================================
# Step 3: Create Release Directory Structure
# =============================================================================

log_info "Step 3: Creating release directory structure..."

# Clean previous release directory if it exists
if [[ -d "${RELEASE_DIR}" ]]; then
    log_warn "Removing previous release directory: ${RELEASE_DIR}"
    rm -rf "${RELEASE_DIR}"
fi

mkdir -p "${RELEASE_DIR}"/{bin,lib,plugins,qml,resources}

log_success "Directory structure created."

# =============================================================================
# Step 4: Copy Binaries
# =============================================================================

log_info "Step 4: Copying binaries..."

cp "${SERVER_BIN}" "${RELEASE_DIR}/bin/ChatRoomServer"
cp "${CLIENT_BIN}" "${RELEASE_DIR}/bin/ChatRoomClient"
chmod +x "${RELEASE_DIR}/bin/ChatRoomServer" "${RELEASE_DIR}/bin/ChatRoomClient"

log_success "Binaries copied."

# =============================================================================
# Step 5: Copy Shared Library Dependencies
# =============================================================================

log_info "Step 5: Collecting shared library dependencies..."

# Collect unique .so files from both binaries
declare -A LIB_FILES

for binary in "${SERVER_BIN}" "${CLIENT_BIN}"; do
    log_info "  Analyzing dependencies of $(basename "${binary}")..."

    while IFS= read -r line; do
        # Parse ldd output: extract the library path
        # ldd output format: libname.so.X => /path/to/lib.so.X (0xaddr)
        if [[ "${line}" =~ "=>" ]]; then
            lib_path=$(echo "${line}" | awk '{print $3}')
        else
            # Some entries don't have the "=>" separator
            lib_path=$(echo "${line}" | awk '{print $1}')
        fi

        # Skip empty paths or non-existent files
        [[ -z "${lib_path}" ]] && continue
        [[ "${lib_path}" != /* ]] && continue
        [[ ! -f "${lib_path}" ]] && continue

        # Skip linux-vdso and ld-linux (dynamic linker)
        basename_lib=$(basename "${lib_path}")
        if [[ "${basename_lib}" == "linux-vdso.so"* ]] || \
           [[ "${basename_lib}" == "ld-linux"* ]]; then
            continue
        fi

        LIB_FILES["${lib_path}"]=1
    done < <(ldd "${binary}" 2>/dev/null)
done

# Copy all collected libraries
lib_count=0
for lib_path in "${!LIB_FILES[@]}"; do
    cp -L "${lib_path}" "${RELEASE_DIR}/lib/"
    ((lib_count++))
done

log_success "Copied ${lib_count} shared libraries to lib/"

# =============================================================================
# Step 6: Copy Qt Plugins
# =============================================================================

log_info "Step 6: Copying Qt plugins..."

for plugin_subdir in "${QT_PLUGIN_SUBDIRS[@]}"; do
    src_dir="${QT_PLUGINS_DIR}/${plugin_subdir}"
    dst_dir="${RELEASE_DIR}/plugins/${plugin_subdir}"

    if [[ ! -d "${src_dir}" ]]; then
        log_warn "  Plugin directory not found, skipping: ${src_dir}"
        continue
    fi

    mkdir -p "${dst_dir}"

    # For platforms plugin, only copy libqxcb.so (the essential X11 platform plugin)
    if [[ "${plugin_subdir}" == "platforms" ]]; then
        if [[ -f "${src_dir}/libqxcb.so" ]]; then
            cp "${src_dir}/libqxcb.so" "${dst_dir}/"
            log_info "  Copied platforms/libqxcb.so"
        else
            log_warn "  libqxcb.so not found in ${src_dir}"
        fi
    # For sqldrivers, only copy libqsqlite.so
    elif [[ "${plugin_subdir}" == "sqldrivers" ]]; then
        if [[ -f "${src_dir}/libqsqlite.so" ]]; then
            cp "${src_dir}/libqsqlite.so" "${dst_dir}/"
            log_info "  Copied sqldrivers/libqsqlite.so"
        else
            log_warn "  libqsqlite.so not found in ${src_dir}"
        fi
    else
        # Copy all .so files in imageformats and styles
        cp_count=0
        for so_file in "${src_dir}"/*.so; do
            [[ -f "${so_file}" ]] || continue
            cp "${so_file}" "${dst_dir}/"
            ((cp_count++))
        done
        log_info "  Copied ${cp_count} plugins from ${plugin_subdir}/"
    fi
done

log_success "Qt plugins copied."

# =============================================================================
# Step 7: Copy Qt QML Modules
# =============================================================================

log_info "Step 7: Copying Qt QML modules..."

for qml_module in "${QML_MODULES[@]}"; do
    src_dir="${QT_QML_DIR}/${qml_module}"
    dst_dir="${RELEASE_DIR}/qml/${qml_module}"

    if [[ ! -d "${src_dir}" ]]; then
        log_warn "  QML module not found, skipping: ${qml_module}"
        continue
    fi

    mkdir -p "${dst_dir}"
    cp -r "${src_dir}"/* "${dst_dir}/"
    log_info "  Copied QML module: ${qml_module}"
done

# Copy the app's own QML module from the build directory
APP_QML_DIR="${BUILD_DIR}/client/ChatRoom"
if [[ -d "${APP_QML_DIR}" ]]; then
    dst_dir="${RELEASE_DIR}/qml/ChatRoom"
    mkdir -p "${dst_dir}"
    cp -r "${APP_QML_DIR}"/* "${dst_dir}/"
    log_info "  Copied app QML module: ChatRoom"
else
    log_warn "  App QML module not found at ${APP_QML_DIR}, skipping."
fi

log_success "QML modules copied."

# =============================================================================
# Step 8: Copy Qt Resources (if any)
# =============================================================================

log_info "Step 8: Checking for Qt resources..."

# Look for qrc files or resource directories in the build output
RESOURCE_SRC="${BUILD_DIR}/client/.rcc"
if [[ -d "${RESOURCE_SRC}" ]]; then
    # Copy compiled resource files (.qrc, .rcc)
    find "${RESOURCE_SRC}" -type f \( -name "*.qrc" -o -name "*.rcc" \) \
        -exec cp {} "${RELEASE_DIR}/resources/" \; 2>/dev/null || true
    rsrc_count=$(find "${RELEASE_DIR}/resources" -type f | wc -l)
    if [[ "${rsrc_count}" -gt 0 ]]; then
        log_info "  Copied ${rsrc_count} resource file(s)."
    else
        log_info "  No compiled resource files found."
    fi
else
    log_info "  No resource directory found, skipping."
fi

log_success "Resources check completed."

# =============================================================================
# Step 9: Create start_server.sh
# =============================================================================

log_info "Step 9: Creating start_server.sh..."

cat > "${RELEASE_DIR}/start_server.sh" << 'SERVER_SCRIPT'
#!/usr/bin/env bash
#
# start_server.sh - Launch the ChatRoom Server
#
# This script sets up the necessary runtime environment
# (LD_LIBRARY_PATH) and starts the ChatRoom server.
#

set -euo pipefail

# Resolve the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set library path to include bundled shared libraries
export LD_LIBRARY_PATH="${SCRIPT_DIR}/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Server configuration (can be overridden via environment variables)
SERVER_PORT="${CHATROOM_PORT:-8080}"
SERVER_HOST="${CHATROOM_HOST:-0.0.0.0}"

echo "========================================="
echo "  ChatRoom Server v1.0.0"
echo "========================================="
echo "  Host: ${SERVER_HOST}"
echo "  Port: ${SERVER_PORT}"
echo "========================================="

exec "${SCRIPT_DIR}/bin/ChatRoomServer" --host "${SERVER_HOST}" --port "${SERVER_PORT}" "$@"
SERVER_SCRIPT

chmod +x "${RELEASE_DIR}/start_server.sh"
log_success "start_server.sh created."

# =============================================================================
# Step 10: Create start_client.sh
# =============================================================================

log_info "Step 10: Creating start_client.sh..."

cat > "${RELEASE_DIR}/start_client.sh" << 'CLIENT_SCRIPT'
#!/usr/bin/env bash
#
# start_client.sh - Launch the ChatRoom Client
#
# This script sets up the necessary runtime environment
# (LD_LIBRARY_PATH, QT_PLUGIN_PATH, QML2_IMPORT_PATH) and
# starts the ChatRoom client GUI application.
#

set -euo pipefail

# Resolve the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set library path to include bundled shared libraries
export LD_LIBRARY_PATH="${SCRIPT_DIR}/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Set Qt plugin path to use bundled plugins
export QT_PLUGIN_PATH="${SCRIPT_DIR}/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"

# Set QML import path to use bundled QML modules
export QML2_IMPORT_PATH="${SCRIPT_DIR}/qml${QML2_IMPORT_PATH:+:$QML2_IMPORT_PATH}"

# Disable Qt's own deployment checks (we handle it ourselves)
export QT_ENABLE_HIGHDPI_SCALING=1

echo "========================================="
echo "  ChatRoom Client v1.0.0"
echo "========================================="

exec "${SCRIPT_DIR}/bin/ChatRoomClient" "$@"
CLIENT_SCRIPT

chmod +x "${RELEASE_DIR}/start_client.sh"
log_success "start_client.sh created."

# =============================================================================
# Step 11: Create README.md
# =============================================================================

log_info "Step 11: Creating README.md..."

cat > "${RELEASE_DIR}/README.md" << 'README_EOF'
# ChatRoom v1.0.0 - Linux x64

A real-time chat application with a client-server architecture built with Qt 6.

## System Requirements

- **OS**: Linux (x86_64)
- **Display Server**: X11 (via xcb platform plugin)
- **Dependencies**: All required shared libraries are bundled in the `lib/` directory

## Quick Start

### Start the Server

```bash
./start_server.sh
```

The server listens on `0.0.0.0:8080` by default. You can customize via environment variables:

```bash
CHATROOM_PORT=9090 CHATROOM_HOST=127.0.0.1 ./start_server.sh
```

### Start the Client

```bash
./start_client.sh
```

## Directory Structure

```
ChatRoom-linux-x64/
+-- bin/            # Executables (ChatRoomServer, ChatRoomClient)
+-- lib/            # Bundled shared library dependencies
+-- plugins/        # Qt plugins (platforms, sqldrivers, imageformats, styles)
+-- qml/            # Qt QML modules and app QML module
+-- resources/      # Qt compiled resources
+-- start_server.sh # Server launch script
+-- start_client.sh # Client launch script
+-- README.md       # This file
```

## Environment Variables

| Variable           | Default        | Description                        |
|--------------------|----------------|------------------------------------|
| `CHATROOM_HOST`    | `0.0.0.0`      | Server bind address                |
| `CHATROOM_PORT`    | `8080`         | Server listen port                 |
| `LD_LIBRARY_PATH`  | (auto-set)     | Path to bundled shared libraries   |
| `QT_PLUGIN_PATH`   | (auto-set)     | Path to bundled Qt plugins         |
| `QML2_IMPORT_PATH` | (auto-set)     | Path to bundled QML modules        |

## Troubleshooting

- **"cannot open shared object file"**: Ensure you are using `start_server.sh` or `start_client.sh` to launch the application. Do not run binaries directly.
- **"could not find the Qt platform plugin xcb"**: The `platforms/libqxcb.so` plugin is included. Make sure `QT_PLUGIN_PATH` is set correctly (handled by the start scripts).
- **Display issues**: Ensure you have an X11 display server running. Set `DISPLAY` environment variable if needed: `export DISPLAY=:0`

## License

See the project source repository for license information.
README_EOF

log_success "README.md created."

# =============================================================================
# Step 12: Package into tar.gz
# =============================================================================

log_info "Step 12: Creating release archive..."

ARCHIVE_PATH="${PROJECT_DIR}/${PACKAGE_NAME}.tar.gz"

# Remove previous archive if it exists
if [[ -f "${ARCHIVE_PATH}" ]]; then
    log_warn "Removing previous archive: ${ARCHIVE_PATH}"
    rm -f "${ARCHIVE_PATH}"
fi

# Create the tar.gz archive
tar -czf "${ARCHIVE_PATH}" -C "${PROJECT_DIR}" "${PACKAGE_NAME}" \
    || die "Failed to create archive."

ARCHIVE_SIZE=$(du -h "${ARCHIVE_PATH}" | cut -f1)

log_success "Archive created: ${ARCHIVE_PATH} (${ARCHIVE_SIZE})"

# =============================================================================
# Summary
# =============================================================================

echo ""
echo "========================================="
echo -e "${GREEN}  Packaging Complete!${NC}"
echo "========================================="
echo ""
echo "  Release directory: ${RELEASE_DIR}/"
echo "  Archive:           ${ARCHIVE_PATH}"
echo "  Archive size:      ${ARCHIVE_SIZE}"
echo ""
echo "  Contents:"
echo "    bin/              $(ls "${RELEASE_DIR}/bin/" | tr '\n' ' ')"
echo "    lib/              $(find "${RELEASE_DIR}/lib" -name '*.so*' | wc -l) shared libraries"
echo "    plugins/          $(find "${RELEASE_DIR}/plugins" -name '*.so' | wc -l) Qt plugins"
echo "    qml/              $(ls "${RELEASE_DIR}/qml/")"
echo ""
echo "  To test locally:"
echo "    cd ${RELEASE_DIR}"
echo "    ./start_server.sh"
echo "    ./start_client.sh"
echo ""
echo "  To distribute:"
echo "    scp ${ARCHIVE_PATH} user@host:/path/"
echo ""
echo "========================================="
