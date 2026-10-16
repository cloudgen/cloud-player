// win_host.cpp
// Resolves %USERPROFILE%\.app\cloud-player and settings.sqlite.

#include "player_project.h"
#include "win_host.h"
#include "win_log.h"
#include "config.h"

#include <windows.h>
#include "sqlite3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PlayerPaths g_paths;
static PlayerUi g_ui;
static std::string g_user_agent;
static bool g_help = false;
static bool g_check = false;
static bool g_quiet_locked = false;
static bool g_json_locked = false;
static bool g_debug_from_env = false;
static std::string g_config_override;

static bool truthy(const char *text)
{
    if (text == NULL) {
        return false;
    }
    return _stricmp(text, "true") == 0 || _stricmp(text, "yes") == 0 || atoi(text) == 1;
}

std::wstring player_widen(const std::string &utf8)
{
    if (utf8.empty()) {
        return std::wstring();
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (n <= 0) {
        return std::wstring();
    }
    std::wstring out(static_cast<size_t>(n - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &out[0], n);
    return out;
}

std::string player_narrow(const std::wstring &wide)
{
    if (wide.empty()) {
        return std::string();
    }
    int n = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
    if (n <= 0) {
        return std::string();
    }
    std::string out(static_cast<size_t>(n - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &out[0], n, NULL, NULL);
    return out;
}

static std::wstring env_wide(const wchar_t *name)
{
    DWORD n = GetEnvironmentVariableW(name, NULL, 0);
    if (n == 0) {
        return std::wstring();
    }
    std::wstring out(n, L'\0');
    DWORD got = GetEnvironmentVariableW(name, &out[0], n);
    if (got == 0 || got >= n) {
        return std::wstring();
    }
    out.resize(got);
    return out;
}

static void mkdir_parents(const std::wstring &path)
{
    std::wstring buf = path;
    for (size_t i = 0; i < buf.size(); i++) {
        if (buf[i] != L'\\' && buf[i] != L'/') {
            continue;
        }
        if (i < 3) {
            continue;
        }
        wchar_t saved = buf[i];
        buf[i] = L'\0';
        CreateDirectoryW(buf.c_str(), NULL);
        buf[i] = saved;
    }
    if (!buf.empty()) {
        CreateDirectoryW(buf.c_str(), NULL);
    }
}

static std::string trim_copy(const char *text)
{
    if (text == NULL) {
        return std::string();
    }
    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
        text++;
    }
    std::string s(text);
    while (!s.empty()) {
        char c = s.back();
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }
        s.pop_back();
    }
    if (s.size() >= 2) {
        char a = s.front();
        char b = s.back();
        if ((a == '"' && b == '"') || (a == '\'' && b == '\'')) {
            s = s.substr(1, s.size() - 2);
        }
    }
    return s;
}

static std::wstring expand_path(const std::string &raw)
{
    std::string s = raw;
    if (!s.empty() && s[0] == '~') {
        s = player_narrow(g_paths.user_profile) + s.substr(1);
    }
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '/') {
            s[i] = '\\';
        }
    }
    return player_widen(s);
}

static void apply_defaults(void)
{
    g_ui.window_title = player_widen(PROJECT_NAME);
    g_ui.width = CONFIG_DEFAULT_WINDOW_WIDTH;
    g_ui.height = CONFIG_DEFAULT_WINDOW_HEIGHT;
    g_ui.show_navigation_buttons = CONFIG_DEFAULT_SHOW_NAVIGATION_BUTTONS;
    g_ui.show_user_agent_label = CONFIG_DEFAULT_SHOW_USER_AGENT_LABEL;
    g_ui.navigation_buttons_enabled = CONFIG_DEFAULT_NAVIGATION_BUTTONS_ENABLED;
    g_ui.bookmarks_enabled = CONFIG_DEFAULT_BOOKMARKS_ENABLED;
    g_ui.cookies_enabled = CONFIG_DEFAULT_COOKIES_ENABLED;
    g_ui.scripts_enabled = CONFIG_DEFAULT_SCRIPTS_ENABLED;
}

static void set_value(const std::string &section, const std::string &key, const std::string &value)
{
    if (section == "core") {
        if (key == "window_title") g_ui.window_title = player_widen(value);
        else if (key == "default_width") g_ui.width = atoi(value.c_str());
        else if (key == "default_height") g_ui.height = atoi(value.c_str());
    } else if (section == "ui") {
        if (key == "show_navigation_buttons") g_ui.show_navigation_buttons = truthy(value.c_str());
        else if (key == "show_user_agent_label") g_ui.show_user_agent_label = truthy(value.c_str());
        else if (key == "navigation_buttons_enabled") g_ui.navigation_buttons_enabled = truthy(value.c_str());
    } else if (section == "storage") {
        if (key == "base_path") g_paths.storage_root = expand_path(value);
        else if (key == "cookies_enabled") g_ui.cookies_enabled = truthy(value.c_str());
    } else if (section == "bookmarks") {
        if (key == "enabled") g_ui.bookmarks_enabled = truthy(value.c_str());
    } else if (section == "output") {
        if (key == "quiet") ciao_quiet_mode = truthy(value.c_str());
        else if (key == "json") ciao_json_mode = truthy(value.c_str());
        else if (key == "debug") g_debug_from_env = g_debug_from_env || truthy(value.c_str());
    } else if (section == "scripts") {
        if (key == "enabled") g_ui.scripts_enabled = truthy(value.c_str());
    }
}

static void write_default_config(const std::wstring &path)
{
    FILE *f = NULL;
    if (_wfopen_s(&f, path.c_str(), L"wb") != 0 || f == NULL) {
        ciao_error("Failed to create default config file");
        return;
    }
    fprintf(f, "# config.conf - created on first run\n");
    fprintf(f, "# Project: %s\n\n", PROJECT_ID);
    fprintf(f, "[core]\n");
    fprintf(f, "window_title = %s\n", PROJECT_NAME);
    fprintf(f, "default_width = %d\n", CONFIG_DEFAULT_WINDOW_WIDTH);
    fprintf(f, "default_height = %d\n", CONFIG_DEFAULT_WINDOW_HEIGHT);
    fprintf(f, "project_name = %s\n\n", PROJECT_ID);
    fprintf(f, "[ui]\n");
    fprintf(f, "show_navigation_buttons = %s\n", CONFIG_DEFAULT_SHOW_NAVIGATION_BUTTONS ? "true" : "false");
    fprintf(f, "show_user_agent_label = %s\n", CONFIG_DEFAULT_SHOW_USER_AGENT_LABEL ? "true" : "false");
    fprintf(f, "navigation_buttons_enabled = %s\n\n", CONFIG_DEFAULT_NAVIGATION_BUTTONS_ENABLED ? "true" : "false");
    fprintf(f, "[storage]\n");
    fprintf(f, "base_path = ~/.app/%s\n", PROJECT_ID);
    fprintf(f, "cookies_enabled = %s\n", CONFIG_DEFAULT_COOKIES_ENABLED ? "true" : "false");
    fprintf(f, "cookies_filename = cookies.sqlite\n");
    fprintf(f, "settings_filename = settings.sqlite\n\n");
    fprintf(f, "[output]\n");
    fprintf(f, "quiet = false\njson = false\ndebug = false\n\n");
    fprintf(f, "[bookmarks]\nenabled = %s\n\n", CONFIG_DEFAULT_BOOKMARKS_ENABLED ? "true" : "false");
    fprintf(f, "[scripts]\nenabled = %s\npath = ~/.app/%s/userscripts\n\n",
            CONFIG_DEFAULT_SCRIPTS_ENABLED ? "true" : "false", PROJECT_ID);
    fprintf(f, "[parameters]\nshutdown_seconds = 0\nbookmark_index = 0\n");
    fclose(f);
    ciao_info("Created new default configuration file");
}

static void load_config_file(const std::wstring &path)
{
    FILE *f = NULL;
    if (_wfopen_s(&f, path.c_str(), L"rb") != 0 || f == NULL) {
        ciao_warn("Config file could not be read");
        return;
    }
    std::string section = "core";
    char line[1024];
    while (fgets(line, sizeof line, f) != NULL) {
        std::string trimmed = trim_copy(line);
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
            continue;
        }
        if (trimmed.front() == '[' && trimmed.back() == ']') {
            section = trimmed.substr(1, trimmed.size() - 2);
            continue;
        }
        size_t eq = trimmed.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = trim_copy(trimmed.substr(0, eq).c_str());
        std::string value = trim_copy(trimmed.substr(eq + 1).c_str());
        set_value(section, key, value);
    }
    fclose(f);
}

static void settings_load(void)
{
    mkdir_parents(g_paths.storage_root);
    std::string path = player_narrow(g_paths.settings_path);
    sqlite3 *db = NULL;
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        ciao_error("Failed to open settings DB: %s", db ? sqlite3_errmsg(db) : "open");
        if (db) sqlite3_close(db);
        g_user_agent = PLAYER_DEFAULT_USER_AGENT;
        return;
    }
    char *err = NULL;
    const char *create_sql =
        "CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT NOT NULL);";
    if (sqlite3_exec(db, create_sql, NULL, NULL, &err) != SQLITE_OK) {
        ciao_error("Failed to create settings table: %s", err ? err : "");
        sqlite3_free(err);
        sqlite3_close(db);
        g_user_agent = PLAYER_DEFAULT_USER_AGENT;
        return;
    }
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT value FROM settings WHERE key = ?;";
    std::string loaded;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, SETTINGS_KEY_USER_AGENT, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *text = (const char *)sqlite3_column_text(stmt, 0);
            if (text != NULL) loaded = text;
        }
        sqlite3_finalize(stmt);
    }
    if (loaded.empty()) {
        sqlite3_stmt *ins = NULL;
        const char *insert_sql = "INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?);";
        if (sqlite3_prepare_v2(db, insert_sql, -1, &ins, NULL) == SQLITE_OK) {
            sqlite3_bind_text(ins, 1, SETTINGS_KEY_USER_AGENT, -1, SQLITE_STATIC);
            sqlite3_bind_text(ins, 2, PLAYER_DEFAULT_USER_AGENT, -1, SQLITE_STATIC);
            sqlite3_step(ins);
            sqlite3_finalize(ins);
        }
        g_user_agent = PLAYER_DEFAULT_USER_AGENT;
        ciao_info("First run: saved default User-Agent to settings.sqlite");
    } else {
        g_user_agent = loaded;
    }
    sqlite3_close(db);
}

void player_save_user_agent(const char *ua_utf8)
{
    if (ua_utf8 == NULL || ua_utf8[0] == '\0') {
        ciao_error("player_save_user_agent: empty User-Agent");
        return;
    }
    std::string path = player_narrow(g_paths.settings_path);
    sqlite3 *db = NULL;
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        ciao_error("Failed to open settings DB for save");
        if (db) sqlite3_close(db);
        return;
    }
    sqlite3_stmt *stmt = NULL;
    const char *sql = "INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, SETTINGS_KEY_USER_AGENT, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, ua_utf8, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            ciao_error("Failed to save User-Agent");
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    g_user_agent = ua_utf8;
}

static void parse_args(int argc, char **argv)
{
    const char *debug_env = getenv("DEBUG");
    g_debug_from_env = (debug_env != NULL && debug_env[0] != '\0' && strcmp(debug_env, "0") != 0);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--quiet") == 0) {
            ciao_quiet_mode = true;
            g_quiet_locked = true;
        } else if (strcmp(argv[i], "--json") == 0) {
            ciao_json_mode = true;
            g_json_locked = true;
        } else if (strncmp(argv[i], "--shutdown=", 11) == 0) {
            int val = atoi(argv[i] + 11);
            if (val > 0) ciao_shutdown_seconds = val;
        } else if (strncmp(argv[i], "--bookmark=", 11) == 0) {
            ciao_bookmark_index = atoi(argv[i] + 11);
            if (ciao_bookmark_index < 0) ciao_bookmark_index = 0;
        } else if (strcmp(argv[i], "--check") == 0) {
            g_check = true;
        } else if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            g_config_override = argv[++i];
        } else if (strcmp(argv[i], "help") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            g_help = true;
        }
    }

    if (!g_quiet_locked) {
        const char *q = getenv("QUIET");
        if (q && (atoi(q) == 1 || strcmp(q, "1") == 0)) {
            ciao_quiet_mode = true;
            g_quiet_locked = true;
        }
    }
    if (!g_json_locked) {
        const char *j = getenv("JSON");
        if (j && (atoi(j) == 1 || strcmp(j, "1") == 0)) {
            ciao_json_mode = true;
            g_json_locked = true;
        }
    }
    if (ciao_shutdown_seconds == 0) {
        const char *s = getenv("SHUTDOWN");
        if (s) {
            ciao_shutdown_seconds = atoi(s);
            if (ciao_shutdown_seconds < 0) ciao_shutdown_seconds = 0;
        }
    }
    if (ciao_bookmark_index == 0) {
        const char *b = getenv("BOOKMARK");
        if (b) {
            ciao_bookmark_index = atoi(b);
            if (ciao_bookmark_index < 0) ciao_bookmark_index = 0;
        }
    }
}

void player_prepare(int argc, char **argv)
{
    win_log_init();
    ciao_shutdown_seconds = 0;
    ciao_bookmark_index = 0;
    ciao_quiet_mode = false;
    ciao_json_mode = false;
    apply_defaults();
    parse_args(argc, argv);

    g_paths.user_profile = env_wide(L"USERPROFILE");
    if (g_paths.user_profile.empty()) {
        g_paths.user_profile = env_wide(L"HOME");
    }
    if (g_paths.user_profile.empty()) {
        ciao_error("USERPROFILE is not set");
        g_paths.user_profile = L"C:\\Users\\Public";
    }

    g_paths.app_root = g_paths.user_profile + L"\\.app\\" + player_widen(PROJECT_ID);
    g_paths.storage_root = g_paths.app_root;
    if (!g_config_override.empty()) {
        g_paths.config_path = expand_path(g_config_override);
    } else {
        g_paths.config_path = g_paths.app_root + L"\\config.conf";
    }

    std::wstring config_dir = g_paths.config_path;
    size_t slash = config_dir.find_last_of(L"\\/");
    if (slash != std::wstring::npos) {
        mkdir_parents(config_dir.substr(0, slash));
    }
    win_log_set_directory((g_paths.app_root + L"\\log").c_str());

    DWORD attr = GetFileAttributesW(g_paths.config_path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        write_default_config(g_paths.config_path);
    }

    bool quiet_before = ciao_quiet_mode;
    bool json_before = ciao_json_mode;
    load_config_file(g_paths.config_path);
    if (g_quiet_locked) ciao_quiet_mode = quiet_before;
    if (g_json_locked) ciao_json_mode = json_before;
    if (ciao_json_mode) ciao_quiet_mode = true;

    if (g_paths.storage_root.empty()) {
        g_paths.storage_root = g_paths.app_root;
    }
    g_paths.settings_path = g_paths.storage_root + L"\\settings.sqlite";
    g_paths.webview_profile = g_paths.storage_root + L"\\webview2";
    g_paths.log_dir = g_paths.storage_root + L"\\log";
    g_paths.userscripts_dir = g_paths.storage_root + L"\\userscripts";
    g_paths.cookies_dir = g_paths.storage_root + L"\\cookies";

    mkdir_parents(g_paths.webview_profile);
    mkdir_parents(g_paths.userscripts_dir);
    mkdir_parents(g_paths.cookies_dir);
    win_log_set_directory(g_paths.log_dir.c_str());
    win_log_set_debug(g_debug_from_env);
    settings_load();

    if (g_ui.width < 640) g_ui.width = CONFIG_DEFAULT_WINDOW_WIDTH;
    if (g_ui.height < 480) g_ui.height = CONFIG_DEFAULT_WINDOW_HEIGHT;
}

bool player_help_requested(void) { return g_help; }
bool player_check_requested(void) { return g_check; }
const char *player_config_override(void) { return g_config_override.c_str(); }
const PlayerPaths &player_paths(void) { return g_paths; }
const PlayerUi &player_ui(void) { return g_ui; }
std::wstring player_user_agent_wide(void) { return player_widen(g_user_agent); }
std::string player_user_agent_utf8(void) { return g_user_agent; }

std::wstring player_startup_url(void)
{
    if (ciao_bookmark_index > 0) {
        ciao_warn("Bookmark index %d out of range, falling back to Start Page", ciao_bookmark_index);
    } else {
        ciao_debug("Using default Start Page (no bookmark requested)");
    }
    return player_widen(START_PAGE);
}

bool player_webview_runtime_version(std::wstring *version_out)
{
    const wchar_t *keys[] = {
        L"SOFTWARE\\WOW6432Node\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}",
        L"SOFTWARE\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}",
    };
    HKEY roots[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };
    for (int r = 0; r < 2; r++) {
        for (int k = 0; k < 2; k++) {
            HKEY key = NULL;
            if (RegOpenKeyExW(roots[r], keys[k], 0, KEY_READ, &key) != ERROR_SUCCESS) {
                continue;
            }
            wchar_t value[128];
            DWORD bytes = sizeof value;
            DWORD type = 0;
            LONG rc = RegQueryValueExW(key, L"pv", NULL, &type, (LPBYTE)value, &bytes);
            RegCloseKey(key);
            if (rc == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ) && value[0] != L'\0') {
                if (version_out) *version_out = value;
                return true;
            }
        }
    }
    return false;
}

void player_print_help(void)
{
    printf("cloud-player %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    printf("Usage: cloud-player [help] [--check] [--quiet] [--json] [--shutdown=N] [--bookmark=N] [--config PATH]\n");
    printf("  help            Show this text and exit\n");
    printf("  --check         Open the profile and the WebView2 environment, then exit\n");
    printf("  --shutdown=N    Close the window after N seconds\n");
    printf("  --bookmark=N    0 opens the start page. Any other index warns and still opens it\n");
    printf("  --config PATH   Read this config file instead of the profile config.conf\n");
    printf("  --quiet         Hide info, warning, and debug on the console\n");
    printf("  --json          Quiet console. Stored log lines are still appended\n");
    printf("Environment: DEBUG=1, SHUTDOWN, BOOKMARK, QUIET, JSON\n");
    printf("Start page: %s\n", START_PAGE);
    fflush(stdout);
}
