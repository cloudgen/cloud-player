**file**: docs/requirements/requirement-cpp-windows-player.md
**Status**: Active
**Updated**: 2026-10-16
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns `cloud-player.exe`: the Win32 and WebView2 program. The toolchain is `requirement-cpp-windows-environment.md`. The Linux GTK window, its menu, and its verbs stay in `requirement-cpp-window.md`, `requirement-cpp-ui-menu.md`, and `requirement-cpp-parameters.md`. The three desktop names for the Linux binary stay in `requirement-cpp-desktop.md`. This executable does not compile those GTK sources.

### 1.1 Human-facing

**In one sentence:** You start `cloud-player.exe` on the signed-in Windows desktop and it opens Gemini in a WebView2 window, with Open File on the address bar, or you pass `--check` and it proves the profile without a window.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person at the Windows desktop, or at a console | You run `cloud-player.exe` or `cloud-player.exe --check` |
| The other role | The WebView2 window | It loads the start page, plays a local `.mp4` you open, and keeps its data under the profile |
| Not this file | The GTK window and libmpv | `requirement-cpp-window.md` |

| Includes | Excludes |
|----------|----------|
| Windows identity, the profile, the console flags, the menu, Open File, in-window `.mp4` playback, and `--check` | `desktop`, `log`, and `luascript` verbs, linking libmpv, and sol2 script execution |

| Surface | What you open | What for |
|---------|---------------|----------|
| `build-win\Debug\cloud-player.exe` | The window, on the signed-in desktop | Gemini in WebView2 |
| The same executable with `--check` | A console | Profile and WebView2 environment, then stop |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Prove the build without a window | The process creates the profile and exits | `cloud-player.exe --check` |
| Read the flags | Help lists the Windows set and exits | `cloud-player.exe help` |
| Open the page | The window appears on the signed-in desktop | `cloud-player.exe` |
| Open a local file | The address bar receives that file's address, then the same action as Go runs | Click Open File |

## 2. Core rules

`src/win/player_project.h` includes `src/main/cpp/project.h` and replaces only these macros:

| Macro | Windows value |
|-------|----------------|
| `PROJECT_DESCRIPTION` | `Gemini and local pages in a WebView2 window on Windows.` |
| `PERSISTENCE_STORAGE_PATH` | `.app/cloud-player/cookies` |
| `PLAYER_DEFAULT_USER_AGENT` | `UA_EDGE` |

`PROJECT_ID` and `PROJECT_NAME` stay `cloud-player`. Version macros stay in `project.h`. User-agent string literals stay in `project.h`. The window classes are `CloudPlayerWindow` and `CloudPlayerPopup`. The popup title is `cloud-player`. Message box titles that name the product say `cloud-player`.

The profile root is `%USERPROFILE%\.app\cloud-player\`. When `USERPROFILE` is empty the process uses `HOME`. When both are empty it logs that `USERPROFILE` is not set and uses `C:\Users\Public`. Under that root:

| Path | Role |
|------|------|
| `config.conf` | Created with defaults when missing |
| `settings.sqlite` | Saved user agent. The Windows build uses the vendored SQLite amalgamation |
| `webview2\` | WebView2 user data, including cookies |
| `log\` | Log directory. File → Log → Log folder opens it |
| `userscripts\` | Folder only. File → Lua Script → Folder opens it. This executable does not load sol2 |
| `cookies\` | Directory created for the profile layout. WebView2 does not write `cookies.sqlite` here |

Flag, then environment, then the config file. A flag the operator passed is not replaced by the config file. Help lists:

```text
cloud-player [help] [--check] [--quiet] [--json] [--shutdown=N] [--bookmark=N] [--config PATH]
```

Environment names printed by help: `DEBUG`, `SHUTDOWN`, `BOOKMARK`, `QUIET`, `JSON`. `help`, `--help`, and `-h` print that text and exit 0. They do not open a window.

`--check` creates the profile, creates the WebView2 environment in the `webview2` folder, prints `check passed`, and exits 0. It does not open a window. A failed environment exits 1. The pass line from the 2026-10-16 run also printed the runtime version before `check passed`.

The menu bar, when bookmarks are enabled:

| Menu | Items |
|------|--------|
| File | AutoClick all bots, Bookmark → Start Page, User Agent, Lua Script → Folder, Log → Log folder, Exit |
| About | Show Version |

User Agent order: Webview (Default), Google Chrome, Mozilla Firefox, Apple Safari, Microsoft Edge. The process default is Microsoft Edge (`UA_EDGE`). AutoClick does not navigate. It logs that the sequence finished. Bookmark → Start Page loads `https://gemini.google.com`. Show Version shows `cloud-player`, the version from `project.h`, and the Windows description.

When `show_navigation_buttons` is true, the address bar is Back, Forward, Reload, Open File, the address edit, and Go. Open File sits beside Reload. Its label is `Open File`. It asks for one local file. The dialog title is `Open File`. Cancel, or closing the dialog, leaves the edit unchanged. Accept writes that file's `file:` URI into the edit, logs `Open file set URL to:`, and then runs the same action as Go. The chooser is closed before that action runs. Open File stays enabled when `navigation_buttons_enabled` is false. Back, Forward, Reload, Go, and the edit follow that flag. The whole bar, including Open File, is hidden when `show_navigation_buttons` is false. When the edit already holds a `file:` URI, the chooser starts at that path.

Go, and Enter in the edit, treat an address whose path ends in `.mp4` as video. The query and the fragment are ignored for that suffix, and a trailing slash is ignored. The video plays in the WebView2 window. For a local file, the address bar keeps that `file:` URI while the view plays it. This executable does not link libmpv and does not compile `video_player.cpp`. A local file is played by mapping its folder to the host `cloud-player.local` with cross-origin access denied, then opening that file in the view. That mapping stays until another local video replaces it. If the mapping cannot be set, the view opens the `file:` address directly. A remote `.mp4` address is opened in the view as that address. Any other address loads as a page. This executable does not run the Linux `.log` checker. When the file cannot be played, the process logs `Cannot play this mp4 address.` and shows a message box titled `Video`. A controller failure `0x80070578` still does not show a message box.

`--bookmark=0` opens the start page. Any other index warns and still opens the start page. `--shutdown=N` closes the window after N seconds. `--config PATH` reads that file instead of the profile `config.conf`. `--quiet` hides info, warning, and debug on the console. `--json` is quiet on the console. Stored log lines are still appended.

A new window requested by the page opens a popup titled `cloud-player`. Browser arguments include `--autoplay-policy=no-user-gesture-required`. Permission requests are allowed.

The window is shown on an interactive desktop. An SSH session is a different window station. When `CreateCoreWebView2Controller` returns `0x80070578`, the process logs that the window is not on an interactive desktop and exits with code 1. It does not show a message box on that path. Other controller failures may show a message box and then exit 1. A missing Evergreen runtime exits 1 before a window is shown. A visible window is proved on the signed-in desktop, not by an SSH session that only ran `--check`.

This executable does not implement `desktop`, `desktopcheck`, `log`, `logcheck`, or `luascript`. It does not link libmpv. It does not run the GTK window.

### 2.1 Implementation notes (this project)

Binary: `build-win\Debug\cloud-player.exe`. Version inherited from `project.h`: 1.2.0. Start page: `https://gemini.google.com`.

```text
cloud-player.exe help
cloud-player.exe --check
cloud-player.exe --shutdown=30
cloud-player.exe --bookmark=0
cloud-player.exe --config C:\path\to\config.conf
```

`--check` on 2026-10-16 created the default config, saved the default user agent, reported WebView2 runtime 155.0.4283.45, created the environment, reported `settings.sqlite` ready, printed `check passed`, and exited 0. A visible window was not opened in that session.

### 2.2 Why this requirement exists

- **CIAO Principle 5** (https://github.com/cloudgen/ciao): one Windows program. The Linux verbs stay on the Linux binary.
- **CIAO Principle 1**: `--check` and a non-interactive desktop say what happened and exit, instead of waiting on a message box.

## Under command line for normal user only

**This requirement:** `cloud-player.exe` runs as the Windows login. It does not call `sudo`, does not run `apt`, and does not create a dedicated system account. Git Bash may start the executable. That shell does not run Termux `pkg`. Admin privilege and a dedicated system user privilege are unused.

## 3. Design principles

- **Caution**: identity macros that already say `cloud-player` stay in `project.h`.
- **Intentional**: `--check` is the console proof. The window is a separate proof.
- **Anti-fragile**: a non-interactive desktop logs `0x80070578` and exits.
- **Over-protect**: libmpv, sol2, and the GTK `desktop` verb stay on Linux. Open File on this executable does not link that Linux media library.

## 4. Design-time verification

| TP-ID | Pass | Status |
|-------|------|--------|
| TP-WIN-CHECK | `cloud-player.exe --check` prints `check passed` and exits 0. It does not open a window | have |
| TP-WIN-PLAYER | On the signed-in Windows desktop the window opens, the title is `cloud-player`, and the start page loads. An SSH session is not this proof | todo |
| TP-WIN-OPEN | On the signed-in Windows desktop, Open File is on the bar after Reload. One local file writes its `file:` URI into the bar, then Go runs. Cancel leaves the bar unchanged. An address whose path ends in `.mp4` plays in the window. An SSH session is not this proof | todo |

`TP-WIN-CHECK` was observed on 2026-10-16. **Map:** `docs/reviews/test-plan.md`.

## 5. Protection rule

Future editors must not point `PROJECT_ID` or `PROJECT_NAME` at a different product, move user-agent string literals out of `project.h`, remove Open File, link libmpv, or compile `video_player.cpp` into this executable. They must not show a message box when the controller returns `0x80070578`, or treat a passing `--check` as proof that the window was visible.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-windows-environment.md` | Toolchain and link set |
| `docs/requirements/requirement-cpp-dual-build.md` | Entry points |
| `docs/requirements/requirement-cpp-desktop.md` | Desktop names for the Linux binary |
| `docs/requirements/requirement-cpp-window.md` | Linux GTK window |
| `docs/requirements/requirement-cpp-ui-menu.md` | Linux menu |
| `docs/requirements/requirement-cpp-parameters.md` | Linux flags and verbs |
| `docs/reviews/test-plan.md` | TP-WIN-CHECK, TP-WIN-PLAYER, TP-WIN-OPEN |
| `src/win/player_project.h` | Windows identity |
| `src/win/win_app.cpp` | Window, menu, and `--check` |
| `src/win/win_host.cpp` | Profile and help text |
| `src/main/cpp/project.h` | Shared id, version, and user-agent strings |

**Last updated**: 2026-10-16
**Owner**: Windows player
