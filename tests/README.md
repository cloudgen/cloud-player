# Tests

Full-program checks for gemini-wilgat. Each assert names a TP id from `docs/reviews/test-plan.md`.

| File | TP ids |
|------|--------|
| `test_desktop.sh` | TP-DESKTOP-WINDOWS, TP-DESKTOP-MACOS, TP-DESKTOP-LINUX-X11, TP-DESKTOP-SEPARATE, TP-DESKTOP-LIVE |

Run `./tests/run.sh` after `./build.sh build`. The checks do not open a window and do not change `HOME`.
