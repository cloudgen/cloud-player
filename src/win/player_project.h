// player_project.h
// Windows product identity. User-agent strings stay in project.h.
// Included only by src/win. The Linux build does not compile these files.
// See docs/requirements/requirement-cpp-windows-player.md.

#ifndef PLAYER_PROJECT_H
#define PLAYER_PROJECT_H

#include "project.h"

#undef PROJECT_DESCRIPTION
#undef PERSISTENCE_STORAGE_PATH

#define PROJECT_DESCRIPTION "Gemini and local pages in a WebView2 window on Windows."
#define PERSISTENCE_STORAGE_PATH ".app/" PROJECT_ID "/cookies"

/* UA_WEBVIEW names Ubuntu WebKit. This process is the Edge WebView2 runtime. */
#define PLAYER_DEFAULT_USER_AGENT UA_EDGE

#endif
