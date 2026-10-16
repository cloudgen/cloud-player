**file**: docs/requirements/requirement-cpp-windows-environment.md
**Status**: Active
**Updated**: 2026-10-16
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the Visual C++ toolchain and the libraries `cloud-player.exe` links. The Linux libraries stay in `requirement-cpp-dependencies.md`. What the Windows program does is `requirement-cpp-windows-player.md`. Which command runs on which host is `requirement-cpp-dual-build.md`.

### 1.1 Human-facing

**In one sentence:** You run `build-win.bat` on a Windows login that already has Visual Studio 2022, and it produces `cloud-player.exe` with the WebView2 loader beside it.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person who builds on Windows | You run `build-win.bat` |
| The other role | `cl.exe` from Visual Studio 2022 | It compiles `src/win` and the SQLite amalgamation |
| Not this file | The GTK packages, and what the window shows | `requirement-cpp-dependencies.md` and `requirement-cpp-windows-player.md` |

| Includes | Excludes |
|----------|----------|
| The compiler, the SDK, the generator, the compile flags, and the Windows link set | GTK, WebKitGTK, Lua, libmpv, and a Linux build of this CMake file |

| Surface | What you open | What for |
|---------|---------------|----------|
| `build-win.bat` | Visual Studio environment, then CMake | `build-win\Debug\cloud-player.exe` |
| `CMakeLists.txt` | The Windows executable only | The same link set |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Build the Windows program | The script loads the 64-bit Visual Studio tools and builds Debug | `build-win.bat` |
| Confirm the pieces | The executable sits next to `WebView2Loader.dll` | look in `build-win\Debug` |

## 2. Core rules

`build-win.bat` is the Windows build. It calls `vcvars64.bat` from Visual Studio 2022 Community, then the CMake executable that ships inside that Visual Studio install. Configure uses `-G "Visual Studio 17 2022" -A x64`. The build uses `--config Debug`. The script stops if `vcvars64.bat` or that CMake executable is missing. It does not download a compiler.

`CMakeLists.txt` configures only when the host is Windows. On any other host it stops with a fatal error that names `./build.sh`. It does not emit a Linux executable.

The executable target is `cloud-player`. It compiles these files and no others:

| File | Language |
|------|----------|
| `src/win/win_log.cpp` | C++ |
| `src/win/win_host.cpp` | C++ |
| `src/win/win_app.cpp` | C++ |
| `third_party/sqlite/sqlite3.c` | C |

Include directories are `src/win`, `src/main/cpp` (headers only), `third_party/sqlite`, and `third_party/webview2/include`. `src/main/cpp` translation units are not in this target.

C++ is C++17. These definitions are on the target: `UNICODE`, `_UNICODE`, `NOMINMAX`, `WIN32_LEAN_AND_MEAN`, `SQLITE_THREADSAFE=1`, `SQLITE_OMIT_LOAD_EXTENSION`, `_CRT_SECURE_NO_WARNINGS`. MSVC flags are `/W3`, `/EHsc`, and `/utf-8`. `sqlite3.c` is compiled with `/w` for that file only. A compiler note that `/W3` was overridden by `/w` on `sqlite3.c` is still a successful build when the link exits 0.

The link line is the vendored import library `third_party/webview2/x64/WebView2Loader.dll.lib`, then `user32`, `gdi32`, `ole32`, `oleaut32`, `shell32`, `shlwapi`, `advapi32`, and `version`. After the link, the build copies `third_party/webview2/x64/WebView2Loader.dll` next to the executable.

The output is `build-win\Debug\cloud-player.exe` with `WebView2Loader.dll` in the same directory. `build-win/` is ignored by git. The executable is a console program so help and `--check` print on the console.

Vendored inputs, committed so the Windows build does not fetch them:

| Piece | Path |
|-------|------|
| WebView2 headers | `third_party/webview2/include/WebView2.h`, `third_party/webview2/include/WebView2EnvironmentOptions.h` |
| WebView2 loader | `third_party/webview2/x64/WebView2Loader.dll`, `third_party/webview2/x64/WebView2Loader.dll.lib` |
| SQLite amalgamation | `third_party/sqlite/sqlite3.c`, `third_party/sqlite/sqlite3.h`, `third_party/sqlite/sqlite3ext.h` |

The Windows build does not require GTK, WebKitGTK, GLib, Lua, sol2, libmpv, pkg-config, Meson, Ninja, MSYS2 `gcc`, vcpkg, or a Linux distribution on the Windows machine. Git Bash may be the shell that starts `cmd.exe`. Git Bash is not the compiler.

### 2.1 Implementation notes (this project)

`build-win.bat` uses this Visual Studio install:

```text
C:\Program Files\Microsoft Visual Studio\2022\Community
```

`vcvars64.bat` is `VC\Auxiliary\Build\vcvars64.bat` under that install. CMake is `Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe` under that install. `CMakeLists.txt` requires CMake 3.20. The bundled CMake version string was not printed by the 2026-10-16 build, so this file does not pin one.

Probe recorded from the successful Debug build on 2026-10-16. The operator named the Windows login `leolio@win` and the tree `prjs\cloud-player` on that account.

| Pin | Value |
|-----|--------|
| Developer prompt | Visual Studio 2022 v17.7.5 |
| MSVC | 19.37.32825.0 |
| Toolset directory | `VC\Tools\MSVC\14.37.32822` |
| Windows SDK selected | 10.0.22621.0, targeting 10.0.26200 |
| MSBuild | 17.7.2 |
| Generator | Visual Studio 17 2022, platform x64, config Debug |
| WebView2 header product version | 154.0.4258.31 (`CORE_WEBVIEW_TARGET_PRODUCT_VERSION` in `WebView2EnvironmentOptions.h`) |
| WebView2 runtime printed by `--check` | 155.0.4283.45 |
| SQLite | 3.54.0, `SQLITE_VERSION_NUMBER` 3054000 |

The NuGet package version of the vendored loader was not recorded. This file does not invent one.

### 2.2 Why this requirement exists

- **CIAO Principle 5** (https://github.com/cloudgen/ciao): one Windows toolchain and one Windows link set. The Linux link set stays in `requirement-cpp-dependencies.md`.
- **CIAO Principle 1**: a missing `vcvars64.bat` or a missing bundled CMake stops the script before it compiles.

## Under command line for normal user only

**This requirement:** `build-win.bat` runs as the Windows login that already has Visual Studio 2022. The build does not call `sudo`, does not run `apt`, and does not install Visual Studio, the Windows SDK, or the WebView2 runtime. Git Bash may start `cmd.exe /c build-win.bat`. That shell does not run Termux `pkg`. Admin privilege and a dedicated system user privilege are unused for this build.

## 3. Design principles

- **Caution**: the Windows executable does not link the Linux libraries.
- **Intentional**: one script, one generator, one Debug output directory.
- **Anti-fragile**: the loader and the SQLite amalgamation are in the tree, so configure does not need the network.
- **Over-protect**: CMake on Linux stops instead of emitting a partial GTK binary.

## 4. Design-time verification

| TP-ID | Pass | Status |
|-------|------|--------|
| TP-WIN-ENV | `build-win.bat` exits 0. `build-win\Debug\cloud-player.exe` is present beside `WebView2Loader.dll`. The log names `cl.exe`. A `/W3` overridden by `/w` note on `sqlite3.c` does not fail that exit code | have |

Observed on 2026-10-16. **Map:** `docs/reviews/test-plan.md`.

## 5. Protection rule

Future editors must not compile `src/main/cpp` translation units with this CMake file, link WebKitGTK or libmpv into `cloud-player.exe`, replace `cl.exe` with MSYS2 `gcc` or a Linux compiler on Windows, or make `CMakeLists.txt` emit a Linux executable from a source list that omits the GTK translation units.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-windows-player.md` | What the executable does |
| `docs/requirements/requirement-cpp-dual-build.md` | Which command runs on which host |
| `docs/requirements/requirement-cpp-dependencies.md` | Linux link set |
| `docs/reviews/test-plan.md` | TP-WIN-ENV |
| `build-win.bat` | The Windows build script |
| `CMakeLists.txt` | The Windows executable |
| `third_party/webview2/` | Vendored loader and headers |
| `third_party/sqlite/` | Vendored amalgamation |

**Last updated**: 2026-10-16
**Owner**: Windows Visual C++ environment
