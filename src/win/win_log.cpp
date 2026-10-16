// win_log.cpp
// Stored line: [YYYY-MM-DD HH:MM:SS] pid:<pid> [LEVEL] @<file> :] message
// Console warning label is WARN. The stored label is WARNING.

#include "player_project.h"
#include "win_log.h"

#include <windows.h>

#include <stdio.h>
#include <string.h>

int ciao_shutdown_seconds = 0;
int ciao_bookmark_index = 0;
bool ciao_quiet_mode = false;
bool ciao_json_mode = false;

static bool g_debug = false;
static bool g_in_append = false;
static bool g_dir_error_reported = false;
static wchar_t g_log_dir[1024];

void win_log_init(void)
{
    g_log_dir[0] = L'\0';
    g_debug = false;
    g_dir_error_reported = false;
}

void win_log_set_directory(const wchar_t *directory)
{
    if (directory == NULL) {
        g_log_dir[0] = L'\0';
        return;
    }
    wcsncpy_s(g_log_dir, directory, _TRUNCATE);
}

void win_log_set_debug(bool enabled)
{
    g_debug = enabled;
}

static void mkdir_parents(const wchar_t *path)
{
    wchar_t buf[1024];
    wcsncpy_s(buf, path, _TRUNCATE);
    size_t n = wcslen(buf);
    for (size_t i = 0; i < n; i++) {
        if (buf[i] != L'\\' && buf[i] != L'/') {
            continue;
        }
        if (i == 0) {
            continue;
        }
        wchar_t saved = buf[i];
        buf[i] = L'\0';
        if (!(i == 2 && buf[1] == L':')) {
            CreateDirectoryW(buf, NULL);
        }
        buf[i] = saved;
    }
    CreateDirectoryW(buf, NULL);
}

static const char *file_base(const char *file)
{
    const char *base = (file != NULL) ? file : "";
    for (const char *p = base; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }
    return base;
}

static void console_line(CiaoOutputLevel level, const char *message)
{
    if (ciao_quiet_mode && level != CIAO_LEVEL_ERROR) {
        return;
    }
    if (level == CIAO_LEVEL_DEBUG && !g_debug) {
        return;
    }
    const char *label =
        (level == CIAO_LEVEL_WARNING) ? "WARN" :
        (level == CIAO_LEVEL_ERROR) ? "ERROR" :
        (level == CIAO_LEVEL_DEBUG) ? "DEBUG" : "INFO";
    FILE *out = (level == CIAO_LEVEL_ERROR) ? stderr : stdout;
    fprintf(out, "[%s] %s\n", label, (message != NULL) ? message : "(null)");
    fflush(out);
}

static void append_stored(CiaoOutputLevel level, const char *file, const char *message)
{
    if (g_in_append || g_log_dir[0] == L'\0') {
        return;
    }
    g_in_append = true;

    SYSTEMTIME st;
    GetLocalTime(&st);
    char civil[32];
    char stamp[16];
    snprintf(civil, sizeof civil, "%04u-%02u-%02u %02u:%02u:%02u",
             (unsigned)st.wYear, (unsigned)st.wMonth, (unsigned)st.wDay,
             (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond);
    snprintf(stamp, sizeof stamp, "%04u%02u%02u",
             (unsigned)st.wYear, (unsigned)st.wMonth, (unsigned)st.wDay);

    if (!CreateDirectoryW(g_log_dir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        mkdir_parents(g_log_dir);
    }

    wchar_t path[1200];
    swprintf_s(path, L"%s\\%hs-%hs%hs", g_log_dir, PROJECT_ID, stamp, LOGGER_FILE_EXT);

    const char *level_name =
        (level == CIAO_LEVEL_WARNING) ? "WARNING" :
        (level == CIAO_LEVEL_ERROR) ? "ERROR" :
        (level == CIAO_LEVEL_DEBUG) ? "DEBUG" : "INFO";

    FILE *f = NULL;
    if (_wfopen_s(&f, path, L"ab") != 0 || f == NULL) {
        if (!g_dir_error_reported) {
            fprintf(stderr, "[ERROR] cannot open log file\n");
            g_dir_error_reported = true;
        }
        g_in_append = false;
        return;
    }
    fprintf(f, "[%s] pid:%lu [%s] @%s :] %s\n",
            civil,
            (unsigned long)GetCurrentProcessId(),
            level_name,
            file_base(file),
            (message != NULL) ? message : "(null)");
    fclose(f);
    g_in_append = false;
}

void ciao_log(CiaoOutputLevel level, const char *file, const char *format, va_list ap)
{
    if (level == CIAO_LEVEL_DEBUG && !g_debug) {
        return;
    }
    char message[2048];
    va_list copy;
    va_copy(copy, ap);
    vsnprintf(message, sizeof message, (format != NULL) ? format : "", copy);
    va_end(copy);
    message[sizeof message - 1] = '\0';
    console_line(level, message);
    append_stored(level, file, message);
}

void ciao_info_at(const char *file, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    ciao_log(CIAO_LEVEL_INFO, file, format, ap);
    va_end(ap);
}

void ciao_warn_at(const char *file, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    ciao_log(CIAO_LEVEL_WARNING, file, format, ap);
    va_end(ap);
}

void ciao_error_at(const char *file, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    ciao_log(CIAO_LEVEL_ERROR, file, format, ap);
    va_end(ap);
}

void ciao_debug_at(const char *file, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    ciao_log(CIAO_LEVEL_DEBUG, file, format, ap);
    va_end(ap);
}
