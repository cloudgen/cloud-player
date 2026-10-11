// desktop_env.h
// =========================================================================
// CIAO DEFENSIVE CODING STYLE - DESKTOP IDENTITY
// =========================================================================
//
// Law: requirement-cpp-desktop.md
//      requirement-cpp-linux-x11.md
//      requirement-cpp-macos.md
//
// One decision names windows, macos, or linux-desktop. The GTK window
// in this tree opens only for linux-desktop. desktopcheck is test-purpose
// and does not open the window.
// Last aligned: 2026-10-11
// =========================================================================

#ifndef DESKTOP_ENV_H
#define DESKTOP_ENV_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DESKTOP_UNKNOWN = 0,
    DESKTOP_WINDOWS = 1,
    DESKTOP_MACOS = 2,
    DESKTOP_LINUX = 3
} DesktopKind;

/* NULL pointers mean "not provided". An empty string is provided and blank.
   os_name NULL means the compile target (windows, macos, or linux). */
typedef struct {
    const char *os_name;
    const char *display;
    const char *gdk_backend;
    const char *wayland_display;
    const char *session_type;
} DesktopSignals;

DesktopKind desktop_kind_from_signals(const DesktopSignals *signals);
const char *desktop_kind_token(DesktopKind kind);
const char *desktop_kind_label(DesktopKind kind);

/* Compile target plus the process environment. */
void desktop_signals_from_environment(DesktopSignals *out);
DesktopKind desktop_kind_live(void);

/* Linux only: when DISPLAY is set and GDK_BACKEND is unset, select x11.
   Does not replace an explicit GDK_BACKEND. Call before gtk_init(). */
void desktop_prepare_linux_x11(void);

gboolean ciao_argv_has_desktop_verb(int argc, char **argv);
gboolean ciao_argv_has_desktopcheck_verb(int argc, char **argv);

/* Print the token and exit. Call before gtk_init(). */
void ciao_desktop_command_dispatch(int argc, char **argv);
void ciao_desktopcheck_command_dispatch(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* DESKTOP_ENV_H */
