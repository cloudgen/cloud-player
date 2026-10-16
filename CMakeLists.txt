# Windows: cloud-player.exe (Win32 + WebView2), Visual Studio 2022.
# The Linux program stays ./build.sh (GTK 3 + WebKitGTK). This file does not build that program.
# See docs/requirements/requirement-cpp-windows-environment.md
# and docs/requirements/requirement-cpp-dual-build.md.

cmake_minimum_required(VERSION 3.20)
project(cloud-player VERSION 1.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "This CMake file builds cloud-player.exe with Visual Studio. On Linux run ./build.sh")
endif()

add_executable(cloud-player
    src/win/win_log.cpp
    src/win/win_host.cpp
    src/win/win_app.cpp
    third_party/sqlite/sqlite3.c
)

target_include_directories(cloud-player PRIVATE
    src/win
    src/main/cpp
    third_party/sqlite
    third_party/webview2/include
)

target_compile_definitions(cloud-player PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    SQLITE_THREADSAFE=1
    SQLITE_OMIT_LOAD_EXTENSION
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_options(cloud-player PRIVATE /W3 /EHsc /utf-8)
    set_source_files_properties(third_party/sqlite/sqlite3.c PROPERTIES COMPILE_OPTIONS "/w")
endif()

target_link_libraries(cloud-player PRIVATE
    "${CMAKE_SOURCE_DIR}/third_party/webview2/x64/WebView2Loader.dll.lib"
    user32
    gdi32
    ole32
    oleaut32
    shell32
    shlwapi
    advapi32
    version
)

add_custom_command(TARGET cloud-player POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/third_party/webview2/x64/WebView2Loader.dll"
        "$<TARGET_FILE_DIR:cloud-player>/WebView2Loader.dll"
    VERBATIM
)
