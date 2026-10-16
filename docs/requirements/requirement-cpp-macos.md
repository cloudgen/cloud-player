**file**: docs/requirements/requirement-cpp-macos.md
**Status**: Active
**Updated**: 2026-10-11
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the macOS identity. The shared decision is `requirement-cpp-desktop.md`. Linux Desktop, the window this repository builds, is `requirement-cpp-linux-x11.md`. Windows is the third name in the desktop file. This repository does not produce a macOS binary and does not open the GTK window for macOS.

### 1.1 Human-facing

**In one sentence:** You can ask the program to name macOS, and that answer is not the Linux window.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person checking the three names | You run `desktopcheck --os macos` |
| The other role | The decision | It returns `macos` and ignores `DISPLAY` |
| Not this file | The X11 window | `requirement-cpp-linux-x11.md` |

| Includes | Excludes |
|----------|----------|
| The macOS token, the label `macOS`, and the rule that this tree has no macOS application bundle | A claim that `./build.sh` runs on macOS |

| Surface | What you open | What for |
|---------|---------------|----------|
| The terminal | `desktopcheck` | The token `macos`, then stop |
| `desktop` on a macOS compile | The same token from the compile target | No GTK window |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Name macOS from this Linux build | The check does not look at your display | `cloud-player desktopcheck --os macos` |
| Ask for JSON | One object whose desktop field is `macos` | `cloud-player --json desktopcheck --os macos` |

## 2. Core rules

1. The token is `macos`. The label is `macOS`. Both come from `requirement-cpp-desktop.md`.
2. `__APPLE__` is the compile target that makes `desktop` report `macos` without reading `DISPLAY` or `GDK_BACKEND`.
3. A macOS compile must not call `gtk_init()` and must not present itself as Linux Desktop.
4. This repository's `./build.sh`, Meson, Ant, and Gradle files are Linux builds. They link GTK 3 and WebKitGTK 4.1. They are not a macOS build.
5. Passing `--display` or `--gdk-backend` with `--os macos` still returns `macos`.

### 2.1 Implementation notes (this project)

There is no `.app` bundle, no Xcode project, and no Cocoa or macOS WebKit target in this tree. The Linux binary proves the branch with `desktopcheck`. A future macOS port would still use the same token and would still refuse this GTK window.

```text
cloud-player desktopcheck --os macos
cloud-player desktopcheck --os macos --display :0 --gdk-backend x11
cloud-player --json desktopcheck --os macos
```

The third command prints `{"desktop":"macos","label":"macOS"}`.

### 2.2 Why this requirement exists

- **CIAO Principle 5** (https://github.com/cloudgen/ciao): macOS is a named kind, not a second copy of the Linux rules.
- **CIAO Principle 1**: this tree does not claim a macOS binary it does not build.

## 3. Design principles

- **Caution**: a set `DISPLAY` does not turn macOS into Linux Desktop.
- **Intentional**: the label is `macOS`, the token is `macos`.
- **Anti-fragile**: the Linux build can still prove the macOS row.
- **Over-protect**: no macOS SDK path is invented here.

## 4. Design-time verification

| TP-ID | Suite | Status |
|-------|-------|--------|
| TP-DESKTOP-MACOS | `tests/test_desktop.sh` | have |
| TP-DESKTOP-SEPARATE | `tests/test_desktop.sh` | have |

**Map:** `docs/reviews/test-plan.md`.

## 5. Protection rule

Future editors must not document `./build.sh` as a macOS build, and must not map `__APPLE__` to `linux-desktop`.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-desktop.md` | The three names |
| `docs/requirements/requirement-cpp-linux-x11.md` | The window this tree builds |
| `src/main/cpp/desktop_env.cpp` | `__APPLE__` and `--os macos` |
| `tests/test_desktop.sh` | The checks |

**Last updated**: 2026-10-11
**Owner**: macOS identity
