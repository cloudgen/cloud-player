# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.0] - 2026-10-16

### Added
- Open File, beside Reload. It asks for one local file, writes that file's address into the bar, and then does the same thing as Go.
- README pictures of the cloud-player window and of the local `.mp4` view, including Open File.

### Changed
- The project name is `cloud-player`. `PROJECT_ID`, `PROJECT_NAME`, the binary, `project_name`, the window title, log file names, and the `~/.app/cloud-player/` and `/var/cloud-player/` directories replace `gemini-wilgat`. Existing data under `~/.app/gemini-wilgat/` is left in place.
- Product docs now describe cloud-player 1.1.0. The program you run is `cloud-player`. The window title is **cloud-player - Linux Desktop**.

## [1.0.8] - 2026-10-11

### Added
- One decision names `windows`, `macos`, or `linux-desktop` (labels Windows, macOS, and Linux Desktop).
- `gemini-wilgat desktop` names this process and exits. `gemini-wilgat desktopcheck --os windows|macos|linux` names a desktop from flags and does not read this process.
- The Linux Desktop window title adds ` - Linux Desktop`, and the window shows `Desktop: Linux Desktop`.
- Requirements for that decision, for the Linux X11 window, and for the macOS name.

### Changed
- The window opens only on Linux Desktop: a Linux build, a non-empty `DISPLAY`, and `GDK_BACKEND` unset or containing `x11`. Anything else exits before `gtk_init` with a `Next:` line.
- Linux builds (`./build.sh`, Meson, `ant compile`, and Gradle `linkDebug`) link libmpv from the host multiarch directory plus epoxy and EGL. Object files from `./build.sh` go under `target/exe/debug/obj/`. Sourcing `project.sh` does not install packages.
- Product docs now describe cloud-player 1.0.8. The program you run is still `gemini-wilgat`. The window title base is still Gemini (wilgat).

This repository still does not produce a macOS or Windows binary. A click on a file inside the page still does not start the video view.

## [1.0.7] - 2026-10-11

### Added
- Local `.mp4` playback in the window through libmpv when the address bar text ends in `.mp4` and you press Enter or Go.
- Public repository [cloudgen/cloud-player](https://github.com/cloudgen/cloud-player).
- README pictures of the Gemini window and of that `.mp4` view.

### Changed
- Product docs now describe cloud-player 1.0.7. The program you run is still `gemini-wilgat`, and the window title is still Gemini (wilgat).
- Version surfaces that had drifted to 1.0.6 (`README.md`, this file, `project.ini`) now match `src/main/cpp/project.h`.

A click on a file inside the page still does not start the video view.

## [1.0.6] - 2026-04-18

### Added
- RECOMMENDATION.md

## [1.0.5] - 2026-04-18

### Added
- Persistent User-Agent settings using `settings.sqlite` (stored alongside cookies).
- User-Agent selected at startup now correctly displays in the UI label (previously showed default Chrome until changed).
- `webkit_setup.c/h` module — Single Point of Truth for all WebKitWebContext, cookie, and User-Agent initialization.
- `ui_window_gemini.c/h` and `ui_menu_gemini.c/h` modules for better separation of concerns.
- Strong CIAO defensive protection comments on critical functions (`create_main_window()`, `build_main_menu_bar()`, etc.).
- `PROJECT_DESCRIPTION` constant in `project.h` (Single Source of Truth).

### Changed
- `main.c` is now pure orchestration (very clean and short).
- `add_url_bar_user_agent()` now accepts an `initial_ua` parameter to respect persisted settings.
- Enhanced protection rules to preserve GitHub-specific UI modules when forking to other projects.
- Updated version dialog to use centralized `PROJECT_DESCRIPTION`.

### Fixed
- UI label under URL bar now shows correct persisted User-Agent on application startup.
- Duplicate loading of User-Agent settings at startup.

---

## [1.0.3] - 2026-04-18

### Added
- Full User-Agent spoofing menu (Chrome, Firefox, Safari, Edge) with Chrome as default.
- Persistent cookie storage using SQLite.
- Advanced output system with `--quiet`, `--json`, and `DEBUG=1` support.
- Generic bookmark system + GitHub-specific bookmarks.
- Real-time URL bar synchronization (`notify::uri` + `load-changed` fallback).
- Enter key support in URL bar.

### Changed
- Migrated to strong CIAO Defensive Programming style across the entire codebase.
- All User-Agent strings centralized in `project.h`.

---

## [1.0.0] - 2026-04-14

### Added
- Initial release of lightweight native GitHub wilgat using GTK3 + WebKit2GTK.
- Basic URL bar and navigation.
- Defensive coding foundation (CIAO principles).

---

## How to Maintain This File

When releasing a new version:
1. Move `[Unreleased]` content into a new version header with today's date.
2. Add a fresh `[Unreleased]` section at the top.
3. Use the standard categories: **Added**, **Changed**, **Fixed**, etc.

---

**Project**: [cloudgen/cloud-player](https://github.com/cloudgen/cloud-player)  
**Philosophy**: Built with strict CIAO Defensive Programming Principles

---

Made with ❤️ and extreme caution.