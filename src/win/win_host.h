// win_host.h
// Profile paths, config.conf, and settings.sqlite for the Windows player.

#ifndef WIN_HOST_H
#define WIN_HOST_H

#include <stdbool.h>
#include <string>

struct PlayerPaths {
    std::wstring user_profile;
    std::wstring app_root;
    std::wstring config_path;
    std::wstring storage_root;
    std::wstring settings_path;
    std::wstring webview_profile;
    std::wstring log_dir;
    std::wstring userscripts_dir;
    std::wstring cookies_dir;
};

struct PlayerUi {
    std::wstring window_title;
    int width;
    int height;
    bool show_navigation_buttons;
    bool show_user_agent_label;
    bool navigation_buttons_enabled;
    bool bookmarks_enabled;
    bool cookies_enabled;
    bool scripts_enabled;
};

void player_prepare(int argc, char **argv);

bool player_help_requested(void);
bool player_check_requested(void);
const char *player_config_override(void);

const PlayerPaths &player_paths(void);
const PlayerUi &player_ui(void);

std::wstring player_user_agent_wide(void);
std::string player_user_agent_utf8(void);
void player_save_user_agent(const char *ua_utf8);

std::wstring player_widen(const std::string &utf8);
std::string player_narrow(const std::wstring &wide);
std::wstring player_startup_url(void);

bool player_webview_runtime_version(std::wstring *version_out);

void player_print_help(void);

#endif
