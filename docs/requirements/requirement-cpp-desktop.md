**file**: docs/requirements/requirement-cpp-desktop.md
**Status**: Active
**Updated**: 2026-10-16
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the one decision that names the desktop: Windows, macOS, or Linux Desktop. `desktop` reports the process. `desktopcheck` reports a supplied combination and does not read the process. The Linux X11 rules for the window are `requirement-cpp-linux-x11.md`. The macOS identity is `requirement-cpp-macos.md`. Windows is a peer of those two in the table below. This tree opens the GTK window only for Linux Desktop. The Visual C++ executable is `requirement-cpp-windows-player.md`. It does not compile `desktop_env.cpp`.

### 1.1 Human-facing

**In one sentence:** You run `desktop` and the program answers `windows`, `macos`, or `linux-desktop`.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person who asks which desktop this is | You run `cloud-player desktop` |
| The other role | The decision in `desktop_env.cpp` | It returns one of the three names, or `unknown` |
| Not this file | How the X11 window is prepared, and what a macOS build is | `requirement-cpp-linux-x11.md` and `requirement-cpp-macos.md` |

| Includes | Excludes |
|----------|----------|
| The three names, `desktop`, `desktopcheck`, the window title suffix, and the Desktop label | Package names (`requirement-cpp-dependencies.md`) and the WebKit load order (`requirement-cpp-window.md`) |

| Surface | What you open | What for |
|---------|---------------|----------|
| The terminal | `desktop` and `desktopcheck` | The name, then the process stops |
| The window | Title and the Desktop label | Linux Desktop, after the X11 check passes |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Ask this process | The answer is one token and the process exits | `cloud-player desktop` |
| Ask for another desktop | The answer does not use this process's display | `cloud-player desktopcheck --os macos` |
| Open the window | The title ends with the desktop label | `cloud-player` on Linux Desktop |

## 2. Core rules

Exactly one function, `desktop_kind_from_signals()` in `src/main/cpp/desktop_env.cpp`, chooses the kind. Tokens and labels:

| Kind | Token | Label | When |
|------|-------|-------|------|
| Windows | `windows` | Windows | `os` is `windows`. Display and backend are ignored |
| macOS | `macos` | macOS | `os` is `macos`. Display and backend are ignored |
| Linux Desktop | `linux-desktop` | Linux Desktop | `os` is `linux`, `DISPLAY` is non-empty, and `GDK_BACKEND` is empty or contains the token `x11` |
| unknown | `unknown` | unknown | Any other `os`, a blank `DISPLAY` on Linux, or a Linux backend that has no `x11` token |

`WAYLAND_DISPLAY` and `XDG_SESSION_TYPE` may be supplied. They do not change the row. An empty `GDK_BACKEND` on Linux with a display means the program will select X11. That selection is `requirement-cpp-linux-x11.md`. A backend of `wayland` is not Linux Desktop, even when `XDG_SESSION_TYPE` is `x11`.

The compile target fills `os` when the caller does not pass one: `_WIN32` is `windows`, `__APPLE__` is `macos`, `__linux__` is `linux`.

`desktop` is an operational command. It prepares the Linux X11 backend, then prints the live token and exits. It does not open the window. Extra arguments are a failure. Exit 0 for the three kinds. Exit 1 for `unknown`, with a `Next:` line.

`desktopcheck` is a test-purpose command. Help lists it under Tests, apart from `desktop`. It does not read the process environment and does not open the window. It requires `--os windows`, `--os macos`, or `--os linux`. Optional: `--display`, `--gdk-backend`, `--wayland`, `--session-type`. Exit codes match `desktop`.

`--json` prints one object, `{"desktop":"...","label":"..."}`, and on failure adds `error`. Text mode prints the token on the first line. A failure prints `unknown`, then what happened, then a line that starts with `Next:`.

On Linux Desktop the window title is the configured title, a space, a hyphen, a space, and `Linux Desktop`. A label in the window reads `Desktop: Linux Desktop`. The base title stays `requirement-cpp-window.md`.

A Windows or macOS compile of these GTK sources does not open this GTK window. It can still answer `desktop`. The Visual C++ executable does not compile `desktop_env.cpp` and does not answer `desktop`.

### 2.1 Implementation notes (this project)

Binary: `cloud-player`. The decision lives in `src/main/cpp/desktop_env.cpp`.

```text
cloud-player desktop
cloud-player desktopcheck --os windows
cloud-player desktopcheck --os macos
cloud-player desktopcheck --os linux --display :0
cloud-player --json desktopcheck --os macos
```

### 2.2 Why this requirement exists

- **CIAO Principle 5** (https://github.com/cloudgen/ciao): one function names the desktop. The Linux and macOS files point here.
- **CIAO Principle 1**: `unknown` exits 1 and says what to do next.

## 3. Design principles

- **Caution**: Windows and macOS do not become Linux Desktop because `DISPLAY` is set.
- **Intentional**: three names, one function.
- **Anti-fragile**: a Wayland-only backend stays `unknown` instead of being relabeled.
- **Over-protect**: `desktopcheck` cannot see this process, so the three rows can be checked on one machine.

## 4. Design-time verification

| TP-ID | Suite | Status |
|-------|-------|--------|
| TP-DESKTOP-WINDOWS | `tests/test_desktop.sh` | have |
| TP-DESKTOP-MACOS | `tests/test_desktop.sh` | have |
| TP-DESKTOP-LINUX-X11 | `tests/test_desktop.sh` | have |
| TP-DESKTOP-SEPARATE | `tests/test_desktop.sh` | have |
| TP-DESKTOP-LIVE | `tests/test_desktop.sh` | have |

**Map:** `docs/reviews/test-plan.md`.

## 5. Protection rule

Future editors must not add a fourth desktop name, let `WAYLAND_DISPLAY` override the table, or open the GTK window for `unknown`, Windows, or macOS.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-linux-x11.md` | X11 selection and the window gate |
| `docs/requirements/requirement-cpp-macos.md` | macOS identity |
| `docs/requirements/requirement-cpp-windows-player.md` | Visual C++ executable. It does not answer `desktop` |
| `docs/requirements/requirement-cpp-parameters.md` | `desktop` and `desktopcheck` in the flag list |
| `docs/requirements/requirement-cpp-window.md` | Base window title |
| `src/main/cpp/desktop_env.cpp` | The decision |
| `tests/test_desktop.sh` | The checks |
| `docs/reviews/test-plan.md` | TP status |

**Last updated**: 2026-10-11
**Owner**: Desktop identity
