**file**: docs/requirements/requirement-cpp-linux-x11.md
**Status**: Active
**Updated**: 2026-10-11
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the Linux Desktop runtime: an X11 display, the GDK backend this program selects, and the refusal to open the window when that display is missing. The three-way name is `requirement-cpp-desktop.md`. macOS is `requirement-cpp-macos.md`. The libraries are `requirement-cpp-dependencies.md`.

### 1.1 Human-facing

**In one sentence:** On Linux you start the program with an X11 display, and the window comes up as Linux Desktop.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person at a Linux graphical session | You start `cloud-player` |
| The other role | The process, before GTK starts | It selects the X11 backend when you have not chosen one |
| Not this file | The names Windows and macOS | `requirement-cpp-desktop.md` |

| Includes | Excludes |
|----------|----------|
| `DISPLAY`, `GDK_BACKEND=x11`, and the message when the window must not open | Which bookmark loads (`requirement-cpp-webui-bookmark.md`) |

| Surface | What you open | What for |
|---------|---------------|----------|
| The terminal | Environment and the program | An X11 window, or a `Next:` line |
| The window | Title ending in `Linux Desktop` | The Gemini page |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Leave the backend unset | The program sets `GDK_BACKEND` to `x11` when `DISPLAY` is set | `cloud-player` |
| Force another backend | The window does not open | `GDK_BACKEND=wayland cloud-player` |

## 2. Core rules

1. The window opens only when `desktop_kind_live()` is Linux Desktop. That rule is the table in `requirement-cpp-desktop.md`.
2. On Linux, when `DISPLAY` is non-empty and `GDK_BACKEND` is unset or empty, `desktop_prepare_linux_x11()` sets `GDK_BACKEND` to `x11` before `gtk_init()`. An explicit `GDK_BACKEND` is left as the operator set it.
3. When the kind is not Linux Desktop, `main()` does not call `gtk_init()`. It prints that this session is not Linux Desktop (X11), then a line that starts with `Next:` telling the operator to set `DISPLAY` and leave `GDK_BACKEND` unset or set `GDK_BACKEND=x11`. The process exits 1.
4. `desktop` uses the same preparation, then exits without a window.
5. This file does not name a display id. The program reads `DISPLAY` from the environment.

### 2.1 Implementation notes (this project)

The call is in `src/main/cpp/main.cpp`, after configuration and before `gtk_init()`. The helper is `desktop_prepare_linux_x11()` in `src/main/cpp/desktop_env.cpp`.

The build that produces this window is Linux x86_64 with GTK 3 and WebKitGTK 4.1. `./build.sh` and `meson.build` are that build. A Wayland-only `GDK_BACKEND` is not this runtime.

### 2.2 Why this requirement exists

- **CIAO Principle 2** (https://github.com/cloudgen/ciao): the window is an X11 client on purpose, including when a Wayland session also publishes `DISPLAY`.
- **CIAO Principle 1**: a missing X11 display stops before GTK, with a next step.

## 3. Design principles

- **Caution**: do not replace an explicit `GDK_BACKEND`.
- **Intentional**: empty backend plus a display means X11.
- **Anti-fragile**: the window does not start on a backend the decision calls `unknown`.
- **Over-protect**: no display number is written into this file.

## 4. Design-time verification

| TP-ID | Suite | Status |
|-------|-------|--------|
| TP-DESKTOP-LINUX-X11 | `tests/test_desktop.sh` | have |
| TP-DESKTOP-LIVE | `tests/test_desktop.sh` | have |

**Map:** `docs/reviews/test-plan.md`.

## 5. Protection rule

Future editors must not call `gtk_init()` when the kind is `unknown`, and must not hard-code a display id.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-desktop.md` | The three names |
| `docs/requirements/requirement-cpp-macos.md` | The other identity |
| `docs/requirements/requirement-cpp-dependencies.md` | GTK and WebKit |
| `src/main/cpp/main.cpp` | The gate before `gtk_init()` |
| `src/main/cpp/desktop_env.cpp` | `desktop_prepare_linux_x11()` |
| `tests/test_desktop.sh` | The checks |

**Last updated**: 2026-10-11
**Owner**: Linux X11 runtime
