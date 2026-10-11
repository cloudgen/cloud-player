#!/bin/bash
# Runner for the desktop checks. The binary is the simple build.
set -u
root="$(cd "$(dirname "$0")/.." && pwd)"
exec "${root}/tests/test_desktop.sh" "${root}/target/exe/debug/gemini-wilgat"
