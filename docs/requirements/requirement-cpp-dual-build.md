**file**: docs/requirements/requirement-cpp-dual-build.md
**Status**: Active
**Updated**: 2026-10-16
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns which command produces which cloud-player binary. Linux keeps `./build.sh`. Windows keeps `build-win.bat`. Each host compiles only its own engine. The Windows toolchain is `requirement-cpp-windows-environment.md`. The Windows program is `requirement-cpp-windows-player.md`. The Linux link set is `requirement-cpp-dependencies.md`. This tree does not produce a macOS binary (`requirement-cpp-macos.md`).

### 1.1 Human-facing

**In one sentence:** On Linux you run `./build.sh build` and get the GTK program; on Windows you run `build-win.bat` and get `cloud-player.exe`.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person building on one host | You use the command for that host |
| The other role | The build for that host | It compiles one engine |
| Not this file | Package names, and what the window shows | `requirement-cpp-dependencies.md` and `requirement-cpp-windows-player.md` |

| Includes | Excludes |
|----------|----------|
| The two entry points, the two outputs, and the rule that CMake is Windows-only here | A Meson, Make, Ant, or Gradle Windows branch, and a macOS binary |

| Surface | What you open | What for |
|---------|---------------|----------|
| `./build.sh` | `build` | `target/exe/debug/cloud-player` on Linux |
| `build-win.bat` | Visual Studio 2022 | `build-win\Debug\cloud-player.exe` on Windows |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Build on Linux | GTK 3 and WebKitGTK, sources under `src/main/cpp` | `./build.sh build` |
| Build on Windows | Win32 and WebView2, sources under `src/win` | `build-win.bat` |

## 2. Core rules

Two commands. `./build.sh` does not call `build-win.bat`. `build-win.bat` does not call `./build.sh`.

| Host | Command | Output | Engine |
|------|---------|--------|--------|
| Linux | `./build.sh build` | `target/exe/debug/cloud-player` | GTK 3, WebKitGTK 4.1, and the other libraries in `requirement-cpp-dependencies.md`. Sources are the Linux translation units |
| Windows | `build-win.bat` | `build-win\Debug\cloud-player.exe` next to `WebView2Loader.dll` | Win32 and WebView2. Sources are `src/win` plus `third_party/sqlite/sqlite3.c` |

`CMakeLists.txt` is the Windows build description. On a host that is not Windows it stops with a fatal error that names `./build.sh`. It is not a second way to build the GTK program.

Meson, GNU Make, Apache Ant, and Gradle are not given a Windows branch by this file. Their existing Linux status stays in their own requirements. `./build.sh` still implements `install`, `clean`, and `build` only.

The product id is `cloud-player` on both hosts. The version number is the one in `src/main/cpp/project.h`. A version change is made there so both hosts follow, and only when a version bump was asked for.

Building the Windows executable on a Windows machine with Visual Studio is a native build. It is not a cross-compile from the Linux tree. A Linux distribution installed on Windows is not this Windows build.

A passing Windows build is exit 0 from `build-win.bat` and the executable on disk. A passing no-window check is `cloud-player.exe --check` printing `check passed` and exiting 0. A visible window is a separate proof and is not required for the build to count as passed.

### 2.1 Implementation notes (this project)

```text
./build.sh build
build-win.bat
build-win\Debug\cloud-player.exe --check
```

The Windows build and `--check` passed on 2026-10-16. The operator named that Windows login `leolio@win` and the tree `prjs\cloud-player`. The Linux command remains `./build.sh build` on a Linux machine with the packages in `requirement-cpp-dependencies.md`.

### 2.2 Why this requirement exists

- **CIAO Principle 5** (https://github.com/cloudgen/ciao): one table of entry points. Each host has one command and one engine.
- **CIAO Principle 1**: CMake on Linux stops with the name of the Linux script, instead of linking a partial program.

## Under command line for normal user only

**This requirement:** each command runs as the login on that host. `build-win.bat` does not call `sudo`, does not run `apt`, and does not install Visual Studio. Git Bash on Windows may start `cmd.exe /c build-win.bat`. That shell does not run Termux `pkg`. Admin privilege and a dedicated system user privilege are unused for the Windows build. `./build.sh install` on Linux may still install the host packages named in `requirement-cpp-dependencies.md`. That install step is not the Windows build.

## 3. Design principles

- **Caution**: the GTK compile lines stay on Linux. They are not rewritten for MSVC.
- **Intentional**: two commands, chosen by the host the person is on.
- **Anti-fragile**: a Windows machine without GTK packages can still build `cloud-player.exe`.
- **Over-protect**: a partial source list in CMake does not become a second Linux binary.

## 4. Design-time verification

| TP-ID | Pass | Status |
|-------|------|--------|
| TP-WIN-ENV | `build-win.bat` exits 0 and the executable is beside `WebView2Loader.dll` | have |
| TP-WIN-CHECK | `cloud-player.exe --check` prints `check passed` and exits 0 | have |

The Linux `./build.sh build` proof stays with the Linux build requirements. **Map:** `docs/reviews/test-plan.md`.

## 5. Protection rule

Future editors must not make `./build.sh` compile `src/win` with the Linux compiler, make `build-win.bat` compile the GTK sources, add a Windows branch to Meson or GNU Make from this file, or treat a Linux compiler on Windows as `cloud-player.exe`.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-windows-environment.md` | Visual C++ toolchain |
| `docs/requirements/requirement-cpp-windows-player.md` | The Windows program |
| `docs/requirements/requirement-cpp-dependencies.md` | Linux link set |
| `docs/requirements/requirement-cpp-macos.md` | No macOS binary |
| `docs/requirements/requirement-cpp-skills-installation-basic.md` | `./build.sh` |
| `docs/reviews/test-plan.md` | TP-WIN-ENV, TP-WIN-CHECK |
| `build.sh` | Linux build |
| `build-win.bat` | Windows build |
| `CMakeLists.txt` | Windows-only CMake file |
| `src/main/cpp/project.h` | Shared version |

**Last updated**: 2026-10-16
**Owner**: Dual build
