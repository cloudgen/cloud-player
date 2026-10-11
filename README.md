# cloud-player - A native window for Gemini and local mp4 files

![Version](https://img.shields.io/badge/Version-1.0.7-blue?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)
[![CIAO](https://img.shields.io/badge/Philosophy-CIAO%20(Caution%20%E2%80%A2%20Intentional%20%E2%80%A2%20Anti--fragile%20%E2%80%A2%20Over--engineered)-purple.svg)](https://github.com/cloudgen/ciao)
[![Stars](https://img.shields.io/github/stars/cloudgen/cloud-player?style=flat-square)](https://github.com/cloudgen/cloud-player)
[![GTK](https://img.shields.io/badge/UI-GTK%203-brightgreen?style=flat-square)](https://www.gtk.org/)
[![WebKit](https://img.shields.io/badge/Engine-WebKitGTK%204.1-orange?style=flat-square)](https://webkitgtk.org/)

You run one program, `gemini-wilgat`, and get one window titled **Gemini (wilgat)**. It opens [Gemini](https://gemini.google.com) and, when you submit an address that ends in `.mp4`, it plays that file in the same window.

| | |
|---|---|
| **You** | You start the program on your own Linux login. The window, its cookies, and its logs stay in that login's app folder. |
| **The window** | It shows the Gemini page, keeps an address bar, and switches to a video view for a local `.mp4` you submit from that bar. |
| **Not this** | It is not a general browser, and a click inside the page does not start the video view. |

**Includes:** the Gemini start page, Back / Forward / Reload, an address bar, a user-agent menu, cookies for this window, and local `.mp4` playback when the address ends in `.mp4`.

**Excludes:** playback from a click on a file inside the page, video types other than `.mp4`, and a numbered menu in the terminal. `gemini-wilgat help` prints commands and then exits.

| Step | What it means | What you type |
|------|----------------|---------------|
| Build | Install the development packages, then compile. The install step uses `sudo` for packages and, if it is missing, the sol2 header. The build downloads the mpv client library into `third_party/`, which is not part of the git tree. | `./build.sh install` then `./build.sh build` |
| Open | The program file is `gemini-wilgat`. A window opens on the Gemini start page. Nothing is installed onto the system path. | `./target/exe/debug/gemini-wilgat` |
| Play | Type the address in the bar and press Enter or Go. An address that ends in `.mp4` plays in the video view. Any other address opens in the page. | `file:///path/to/clip.mp4` in the address bar |

## Features

- One GTK 3 window for [Gemini](https://gemini.google.com), using WebKitGTK 4.1.
- Address bar with Back, Forward, Reload, and Go. Enter does the same thing as Go.
- A submitted address that ends in `.mp4` plays through libmpv in the window. Query strings are ignored when deciding that suffix (`clip.mp4?unused=1` still counts).
- A click on a link or a file inside the page stays in the page. It does not start the video view.
- Cookies and the last user-agent choice are stored under `~/.app/gemini-wilgat/`.
- User-agent menu: Webview (default), Chrome, Firefox, Safari, and Edge.
- Log and Lua-script commands from `gemini-wilgat help`. With no command, the window opens.
- Version **1.0.7**, from `src/main/cpp/project.h`.

## Quick Installation

Primary build, from a clone of this repository:

```bash
./build.sh install
./build.sh build
./target/exe/debug/gemini-wilgat
```

`./build.sh install` runs `sudo apt` for `build-essential`, `pkg-config`, `libgtk-3-dev`, `libwebkit2gtk-4.1-dev`, `libsqlite3-dev`, `libglib2.0-dev`, `liblua5.3-dev`, `lua5.3`, and `git`. If `/usr/local/include/sol/sol.hpp` is missing, it clones [sol2](https://github.com/ThePhD/sol2) and copies that header into `/usr/local/include/sol/`.

`./build.sh build` downloads `libmpv2` and `libmpv-dev` into `third_party/libmpv` when they are not already there, then compiles every C and C++ file under `src/` into `target/exe/debug/gemini-wilgat`. The link step also asks `pkg-config` for `epoxy` and `egl` (`libepoxy-dev` and `libegl-dev` on Debian and Ubuntu). There is no online install script and no checksum sidecar.

Meson compiles the same sources:

```bash
meson setup builddir --buildtype=debug
meson compile -C builddir
./builddir/gemini-wilgat
```

## Usage

Run `./target/exe/debug/gemini-wilgat` with no arguments to open the window. The start page is `https://gemini.google.com`. The binary name is `gemini-wilgat`. The title on the window is **Gemini (wilgat)**.

```text
./target/exe/debug/gemini-wilgat help
./target/exe/debug/gemini-wilgat --config config-sample.conf
./target/exe/debug/gemini-wilgat --shutdown=3600
./target/exe/debug/gemini-wilgat log list
```

| Command or option | What it does |
|-------------------|--------------|
| `help` | Print commands and options, then exit. |
| `--config <path>` | Load that config file instead of `~/.app/gemini-wilgat/config.conf`. |
| `--quiet` / `QUIET=1` | Hide non-error messages. |
| `--json` / `JSON=1` | Structured JSON on the terminal. This also quiets ordinary messages. |
| `--shutdown=N` / `SHUTDOWN=N` | Close the window after N seconds. `0` means do not close on a timer. |
| `--bookmark=N` / `BOOKMARK=N` | Startup bookmark index. `0` is the Gemini start page. |
| `DEBUG=1` | Also print debug lines. |
| `log list` / `log show N` / `log folder` / `log clear` | List, print, locate, or delete log files, then exit. |
| `luascript list` / `luascript folder` / `luascript view N` / `luascript new NAME` | Inspect or create Lua userscripts, then exit. |

Logs for a normal login are `~/.app/gemini-wilgat/log/gemini-wilgat-YYYYMMDD.log`. Cookies are `~/.app/gemini-wilgat/cookies/cookies.sqlite`.

To play a file, put an address that ends in `.mp4` in the bar and press Enter or Go. The page hides and the video view shows the picture. Submit any other address to go back to the page.

## Examples

Gemini in the window, before you sign in. This capture used a fresh app folder, so it is not a signed-in session.

![Gemini start page in the cloud-player window](images/browser.png)

The same window after the address bar was given `file:///tmp/cp-sample.mp4` and Enter. The picture is a generated color-bar clip, not a personal video.

![Local mp4 playing in the cloud-player window](images/video.png)

```bash
./target/exe/debug/gemini-wilgat
```

Then, in the address bar:

```text
file:///path/to/clip.mp4
```

Press Enter. The video view opens. A click on `clip.mp4` inside a directory listing does not do this. Paste or type the address and press Enter or Go.

## Platform Compatibility

| Environment | Status |
|-------------|--------|
| Linux x86_64, GTK 3, WebKitGTK 4.1 | This is the build that produced the binary and the pictures above (GTK 3.24, WebKitGTK 2.52). |
| Debian or Ubuntu package names | The install commands above use `apt`. |
| Other architectures or operating systems | Not claimed. The mpv download path is the Ubuntu `x86_64` library layout. |
| Terminal-only use | `help`, `log`, and `luascript` run without keeping the window open. Watching a video needs the window. |

## Related Projects

- [CIAO](https://github.com/cloudgen/ciao) — the defensive style this tree is written in.
- [Gemini](https://gemini.google.com) — the page the window opens.
- [mpv](https://mpv.io/) — the player library behind the `.mp4` view. This repo downloads the client library at build time and does not vendor the mpv sources.

## Contributing

Open an issue or a pull request on [cloudgen/cloud-player](https://github.com/cloudgen/cloud-player). Please keep behavior changes aligned with `src/main/cpp/project.h` and say what a person actually sees in the window.

Report security-sensitive bugs in private. The address and the preferred channel are in [SECURITY.md](SECURITY.md).

## License

[MIT](LICENSE.md). Copyright (c) 2026 Wong Chun Fai, aka: Cloudgen (cloudgen.wong@gmail.com).

## Last Update

2026-10-11 — product docs for 1.0.7, including the Gemini window and the local `.mp4` view. The version badge matches `src/main/cpp/project.h`.
