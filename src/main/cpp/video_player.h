// video_player.h
// =========================================================================
// CIAO DEFENSIVE CODING STYLE - MPV VIDEO VIEW
// =========================================================================
//
// Plays an address whose path ends in .mp4 inside this window.
// The player is libmpv, the C client library of the sibling mpv project.
// Web pages keep using the WebKit view.
//
// =========================================================================

#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

typedef struct VideoPlayer VideoPlayer;

// TRUE when the path (query and fragment ignored) ends with ".mp4".
gboolean video_player_url_is_mp4(const char *url);

// One player for the process. web_view is the page shown when playback stops.
// Returns NULL only when the GL widget cannot be created.
VideoPlayer *video_player_new(WebKitWebView *web_view);

GtkWidget *video_player_widget(VideoPlayer *player);

// stack holds the web view ("web") and this player's widget ("video").
void video_player_set_stack(VideoPlayer *player, GtkWidget *stack);

// TRUE while the mpv widget is the visible stack child.
gboolean video_player_is_showing(VideoPlayer *player);

// Show the mpv widget and load url. On GL failure, open the mpv program.
// Returns FALSE when nothing could start.
gboolean video_player_play(VideoPlayer *player, const char *url);

// Stop playback and show the web view again.
void video_player_stop(VideoPlayer *player);

// Playback position in seconds, or a negative value when unknown.
double video_player_time_pos(VideoPlayer *player);

#endif // VIDEO_PLAYER_H
