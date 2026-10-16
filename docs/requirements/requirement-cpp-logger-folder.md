**file**: docs/requirements/requirement-cpp-logger-folder.md
**Status**: Deferred
**Updated**: 2026-10-03
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the log directory for a normal run and the rehearsal log directory for `test`, `list`, and `show`. The dated file is `requirement-cpp-logger.md`. Config, settings, cookies, and userscripts stay in `requirement-cpp-project-structure.md`.

### 1.1 Human-facing

**In one sentence:** Root or sudo logs under `/var/cloud-player/log`, and every other user logs under `$HOME/.app/cloud-player/log`.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person whose login selects the directory | A normal user, or root or sudo |
| The other role | `output.cpp` | It resolves one log directory |
| Not this file | The dated file name, and config, settings, cookies, and userscripts | `requirement-cpp-logger.md` and `requirement-cpp-project-structure.md` |

| Includes | Excludes |
|----------|----------|
| The two log directories | `$PREFIX/bin`, `$HOME/.local/bin`, and the runtime files that are not logs |

| Surface | What you open | What for |
|---------|---------------|----------|
| The log directory | The folder that holds the dated file | Find `cloud-player-<YYYYMMDD>.log` |

For a normal run:

| Environment | Base directory | Log directory |
|-------------|---------------|---------------|
| Root or sudo | `/var/cloud-player` | `/var/cloud-player/log` |
| Other | `$HOME/.app/cloud-player` | `$HOME/.app/cloud-player/log` |

For the test command, the literal directory `/tmp` stands in:

| Switch | Rehearsal base | Rehearsal log directory |
|--------|----------------|-------------------------|
| `--prefix /tmp` | `/tmp/var/cloud-player` | `/tmp/var/cloud-player/log` |
| `--home /tmp` | `/tmp/.app/cloud-player` | `/tmp/.app/cloud-player/log` |

## 2. Core rules

1. There are two environments. Root or sudo is a process whose effective user id is 0, including a process started with sudo. Every other user is the other environment.
2. The base directory for root or sudo is `/var/cloud-player`. The log directory is `/var/cloud-player/log`. The base directory for every other user is `$HOME/.app/cloud-player`. The log directory is `$HOME/.app/cloud-player/log`.
3. A normal run uses those two paths. It does not read `--prefix`, `--home`, or `PREFIX` to choose a directory. When the other user's `HOME` is empty, the directory stays unresolved. The program does not call `getpwuid` and does not invent a path.
4. The test command uses the rehearsal paths. `--prefix <dir>` names `<dir>/var/cloud-player` and `<dir>/var/cloud-player/log`. `--home <dir>` names `<dir>/.app/cloud-player` and `<dir>/.app/cloud-player/log`. `--prefix /tmp` therefore names `/tmp/var/cloud-player/log`. `--home /tmp` names `/tmp/.app/cloud-player/log`. Those two directories are different. `--prefix` wins when both switches are present. The test command does not read `PREFIX`, `HOME`, or the effective user id.
5. A non-empty `DEBUG` prints `logger folder <path>` for the log directory and `log file <path>` for the dated file, through `ciao_debug`. When the directory is unresolved the line is `logger folder unresolved`. Quiet still hides a human debug line on the terminal. That gate is `requirement-cpp-output.md`.
6. Printing the folder does not create the directory. The directory is created only when a log line is appended, which is `requirement-cpp-logger.md`. The program does not use `mkstemp`.

The other user's base directory is the same folder `requirement-cpp-project-structure.md` uses for config, settings, cookies, and userscripts. This file owns only the `log` child and the root-or-sudo pair. It does not own those other files.

## 3. Protection rule

Future editors must not invent a third environment, point this directory at an install bin, fall back to another home when `HOME` is empty, or treat `src/main/cpp` as the log directory.

## 4. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-logger.md` | The file inside the log directory |
| `docs/requirements/requirement-cpp-project-structure.md` | Config, settings, cookies, and userscripts |
| `docs/requirements/requirement-cpp-output.md` | Terminal quiet for the debug folder line |
| `docs/requirements/requirement-cpp-tests.md` | `TP-CPP-LOGGER-FOLDER` |
| `docs/reviews/test-plan.md` | Status of `TP-CPP-LOGGER-FOLDER` |
| `src/main/cpp/output.cpp` | Resolver, when this row is implemented |
| `docs/requirements/requirement-cpp-log-commands.md` | `log folder` prints this directory and does not create it |

**Last updated**: 2026-10-03
**Owner**: Logger folder
