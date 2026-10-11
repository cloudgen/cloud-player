#!/bin/bash
# =========================================================================
# build.sh - Build Script for gemini-wilgat
# Minimal change: Sources project.sh as Single Source of Truth + safe fallbacks
# =========================================================================

set -e

# ====================== Source project.sh (Single Source of Truth) ======================
if [ -f "./project.sh" ]; then
    echo "[INFO] Sourcing project.sh as Single Source of Truth..."
    source ./project.sh
else
    echo "[WARN] project.sh not found — using safe generic fallbacks"
    export PRJ="${PRJ:-$(basename "$(pwd)")}"
    export SRC="${SRC:-src}"
    export SRC_C="${SRC_C:-$SRC/main/c}"
    export SRC_CPP="${SRC_CPP:-$SRC/main/cpp}"
    export BUILD="${BUILD:-target}"
    export BUILD_DEBUG="${BUILD_DEBUG:-$BUILD/exe/debug}"
fi

# Ensure required variables have sensible defaults
PROJECT_NAME="${PRJ:-app}"
SRC_DIR="${SRC_CPP:-src/main/cpp}"
BUILD_DIR="${BUILD_DEBUG:-target/exe/debug}"
BINARY="${BUILD_DIR}/${PROJECT_NAME}"

install_deps() {
    echo "[INFO] Installing required development packages..."
    sudo apt update -qq
    sudo apt install -y -qq \
        build-essential \
        pkg-config \
        libgtk-3-dev \
        libwebkit2gtk-4.1-dev \
        libsqlite3-dev \
        libglib2.0-dev \
        liblua5.3-dev \
        lua5.3 \
        git

    # ====================== sol2 (sol.hpp) via git ======================
    echo "[INFO] Checking for sol.hpp (from official sol2 repo)..."
    SOL_HEADER="/usr/local/include/sol/sol.hpp"
    MIN_SIZE=1000000  # ~1.2 MB expected

    if [ -f "$SOL_HEADER" ] && [ "$(stat -c %s "$SOL_HEADER")" -gt "$MIN_SIZE" ]; then
        echo "[INFO] ✅ sol.hpp already exists and looks correct — skipping download"
    else
        echo "[INFO] Downloading sol2 from official repository..."
        cd /tmp
        rm -rf sol2-temp
        git clone --depth 1 --single-branch -b develop https://github.com/ThePhD/sol2.git sol2-temp

        sudo mkdir -p /usr/local/include/sol
        sudo cp -r sol2-temp/include/sol/* /usr/local/include/sol/

        rm -rf sol2-temp

        if [ -f "$SOL_HEADER" ] && [ "$(stat -c %s "$SOL_HEADER")" -gt "$MIN_SIZE" ]; then
            echo "[INFO] ✅ sol.hpp installed successfully from git"
        else
            echo "[ERROR] Failed to install sol.hpp"
            exit 1
        fi
    fi

    echo "[INFO] All dependencies installed successfully."
}

# ====================== clean ======================
clean() {
    echo "[INFO] Cleaning build artifacts..."
    rm -rf "${BUILD_DIR}" target/ build/ .gradle/ autom4te.cache 2>/dev/null || true
    echo "[INFO] Clean completed."
}

# libmpv is the client library of the sibling mpv project. The sibling tree
# is not built here, so fetch the matching Ubuntu libmpv into third_party.
ensure_libmpv() {
    local root dest so
    root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    dest="${root}/third_party/libmpv"
    so="${dest}/usr/lib/x86_64-linux-gnu/libmpv.so"
    if [ -f "${so}" ] && [ -f "${dest}/usr/include/mpv/client.h" ]; then
        echo "[INFO] libmpv already present in third_party/libmpv"
        return 0
    fi
    echo "[INFO] Fetching libmpv client library..."
    local tmp
    tmp="$(mktemp -d)"
    (
        cd "${tmp}"
        apt-get download libmpv2 libmpv-dev
        mkdir -p "${dest}"
        local deb
        for deb in ./*.deb; do
            dpkg-deb -x "${deb}" "${dest}"
        done
    )
    rm -rf "${tmp}"
    if [ ! -f "${so}" ]; then
        echo "[ERROR] libmpv was not fetched"
        exit 1
    fi
}

# ====================== build ======================
# =========================================================================
# CIAO PROTECTION BLOCK - build() function (Sacred)
# =========================================================================
#
# This function is the Single Point of Truth for compilation and linking.
# Future AI or developers MUST NOT:
#   - Rewrite or heavily refactor the compilation logic
#   - Remove or simplify the PKG_FLAGS
#   - Change the compile + link approach without strong justification
#   - Bypass this function for building the application
#
# Reason: This logic was battle-tested and finally produced a successful build.
# Changing it violates CIAO Principle 7 (Reusable Function Protection) and
# Principle 18 (Protect Against AI & Human Modification).
#
# Last updated: 2026-05-24
# =========================================================================
build() {
    echo "[INFO] Starting simple C++ build for ${PROJECT_NAME}..."

    ensure_libmpv

    mkdir -p "${BUILD_DIR}"

    echo "[DEBUG] PKG_CONFIG_PATH = ${PKG_CONFIG_PATH}"
    echo "[DEBUG] Packages requested from pkg-config: gtk+-3.0 webkit2gtk-4.1 ... lua5.3"

    PKG_FLAGS=$(pkg-config --cflags --libs \
        gtk+-3.0 \
        webkit2gtk-4.1 \
        glib-2.0 \
        gobject-2.0 \
        gio-2.0 \
        sqlite3 \
        lua5.3 2>&1)

    echo "[DEBUG] pkg-config output:"
    echo "${PKG_FLAGS}"

    local project_root mpv_prefix mpv_flags epoxy_flags
    project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    mpv_prefix="${project_root}/third_party/libmpv/usr"
    # Do not ask pkg-config for mpv. Its .pc file requires private dev
    # packages that are not installed. The shared library itself is enough.
    mpv_flags="-I${mpv_prefix}/include -L${mpv_prefix}/lib/x86_64-linux-gnu -lmpv"
    epoxy_flags=$(pkg-config --cflags --libs epoxy egl)
    PKG_FLAGS="${PKG_FLAGS} ${mpv_flags} ${epoxy_flags} -ldl -Wl,-rpath,${mpv_prefix}/lib/x86_64-linux-gnu"

    CPP_FILES=$(find "${SRC_DIR}" -type f -name "*.cpp" | tr '\n' ' ')
    C_FILES=$(find "${SRC_DIR}" -type f -name "*.c" | tr '\n' ' ')

    echo "[DEBUG] Found C++ files: ${CPP_FILES}"
    echo "[DEBUG] Found C files: ${C_FILES}"

    if [ -z "$CPP_FILES" ] && [ -z "$C_FILES" ]; then
        echo "[ERROR] No source files found!"
        exit 1
    fi

    echo "[INFO] Compiling..."
    # -I/usr/local/include so <sol/sol.hpp> resolves.
    if [ -n "$C_FILES" ]; then
        gcc -std=c11 -Wall -Wextra -g -O2 -I/usr/local/include ${PKG_FLAGS} -c ${C_FILES}
    fi
    if [ -n "$CPP_FILES" ]; then
        g++ -std=c++17 -Wall -Wextra -g -O2 -I/usr/local/include ${PKG_FLAGS} -c ${CPP_FILES}
    fi

    echo "[INFO] Linking final binary..."
    if [ -n "$CPP_FILES" ]; then
        g++ *.o -o "${BINARY}" ${PKG_FLAGS} -pthread
    else
        gcc *.o -o "${BINARY}" ${PKG_FLAGS} -pthread
    fi

    rm -f *.o

    echo "[INFO] Build successful!"
    echo "[INFO] Binary ready → ${BINARY}"
    echo "[INFO] Run with: ${BINARY}"
}

# ====================== Main logic ======================
case "$1" in
    install|deps)
        install_deps
        ;;
    clean)
        clean
        ;;
    build|""|simple)
        build
        ;;
    *)
        echo "Usage: $0 {install|clean|build}"
        echo "   install   → install system dependencies"
        echo "   clean     → remove build artifacts"
        echo "   build     → compile the application"
        exit 1
        ;;
esac