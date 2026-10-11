#!/bin/bash
# Checks for requirement-cpp-desktop.md, requirement-cpp-linux-x11.md,
# and requirement-cpp-macos.md. Does not point HOME at a scratch directory.
set -u

BIN="${1:-}"
if [ -z "${BIN}" ]; then
    echo "usage: $0 /path/to/gemini-wilgat" >&2
    exit 2
fi
if [ ! -x "${BIN}" ]; then
    echo "not executable: ${BIN}" >&2
    exit 2
fi

pass=0
fail=0

ok() {
    pass=$((pass + 1))
    printf 'PASS %s\n' "$1"
}

bad() {
    fail=$((fail + 1))
    printf 'FAIL %s\n' "$1" >&2
}

# Run and capture stdout. Exit status is stored in RUN_EC.
run_bin() {
    RUN_OUT="$( "$@" 2>/dev/null )"
    RUN_EC=$?
}

# TP-DESKTOP-WINDOWS
run_bin "${BIN}" desktopcheck --os windows
if [ "${RUN_EC}" -eq 0 ] && [ "${RUN_OUT}" = "windows" ]; then
    ok "TP-DESKTOP-WINDOWS token"
else
    bad "TP-DESKTOP-WINDOWS token (${RUN_EC}: ${RUN_OUT})"
fi
run_bin "${BIN}" desktopcheck --os windows --display :0 --gdk-backend x11
if [ "${RUN_EC}" -eq 0 ] && [ "${RUN_OUT}" = "windows" ]; then
    ok "TP-DESKTOP-WINDOWS ignores display"
else
    bad "TP-DESKTOP-WINDOWS ignores display (${RUN_EC}: ${RUN_OUT})"
fi

# TP-DESKTOP-MACOS
run_bin "${BIN}" desktopcheck --os macos
if [ "${RUN_EC}" -eq 0 ] && [ "${RUN_OUT}" = "macos" ]; then
    ok "TP-DESKTOP-MACOS token"
else
    bad "TP-DESKTOP-MACOS token (${RUN_EC}: ${RUN_OUT})"
fi
run_bin "${BIN}" desktopcheck --os macos --display :0 --gdk-backend x11
if [ "${RUN_EC}" -eq 0 ] && [ "${RUN_OUT}" = "macos" ]; then
    ok "TP-DESKTOP-MACOS ignores display"
else
    bad "TP-DESKTOP-MACOS ignores display (${RUN_EC}: ${RUN_OUT})"
fi
run_bin "${BIN}" --json desktopcheck --os macos
if [ "${RUN_EC}" -eq 0 ] && [ "${RUN_OUT}" = '{"desktop":"macos","label":"macOS"}' ]; then
    ok "TP-DESKTOP-MACOS json"
else
    bad "TP-DESKTOP-MACOS json (${RUN_EC}: ${RUN_OUT})"
fi

# TP-DESKTOP-LINUX-X11
run_bin "${BIN}" desktopcheck --os linux --display :99
if [ "${RUN_EC}" -eq 0 ] && [ "${RUN_OUT}" = "linux-desktop" ]; then
    ok "TP-DESKTOP-LINUX-X11 empty backend"
else
    bad "TP-DESKTOP-LINUX-X11 empty backend (${RUN_EC}: ${RUN_OUT})"
fi
run_bin "${BIN}" desktopcheck --os linux --display :99 --gdk-backend x11
if [ "${RUN_EC}" -eq 0 ] && [ "${RUN_OUT}" = "linux-desktop" ]; then
    ok "TP-DESKTOP-LINUX-X11 x11 backend"
else
    bad "TP-DESKTOP-LINUX-X11 x11 backend (${RUN_EC}: ${RUN_OUT})"
fi
run_bin "${BIN}" desktopcheck --os linux --display :99 --gdk-backend wayland --session-type x11
if [ "${RUN_EC}" -eq 1 ] && printf '%s\n' "${RUN_OUT}" | grep -q '^unknown$' && printf '%s\n' "${RUN_OUT}" | grep -q '^Next:'; then
    ok "TP-DESKTOP-LINUX-X11 wayland backend"
else
    bad "TP-DESKTOP-LINUX-X11 wayland backend (${RUN_EC}: ${RUN_OUT})"
fi
run_bin "${BIN}" desktopcheck --os linux --display :99 --session-type wayland --wayland wayland-0
if [ "${RUN_EC}" -eq 0 ] && [ "${RUN_OUT}" = "linux-desktop" ]; then
    ok "TP-DESKTOP-LINUX-X11 session type does not rename"
else
    bad "TP-DESKTOP-LINUX-X11 session type does not rename (${RUN_EC}: ${RUN_OUT})"
fi
run_bin "${BIN}" desktopcheck --os linux
if [ "${RUN_EC}" -eq 1 ] && printf '%s\n' "${RUN_OUT}" | grep -q '^Next:'; then
    ok "TP-DESKTOP-LINUX-X11 missing display"
else
    bad "TP-DESKTOP-LINUX-X11 missing display (${RUN_EC}: ${RUN_OUT})"
fi

# TP-DESKTOP-SEPARATE
w="$("${BIN}" desktopcheck --os windows)"
m="$("${BIN}" desktopcheck --os macos)"
l="$("${BIN}" desktopcheck --os linux --display :99)"
if [ "${w}" = "windows" ] && [ "${m}" = "macos" ] && [ "${l}" = "linux-desktop" ] && [ "${w}" != "${m}" ] && [ "${m}" != "${l}" ] && [ "${w}" != "${l}" ]; then
    ok "TP-DESKTOP-SEPARATE three tokens"
else
    bad "TP-DESKTOP-SEPARATE three tokens (${w}/${m}/${l})"
fi
run_bin "${BIN}" desktop --os windows
if [ "${RUN_EC}" -eq 1 ] && printf '%s\n' "${RUN_OUT}" | grep -q '^Next:'; then
    ok "TP-DESKTOP-SEPARATE live rejects --os"
else
    bad "TP-DESKTOP-SEPARATE live rejects --os (${RUN_EC}: ${RUN_OUT})"
fi
help_out="$("${BIN}" help)"
if printf '%s\n' "${help_out}" | grep -q 'desktop' && printf '%s\n' "${help_out}" | grep -A 20 'Commands:' | grep -q 'desktop' && printf '%s\n' "${help_out}" | grep -A 12 'Tests:' | grep -q 'desktopcheck'; then
    ok "TP-DESKTOP-SEPARATE help headings"
else
    bad "TP-DESKTOP-SEPARATE help headings"
fi

# TP-DESKTOP-LIVE
run_bin env -u GDK_BACKEND DISPLAY=:99 "${BIN}" desktop
if [ "${RUN_EC}" -eq 0 ] && [ "${RUN_OUT}" = "linux-desktop" ]; then
    ok "TP-DESKTOP-LIVE selects x11"
else
    bad "TP-DESKTOP-LIVE selects x11 (${RUN_EC}: ${RUN_OUT})"
fi
run_bin env GDK_BACKEND=wayland DISPLAY=:99 "${BIN}" desktop
if [ "${RUN_EC}" -eq 1 ] && printf '%s\n' "${RUN_OUT}" | grep -q '^Next:'; then
    ok "TP-DESKTOP-LIVE wayland is not linux desktop"
else
    bad "TP-DESKTOP-LIVE wayland is not linux desktop (${RUN_EC}: ${RUN_OUT})"
fi

printf 'passed %s failed %s\n' "${pass}" "${fail}"
if [ "${fail}" -ne 0 ]; then
    exit 1
fi
exit 0
