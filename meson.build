# meson.build — second build for cloud-player.
# Compiles the same C++ sources as ./build.sh.
# project() version matches the runtime macros in src/main/cpp/project.h (1.2.0).

project('cloud-player', 'cpp',
    version: '1.2.0',
    license: 'MIT',
    default_options: [
        'warning_level=3',
        'buildtype=debugoptimized',
        'cpp_std=c++17',
    ]
)

gtk_dep = dependency('gtk+-3.0', required: true)
webkit_dep = dependency('webkit2gtk-4.1', required: true)
glib_dep = dependency('glib-2.0', required: true)
gobject_dep = dependency('gobject-2.0', required: true)
gio_dep = dependency('gio-2.0', required: true)
sqlite_dep = dependency('sqlite3', required: true)
lua_dep = dependency('lua5.3', required: true)
thread_dep = dependency('threads', required: true)

sol_inc = include_directories('/usr/local/include')

src_files = files(
    'src/main/cpp/auto_follow.cpp',
    'src/main/cpp/config.cpp',
    'src/main/cpp/cookies.cpp',
    'src/main/cpp/desktop_env.cpp',
    'src/main/cpp/dialog_about.cpp',
    'src/main/cpp/dialog_version.cpp',
    'src/main/cpp/load_page_ogin_check.cpp',
    'src/main/cpp/login_detector_app.cpp',
    'src/main/cpp/log_commands.cpp',
    'src/main/cpp/log_format.cpp',
    'src/main/cpp/log_view_html.cpp',
    'src/main/cpp/lua_script_commands.cpp',
    'src/main/cpp/main.cpp',
    'src/main/cpp/menu_bookmark.cpp',
    'src/main/cpp/menu_bookmark_app.cpp',
    'src/main/cpp/menu_exit.cpp',
    'src/main/cpp/menu_log.cpp',
    'src/main/cpp/menu_lua_script.cpp',
    'src/main/cpp/menu_user_agent.cpp',
    'src/main/cpp/output.cpp',
    'src/main/cpp/path_utils.cpp',
    'src/main/cpp/settings.cpp',
    'src/main/cpp/ui_menu_app.cpp',
    'src/main/cpp/ui_window_app.cpp',
    'src/main/cpp/url_bar_go_button.cpp',
    'src/main/cpp/url_bar_page.cpp',
    'src/main/cpp/url_bar_user_agent.cpp',
    'src/main/cpp/webkit_setup.cpp',
    'src/main/cpp/userscript_manager.cpp',
    'src/main/cpp/video_player.cpp',
)

# libmpv is fetched by ./build.sh into third_party/libmpv (sibling mpv client library).
mpv_prefix = meson.current_source_dir() / 'third_party/libmpv/usr'
multiarch = run_command('gcc', '-print-multiarch', check: false).stdout().strip()
if multiarch == ''
    multiarch = 'x86_64-linux-gnu'
endif
mpv_libdir = mpv_prefix / 'lib' / multiarch
mpv_inc = include_directories('third_party/libmpv/usr/include')
cpp = meson.get_compiler('cpp')
mpv_dep = cpp.find_library('mpv', dirs: [mpv_libdir], required: true)
epoxy_dep = dependency('epoxy', required: true)
egl_dep = dependency('egl', required: true)
dl_dep = cpp.find_library('dl', required: true)

executable('cloud-player',
    sources: src_files,
    include_directories: [sol_inc, mpv_inc],
    dependencies: [
        gtk_dep,
        webkit_dep,
        glib_dep,
        gobject_dep,
        gio_dep,
        sqlite_dep,
        lua_dep,
        thread_dep,
        mpv_dep,
        epoxy_dep,
        egl_dep,
        dl_dep,
    ],
    build_rpath: mpv_libdir,
    install_rpath: mpv_libdir,
    install: true,
    install_dir: get_option('bindir')
)
