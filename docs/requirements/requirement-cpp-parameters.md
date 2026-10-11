**file**: docs/requirements/requirement-cpp-parameters.md
**Status**: Active
**Updated**: 2026-10-11
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns every command-line flag, the `log` verb, the `luascript` verb, the `desktop` verb, the `logcheck` verb, the `desktopcheck` verb, and every environment variable the gemini-wilgat binary accepts. `ciao_print_help()` must list each of them. The `log` actions are `requirement-cpp-log-commands.md`. The `luascript` actions are `requirement-cpp-lua-script-commands.md`. `logcheck` is `requirement-cpp-log-format.md`. `desktop` and `desktopcheck` are `requirement-cpp-desktop.md`.

### 1.1 Human-facing

**In one sentence:** You pass flags, `log`, `luascript`, `desktop`, `logcheck`, `desktopcheck`, or environment variables before the window opens, and `help` prints the same set.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person who starts the program | You pass `--shutdown=30` |
| The other role | `ciao_output_init()` | It reads argv and the environment first |
| Not this file | What the bookmark index loads | `requirement-cpp-webui-bookmark.md` |

| Includes | Excludes |
|----------|----------|
| The flag table, the `log` verb, the `luascript` verb, the `desktop` verb, the `logcheck` verb, the `desktopcheck` verb, the environment table, and priority | What `log` prints (`requirement-cpp-log-commands.md`), what `logcheck` prints (`requirement-cpp-log-format.md`), what `luascript` prints (`requirement-cpp-lua-script-commands.md`), and which desktop name is returned (`requirement-cpp-desktop.md`) |

| Surface | What you open | What for |
|---------|---------------|----------|
| The terminal | argv and environment | These switches |
| `help` | Usage text | The same list |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Read the list | Help shows every row below | `gemini-wilgat help` |
| Run `log` | The verb stops before the window | `gemini-wilgat log list` |
| Run `luascript` | The verb stops before the window | `gemini-wilgat luascript folder` |
| Run `logcheck` | The test-purpose verb stops before the window | `gemini-wilgat logcheck --home /tmp --log-datestamp 20261003` |
| Run `desktop` | The verb prints one name and stops before the window | `gemini-wilgat desktop` |
| Run `desktopcheck` | The test-purpose verb names a supplied desktop and stops | `gemini-wilgat desktopcheck --os linux --display :0` |
| Exit after a delay | The timer is armed before the window | `--shutdown=30` |

## 2. Core rules

Priority, highest first: command line, environment, `config.conf`, then built-in defaults.

`ciao_print_help()` lists every row in both tables. `help` as the first argument prints that text and does not open the window. `log`, `luascript`, `desktop`, `logcheck`, and `desktopcheck` do not open the window either. Help lists `logcheck` and `desktopcheck` under a Tests heading, apart from `log`, `luascript`, and `desktop`.

| Parameter | Meaning |
|-----------|---------|
| `--config <path>` | Load this config file instead of the default |
| `--quiet` | Suppress non-error messages |
| `--json` | One JSON object per line; forces quiet |
| `--shutdown=N` | Exit after N seconds; 0 disables |
| `--bookmark=N` | Open bookmark index N at startup |
| `help` | Print help and stop |
| `log` | `list`, `show N`, `folder`, or `clear`. Text or `--json`. `requirement-cpp-log-commands.md` |
| `luascript` | `folder`, `list`, `view`, `edit`, or `new`. Text or `--json`. `requirement-cpp-lua-script-commands.md` |
| `logcheck` | Test-purpose checker. `--file <path>`, or `--home` / `--prefix` with `--log-datestamp YYYYMMDD`. Text or `--json`. `requirement-cpp-log-format.md` |
| `desktop` | Print `windows`, `macos`, or `linux-desktop` for this process. Text or `--json`. `requirement-cpp-desktop.md` |
| `desktopcheck` | Test-purpose. `--os windows`, `--os macos`, or `--os linux`, with optional `--display` and `--gdk-backend`. Text or `--json`. `requirement-cpp-desktop.md` |

| Variable | Meaning |
|----------|---------|
| `DEBUG=1` | Enable `ciao_debug`. The C++ values that must appear are listed in `requirement-cpp-debug-mode.md` |
| `QUIET=1` | Same as `--quiet` |
| `JSON=1` | Same as `--json` |
| `SHUTDOWN=N` | Same as `--shutdown=N` |
| `BOOKMARK=N` | Same as `--bookmark=N` |

Parsing happens in `ciao_output_init()` before the window is created. `--config` is read in `main()` and passed to `config_init()`.

## 3. Protection rule

Future editors must not add a flag, the `log` verb, the `luascript` verb, the `desktop` verb, the `logcheck` verb, or the `desktopcheck` verb in `output.cpp` without adding the same row to this table and to `ciao_print_help()` in the same change. `logcheck` and `desktopcheck` stay under the Tests heading.

## 4. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-output.md` | What quiet and JSON do to messages |
| `docs/requirements/requirement-cpp-debug-mode.md` | Values printed when `DEBUG=1` |
| `docs/requirements/requirement-cpp-webui-bookmark.md` | What `--bookmark` opens |
| `src/main/cpp/output.cpp` | Parser and help text |
| `docs/requirements/requirement-cpp-log-commands.md` | The `log` verb |
| `docs/requirements/requirement-cpp-log-format.md` | The `logcheck` verb |
| `docs/requirements/requirement-cpp-lua-script-commands.md` | The `luascript` verb |
| `docs/requirements/requirement-cpp-desktop.md` | The `desktop` and `desktopcheck` verbs |
| `src/main/cpp/main.cpp` | `--config` and the shutdown timer |

**Last updated**: 2026-10-06
**Owner**: Parameters
