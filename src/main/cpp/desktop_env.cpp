// desktop_env.cpp
// =========================================================================
// CIAO DEFENSIVE CODING STYLE - DESKTOP IDENTITY
// =========================================================================
//
// Law: requirement-cpp-desktop.md
// The three tokens are windows, macos, and linux-desktop. Anything else
// is unknown. WAYLAND_DISPLAY and XDG_SESSION_TYPE are accepted and do
// not rename a result that DISPLAY and GDK_BACKEND already decide.
// Last aligned: 2026-10-11
// =========================================================================

#include "desktop_env.h"
#include "output.h"

#include <string.h>

static const char *compiled_os_name(void)
{
#if defined(_WIN32)
    return "windows";
#elif defined(__APPLE__)
    return "macos";
#elif defined(__linux__)
    return "linux";
#else
    return "unknown";
#endif
}

static gboolean backend_has_token(const char *backend, const char *token)
{
    size_t token_len;
    const char *cursor;

    if (backend == NULL || token == NULL || token[0] == '\0') {
        return FALSE;
    }
    token_len = strlen(token);
    cursor = backend;
    while (*cursor != '\0') {
        const char *start;
        size_t len;

        while (*cursor == ',' || *cursor == ' ') {
            cursor++;
        }
        if (*cursor == '\0') {
            break;
        }
        start = cursor;
        while (*cursor != '\0' && *cursor != ',') {
            cursor++;
        }
        len = (size_t)(cursor - start);
        while (len > 0 && start[len - 1] == ' ') {
            len--;
        }
        if (len == token_len && strncmp(start, token, token_len) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean text_is(const char *value, const char *expected)
{
    return value != NULL && strcmp(value, expected) == 0;
}

DesktopKind desktop_kind_from_signals(const DesktopSignals *signals)
{
    const char *os_name;
    DesktopSignals empty;

    if (signals == NULL) {
        memset(&empty, 0, sizeof(empty));
        signals = &empty;
    }
    os_name = signals->os_name;
    if (os_name == NULL || os_name[0] == '\0') {
        os_name = compiled_os_name();
    }
    if (text_is(os_name, "windows")) {
        return DESKTOP_WINDOWS;
    }
    if (text_is(os_name, "macos")) {
        return DESKTOP_MACOS;
    }
    if (!text_is(os_name, "linux")) {
        return DESKTOP_UNKNOWN;
    }
    if (signals->display == NULL || signals->display[0] == '\0') {
        return DESKTOP_UNKNOWN;
    }
    if (signals->gdk_backend == NULL || signals->gdk_backend[0] == '\0') {
        return DESKTOP_LINUX;
    }
    if (backend_has_token(signals->gdk_backend, "x11")) {
        return DESKTOP_LINUX;
    }
    return DESKTOP_UNKNOWN;
}

const char *desktop_kind_token(DesktopKind kind)
{
    switch (kind) {
    case DESKTOP_WINDOWS:
        return "windows";
    case DESKTOP_MACOS:
        return "macos";
    case DESKTOP_LINUX:
        return "linux-desktop";
    default:
        return "unknown";
    }
}

const char *desktop_kind_label(DesktopKind kind)
{
    switch (kind) {
    case DESKTOP_WINDOWS:
        return "Windows";
    case DESKTOP_MACOS:
        return "macOS";
    case DESKTOP_LINUX:
        return "Linux Desktop";
    default:
        return "unknown";
    }
}

void desktop_signals_from_environment(DesktopSignals *out)
{
    if (out == NULL) {
        return;
    }
    out->os_name = compiled_os_name();
    out->display = g_getenv("DISPLAY");
    out->gdk_backend = g_getenv("GDK_BACKEND");
    out->wayland_display = g_getenv("WAYLAND_DISPLAY");
    out->session_type = g_getenv("XDG_SESSION_TYPE");
}

DesktopKind desktop_kind_live(void)
{
    DesktopSignals signals;

    desktop_signals_from_environment(&signals);
    return desktop_kind_from_signals(&signals);
}

void desktop_prepare_linux_x11(void)
{
#if defined(__linux__)
    const char *display = g_getenv("DISPLAY");
    const char *backend = g_getenv("GDK_BACKEND");

    if (display != NULL && display[0] != '\0' &&
        (backend == NULL || backend[0] == '\0')) {
        g_setenv("GDK_BACKEND", "x11", TRUE);
    }
#else
    return;
#endif
}

static gboolean is_option(const char *arg)
{
    return arg != NULL && strncmp(arg, "--", 2) == 0;
}

static gboolean argv_first_command_is(int argc, char **argv, const char *name)
{
    if (argv == NULL || name == NULL) {
        return FALSE;
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i] == NULL) {
            continue;
        }
        if (strcmp(argv[i], "--config") == 0) {
            i++;
            continue;
        }
        if (is_option(argv[i])) {
            const char *eq = strchr(argv[i], '=');
            if (eq == NULL) {
                /* A following value is skipped only when this option is
                   known to take one. Unknown options stay in place so a
                   command name is not hidden behind them. */
                if (strcmp(argv[i], "--os") == 0 ||
                    strcmp(argv[i], "--display") == 0 ||
                    strcmp(argv[i], "--gdk-backend") == 0 ||
                    strcmp(argv[i], "--wayland") == 0 ||
                    strcmp(argv[i], "--session-type") == 0) {
                    i++;
                }
            }
            continue;
        }
        return strcmp(argv[i], name) == 0;
    }
    return FALSE;
}

gboolean ciao_argv_has_desktop_verb(int argc, char **argv)
{
    return argv_first_command_is(argc, argv, "desktop");
}

gboolean ciao_argv_has_desktopcheck_verb(int argc, char **argv)
{
    return argv_first_command_is(argc, argv, "desktopcheck");
}

static void finish_desktop(DesktopKind kind, gboolean ok, const char *detail)
{
    const char *token = desktop_kind_token(kind);
    const char *label = desktop_kind_label(kind);

    if (ciao_json_mode) {
        if (ok) {
            g_print("{\"desktop\":\"%s\",\"label\":\"%s\"}\n", token, label);
        } else {
            gchar *error_json = g_strescape(detail != NULL ? detail : "unknown", NULL);
            g_print("{\"desktop\":\"unknown\",\"label\":\"unknown\",\"error\":\"%s\"}\n",
                    error_json != NULL ? error_json : "");
            g_free(error_json);
        }
        exit(ok ? 0 : 1);
    }
    if (ok) {
        g_print("%s\n", token);
        exit(0);
    }
    g_print("%s\n%s\n", token, detail != NULL ? detail : "unknown");
    exit(1);
}

static const char *option_value(int argc, char **argv, int *index, const char *name, gboolean *present)
{
    const char *arg = argv[*index];
    size_t name_len = strlen(name);

    if (strncmp(arg, name, name_len) == 0 && arg[name_len] == '=') {
        *present = TRUE;
        return arg + name_len + 1;
    }
    if (strcmp(arg, name) == 0) {
        *present = TRUE;
        if (*index + 1 >= argc || argv[*index + 1] == NULL) {
            return NULL;
        }
        *index += 1;
        return argv[*index];
    }
    return NULL;
}

void ciao_desktop_command_dispatch(int argc, char **argv)
{
    desktop_prepare_linux_x11();
    for (int i = 1; i < argc; i++) {
        if (argv[i] == NULL || strcmp(argv[i], "desktop") == 0) {
            continue;
        }
        if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "--json") == 0) {
            continue;
        }
        if (strcmp(argv[i], "--config") == 0) {
            i++;
            continue;
        }
        if (strncmp(argv[i], "--shutdown=", 11) == 0 || strncmp(argv[i], "--bookmark=", 11) == 0) {
            continue;
        }
        finish_desktop(
            DESKTOP_UNKNOWN,
            FALSE,
            "The desktop command reports this process.\n"
            "Next: run desktop with no extra arguments, or run desktopcheck --os windows, desktopcheck --os macos, or desktopcheck --os linux.");
    }
    {
        DesktopKind kind = desktop_kind_live();
        if (kind == DESKTOP_UNKNOWN) {
            finish_desktop(
                DESKTOP_UNKNOWN,
                FALSE,
                "This process is not Windows, macOS, or Linux Desktop.\n"
                "Next: on Linux, set DISPLAY to an X11 server and leave GDK_BACKEND unset, or set GDK_BACKEND=x11.");
        }
        finish_desktop(kind, TRUE, NULL);
    }
}

void ciao_desktopcheck_command_dispatch(int argc, char **argv)
{
    DesktopSignals signals;
    gboolean saw_os = FALSE;
    const char *os_name = NULL;
    gboolean missing_os_value = FALSE;

    memset(&signals, 0, sizeof(signals));
    for (int i = 1; i < argc; i++) {
        gboolean present = FALSE;
        const char *value;

        if (argv[i] == NULL) {
            continue;
        }
        if (strcmp(argv[i], "desktopcheck") == 0 ||
            strcmp(argv[i], "--quiet") == 0 ||
            strcmp(argv[i], "--json") == 0) {
            continue;
        }
        if (strcmp(argv[i], "--config") == 0) {
            i++;
            continue;
        }
        if (strncmp(argv[i], "--shutdown=", 11) == 0 || strncmp(argv[i], "--bookmark=", 11) == 0) {
            continue;
        }
        value = option_value(argc, argv, &i, "--os", &present);
        if (present) {
            saw_os = TRUE;
            if (value == NULL) {
                missing_os_value = TRUE;
            } else {
                os_name = value;
            }
            continue;
        }
        value = option_value(argc, argv, &i, "--display", &present);
        if (present) {
            signals.display = value != NULL ? value : "";
            continue;
        }
        value = option_value(argc, argv, &i, "--gdk-backend", &present);
        if (present) {
            signals.gdk_backend = value != NULL ? value : "";
            continue;
        }
        value = option_value(argc, argv, &i, "--wayland", &present);
        if (present) {
            signals.wayland_display = value != NULL ? value : "";
            continue;
        }
        value = option_value(argc, argv, &i, "--session-type", &present);
        if (present) {
            signals.session_type = value != NULL ? value : "";
            continue;
        }
        finish_desktop(
            DESKTOP_UNKNOWN,
            FALSE,
            "desktopcheck does not accept that argument.\n"
            "Next: pass --os windows, --os macos, or --os linux. Linux Desktop also needs --display and, when the backend is not X11, --gdk-backend x11.");
    }
    if (!saw_os || missing_os_value || os_name == NULL || os_name[0] == '\0') {
        finish_desktop(
            DESKTOP_UNKNOWN,
            FALSE,
            "desktopcheck needs an operating-system name.\n"
            "Next: pass --os windows, --os macos, or --os linux.");
    }
    if (!text_is(os_name, "windows") && !text_is(os_name, "macos") && !text_is(os_name, "linux")) {
        finish_desktop(
            DESKTOP_UNKNOWN,
            FALSE,
            "That --os name is not windows, macos, or linux.\n"
            "Next: pass --os windows, --os macos, or --os linux.");
    }
    signals.os_name = os_name;
    {
        DesktopKind kind = desktop_kind_from_signals(&signals);
        if (kind == DESKTOP_UNKNOWN) {
            finish_desktop(
                DESKTOP_UNKNOWN,
                FALSE,
                "This combination is not Windows, macOS, or Linux Desktop.\n"
                "Next: for Linux Desktop pass --os linux, a non-empty --display, and leave --gdk-backend unset or set it to x11.");
        }
        finish_desktop(kind, TRUE, NULL);
    }
}
