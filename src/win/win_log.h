// win_log.h
// Console and dated-file log for the Windows player. Does not use GLib.

#ifndef WIN_LOG_H
#define WIN_LOG_H

#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CIAO_LEVEL_INFO,
    CIAO_LEVEL_WARNING,
    CIAO_LEVEL_ERROR,
    CIAO_LEVEL_DEBUG
} CiaoOutputLevel;

extern int ciao_shutdown_seconds;
extern int ciao_bookmark_index;
extern bool ciao_quiet_mode;
extern bool ciao_json_mode;

void win_log_init(void);
void win_log_set_directory(const wchar_t *directory);
void win_log_set_debug(bool enabled);

void ciao_log(CiaoOutputLevel level, const char *file, const char *format, va_list ap);
void ciao_info_at(const char *file, const char *format, ...);
void ciao_warn_at(const char *file, const char *format, ...);
void ciao_error_at(const char *file, const char *format, ...);
void ciao_debug_at(const char *file, const char *format, ...);

#define ciao_info(...)  ciao_info_at(__FILE__, __VA_ARGS__)
#define ciao_warn(...)  ciao_warn_at(__FILE__, __VA_ARGS__)
#define ciao_error(...) ciao_error_at(__FILE__, __VA_ARGS__)
#define ciao_debug(...) ciao_debug_at(__FILE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
