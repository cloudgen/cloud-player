// video_player.cpp
// =========================================================================
// CIAO DEFENSIVE CODING STYLE - MPV VIDEO VIEW
// =========================================================================
//
// The sibling mpv project is the player. This file embeds its libmpv client
// library in a GtkGLArea. build.sh fetches libmpv into third_party/libmpv
// because that sibling tree is not built in this workspace.
//
// An address is an mp4 when its path ends with ".mp4". The query string and
// the fragment are not part of the path. Go on that address shows this view
// and calls mpv loadfile. Any other address stops playback and returns to
// the WebKit view.
//
// libmpv requires LC_NUMERIC "C" for the whole process.
// =========================================================================

#include "video_player.h"
#include "output.h"

#include <clocale>
#include <cstring>

#include <dlfcn.h>
#include <epoxy/gl.h>

// Declared here so this file does not include EGL/egl.h. That header
// clashes with epoxy's Khronos enum when both are included.
extern "C" void *eglGetProcAddress(const char *procname);

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#include <mpv/client.h>
#include <mpv/render_gl.h>

struct VideoPlayer {
    gint alive;
    gint render_scheduled;
    gint events_scheduled;
    gboolean embed_failed;
    GtkWidget *gl_area;
    GtkWidget *stack;
    WebKitWebView *web_view;
    mpv_handle *mpv;
    mpv_render_context *render;
    gchar *pending_url;
    gchar *current_url;
};

static VideoPlayer g_player;

extern "C" void *mpv_get_proc_address(void *ctx, const char *name)
{
    (void)ctx;
    if (name == NULL) {
        return NULL;
    }
    void *addr = reinterpret_cast<void *>(eglGetProcAddress(name));
    if (addr != NULL) {
        return addr;
    }
    return dlsym(RTLD_DEFAULT, name);
}

static void submit_pending(VideoPlayer *vp);

static gboolean render_update_idle(gpointer data)
{
    VideoPlayer *vp = static_cast<VideoPlayer *>(data);
    g_atomic_int_set(&vp->render_scheduled, 0);
    if (!g_atomic_int_get(&vp->alive) || vp->render == NULL || vp->gl_area == NULL) {
        return G_SOURCE_REMOVE;
    }
    uint64_t flags = mpv_render_context_update(vp->render);
    if ((flags & MPV_RENDER_UPDATE_FRAME) != 0) {
        gtk_gl_area_queue_render(GTK_GL_AREA(vp->gl_area));
    }
    return G_SOURCE_REMOVE;
}

extern "C" void mpv_render_update_cb(void *ctx)
{
    VideoPlayer *vp = static_cast<VideoPlayer *>(ctx);
    if (vp == NULL || !g_atomic_int_get(&vp->alive)) {
        return;
    }
    if (g_atomic_int_compare_and_exchange(&vp->render_scheduled, 0, 1)) {
        g_idle_add(render_update_idle, vp);
    }
}

static void show_video_error(VideoPlayer *vp, const char *body)
{
    GtkWindow *parent = NULL;
    if (vp != NULL && vp->gl_area != NULL) {
        GtkWidget *toplevel = gtk_widget_get_toplevel(vp->gl_area);
        if (GTK_IS_WINDOW(toplevel)) {
            parent = GTK_WINDOW(toplevel);
        }
    }
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_CLOSE,
        "%s",
        body != NULL ? body : "Playback failed");
    gtk_window_set_title(GTK_WINDOW(dialog), "Video");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static gboolean log_time_pos_once(gpointer data)
{
    VideoPlayer *vp = static_cast<VideoPlayer *>(data);
    if (!g_atomic_int_get(&vp->alive) || vp->mpv == NULL) {
        return G_SOURCE_REMOVE;
    }
    double pos = -1.0;
    int rc = mpv_get_property(vp->mpv, "time-pos", MPV_FORMAT_DOUBLE, &pos);
    if (rc >= 0) {
        ciao_info("mpv time-pos %.3f", pos);
    } else {
        ciao_warn("mpv time-pos unavailable: %s", mpv_error_string(rc));
    }
    return G_SOURCE_REMOVE;
}

static gboolean events_idle(gpointer data)
{
    VideoPlayer *vp = static_cast<VideoPlayer *>(data);
    g_atomic_int_set(&vp->events_scheduled, 0);
    if (!g_atomic_int_get(&vp->alive) || vp->mpv == NULL) {
        return G_SOURCE_REMOVE;
    }

    while (true) {
        mpv_event *event = mpv_wait_event(vp->mpv, 0);
        if (event == NULL || event->event_id == MPV_EVENT_NONE) {
            break;
        }
        if (event->event_id == MPV_EVENT_LOG_MESSAGE) {
            mpv_event_log_message *log = static_cast<mpv_event_log_message *>(event->data);
            if (log != NULL && log->text != NULL) {
                ciao_warn("mpv: %s", log->text);
            }
            continue;
        }
        if (event->event_id == MPV_EVENT_FILE_LOADED) {
            ciao_info("mpv file loaded");
            g_timeout_add(1000, log_time_pos_once, vp);
            continue;
        }
        if (event->event_id == MPV_EVENT_END_FILE && event->data != NULL) {
            mpv_event_end_file *end = static_cast<mpv_event_end_file *>(event->data);
            if (end->reason == MPV_END_FILE_REASON_ERROR) {
                const char *why = mpv_error_string(end->error);
                ciao_error("mpv playback failed: %s", why);
                gchar *body = g_strdup_printf("Cannot play this mp4.\n%s", why);
                show_video_error(vp, body);
                g_free(body);
            }
            continue;
        }
        if (event->event_id == MPV_EVENT_SHUTDOWN) {
            ciao_warn("mpv core shut down");
        }
    }
    return G_SOURCE_REMOVE;
}

extern "C" void mpv_wakeup_cb(void *ctx)
{
    VideoPlayer *vp = static_cast<VideoPlayer *>(ctx);
    if (vp == NULL || !g_atomic_int_get(&vp->alive)) {
        return;
    }
    if (g_atomic_int_compare_and_exchange(&vp->events_scheduled, 0, 1)) {
        g_idle_add(events_idle, vp);
    }
}

static void send_mpv_command(VideoPlayer *vp, const char *const *args)
{
    if (vp == NULL || vp->mpv == NULL || args == NULL || !g_atomic_int_get(&vp->alive)) {
        return;
    }
    int rc = mpv_command_async(vp->mpv, 0, const_cast<const char **>(args));
    if (rc < 0) {
        ciao_warn("mpv command %s failed: %s", args[0] != NULL ? args[0] : "?", mpv_error_string(rc));
    }
}

static void send_mouse(VideoPlayer *vp, double x, double y, int button)
{
    if (vp == NULL || vp->gl_area == NULL) {
        return;
    }
    int scale = gtk_widget_get_scale_factor(vp->gl_area);
    if (scale < 1) {
        scale = 1;
    }
    gchar *xs = g_strdup_printf("%d", static_cast<int>(x * scale));
    gchar *ys = g_strdup_printf("%d", static_cast<int>(y * scale));
    if (button < 0) {
        const char *cmd[] = {"mouse", xs, ys, NULL};
        send_mpv_command(vp, cmd);
    } else {
        gchar *bs = g_strdup_printf("%d", button);
        const char *cmd[] = {"mouse", xs, ys, bs, NULL};
        send_mpv_command(vp, cmd);
        g_free(bs);
    }
    g_free(xs);
    g_free(ys);
}

static const char *mpv_key_name(guint keyval)
{
    switch (keyval) {
    case GDK_KEY_space: return "SPACE";
    case GDK_KEY_Return: return "ENTER";
    case GDK_KEY_KP_Enter: return "ENTER";
    case GDK_KEY_Left: return "LEFT";
    case GDK_KEY_Right: return "RIGHT";
    case GDK_KEY_Up: return "UP";
    case GDK_KEY_Down: return "DOWN";
    case GDK_KEY_Escape: return "ESC";
    case GDK_KEY_BackSpace: return "BS";
    case GDK_KEY_Home: return "HOME";
    case GDK_KEY_End: return "END";
    default:
        break;
    }
    if (keyval >= 32 && keyval < 127) {
        static char ascii[2];
        ascii[0] = static_cast<char>(keyval);
        ascii[1] = '\0';
        return ascii;
    }
    return NULL;
}

static void ensure_render_context(GtkGLArea *area, VideoPlayer *vp)
{
    if (vp->render != NULL || vp->mpv == NULL || vp->embed_failed) {
        return;
    }
    gtk_gl_area_make_current(area);
    if (gtk_gl_area_get_error(area) != NULL) {
        ciao_error("GtkGLArea: %s", gtk_gl_area_get_error(area)->message);
        vp->embed_failed = TRUE;
        return;
    }

    const GLubyte *version = glGetString(GL_VERSION);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    ciao_info("OpenGL version %s renderer %s",
              version != NULL ? reinterpret_cast<const char *>(version) : "?",
              renderer != NULL ? reinterpret_cast<const char *>(renderer) : "?");

    mpv_opengl_init_params gl_init;
    std::memset(&gl_init, 0, sizeof(gl_init));
    gl_init.get_proc_address = mpv_get_proc_address;
    gl_init.get_proc_address_ctx = NULL;

    int advanced = 1;
    mpv_render_param params[5];
    int count = 0;
    params[count].type = MPV_RENDER_PARAM_API_TYPE;
    params[count].data = const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL);
    count++;
    params[count].type = MPV_RENDER_PARAM_OPENGL_INIT_PARAMS;
    params[count].data = &gl_init;
    count++;
    params[count].type = MPV_RENDER_PARAM_ADVANCED_CONTROL;
    params[count].data = &advanced;
    count++;

    GdkDisplay *display = gtk_widget_get_display(GTK_WIDGET(area));
#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_DISPLAY(display)) {
        params[count].type = MPV_RENDER_PARAM_WL_DISPLAY;
        params[count].data = gdk_wayland_display_get_wl_display(display);
        count++;
    }
#endif
#ifdef GDK_WINDOWING_X11
    if (count == 3 && GDK_IS_X11_DISPLAY(display)) {
        params[count].type = MPV_RENDER_PARAM_X11_DISPLAY;
        params[count].data = gdk_x11_display_get_xdisplay(display);
        count++;
    }
#endif
    params[count].type = MPV_RENDER_PARAM_INVALID;
    params[count].data = NULL;

    int rc = mpv_render_context_create(&vp->render, vp->mpv, params);
    if (rc < 0) {
        ciao_error("mpv_render_context_create failed: %s", mpv_error_string(rc));
        vp->render = NULL;
        vp->embed_failed = TRUE;
        return;
    }
    mpv_render_context_set_update_callback(vp->render, mpv_render_update_cb, vp);
    ciao_info("mpv OpenGL view ready");
    submit_pending(vp);
}

static gboolean on_gl_render(GtkGLArea *area, GdkGLContext *context, gpointer data)
{
    (void)context;
    VideoPlayer *vp = static_cast<VideoPlayer *>(data);
    if (vp == NULL || !g_atomic_int_get(&vp->alive)) {
        return FALSE;
    }
    ensure_render_context(area, vp);
    if (vp->render == NULL || vp->embed_failed) {
        return FALSE;
    }

    gtk_gl_area_attach_buffers(area);
    int scale = gtk_widget_get_scale_factor(GTK_WIDGET(area));
    if (scale < 1) {
        scale = 1;
    }
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(area)) * scale;
    int height = gtk_widget_get_allocated_height(GTK_WIDGET(area)) * scale;
    if (width <= 0 || height <= 0) {
        return FALSE;
    }

    while (glGetError() != GL_NO_ERROR) {
    }

    GLint fbo_id = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo_id);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        return FALSE;
    }

    mpv_opengl_fbo fbo;
    std::memset(&fbo, 0, sizeof(fbo));
    fbo.fbo = static_cast<int>(fbo_id);
    fbo.w = width;
    fbo.h = height;
    fbo.internal_format = 0;

    int flip_y = 1;
    int block = 0;
    mpv_render_param params[4];
    params[0].type = MPV_RENDER_PARAM_OPENGL_FBO;
    params[0].data = &fbo;
    params[1].type = MPV_RENDER_PARAM_FLIP_Y;
    params[1].data = &flip_y;
    params[2].type = MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME;
    params[2].data = &block;
    params[3].type = MPV_RENDER_PARAM_INVALID;
    params[3].data = NULL;

    mpv_render_context_render(vp->render, params);
    return TRUE;
}

static void on_gl_realize(GtkGLArea *area, gpointer data)
{
    VideoPlayer *vp = static_cast<VideoPlayer *>(data);
    gtk_gl_area_make_current(area);
    if (gtk_gl_area_get_error(area) != NULL) {
        ciao_error("GtkGLArea: %s", gtk_gl_area_get_error(area)->message);
        vp->embed_failed = TRUE;
        return;
    }
    if (vp->mpv == NULL) {
        vp->embed_failed = TRUE;
    }
}

static void on_gl_resize(GtkGLArea *area, gint width, gint height, gpointer data)
{
    (void)width;
    (void)height;
    VideoPlayer *vp = static_cast<VideoPlayer *>(data);
    if (vp != NULL && vp->render != NULL && g_atomic_int_get(&vp->alive)) {
        gtk_gl_area_queue_render(area);
    }
}

static void on_gl_destroy(GtkWidget *widget, gpointer data)
{
    VideoPlayer *vp = static_cast<VideoPlayer *>(data);
    g_atomic_int_set(&vp->alive, 0);
    if (vp->mpv != NULL) {
        mpv_set_wakeup_callback(vp->mpv, NULL, NULL);
    }
    if (vp->render != NULL) {
        mpv_render_context_set_update_callback(vp->render, NULL, NULL);
        gtk_gl_area_make_current(GTK_GL_AREA(widget));
        mpv_render_context_free(vp->render);
        vp->render = NULL;
    }
    if (vp->mpv != NULL) {
        mpv_terminate_destroy(vp->mpv);
        vp->mpv = NULL;
    }
    g_free(vp->pending_url);
    vp->pending_url = NULL;
    g_free(vp->current_url);
    vp->current_url = NULL;
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    gtk_widget_grab_focus(widget);
    if (event != NULL) {
        send_mouse(static_cast<VideoPlayer *>(data), event->x, event->y,
                   static_cast<int>(event->button) - 1);
    }
    return TRUE;
}

static gboolean on_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
    (void)widget;
    if (event != NULL) {
        send_mouse(static_cast<VideoPlayer *>(data), event->x, event->y, -1);
    }
    return FALSE;
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    (void)widget;
    if (event == NULL) {
        return FALSE;
    }
    if ((event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0) {
        return FALSE;
    }
    const char *name = mpv_key_name(event->keyval);
    if (name == NULL) {
        return FALSE;
    }
    const char *cmd[] = {"keypress", name, NULL};
    send_mpv_command(static_cast<VideoPlayer *>(data), cmd);
    return TRUE;
}

static gboolean on_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
    (void)widget;
    if (event == NULL) {
        return FALSE;
    }
    const char *name = NULL;
    if (event->direction == GDK_SCROLL_UP || event->delta_y < 0) {
        name = "WHEEL_UP";
    } else if (event->direction == GDK_SCROLL_DOWN || event->delta_y > 0) {
        name = "WHEEL_DOWN";
    }
    if (name == NULL) {
        return FALSE;
    }
    const char *cmd[] = {"keypress", name, NULL};
    send_mpv_command(static_cast<VideoPlayer *>(data), cmd);
    return TRUE;
}

static gboolean set_mpv_option(mpv_handle *mpv, const char *name, const char *value)
{
    int rc = mpv_set_option_string(mpv, name, value);
    if (rc < 0) {
        ciao_error("mpv option %s=%s failed: %s", name, value, mpv_error_string(rc));
        return FALSE;
    }
    return TRUE;
}

static void submit_pending(VideoPlayer *vp)
{
    if (vp == NULL || vp->pending_url == NULL) {
        return;
    }
    if (vp->embed_failed || vp->mpv == NULL || vp->render == NULL) {
        return;
    }

    g_free(vp->current_url);
    vp->current_url = vp->pending_url;
    vp->pending_url = NULL;

    const char *cmd[] = {"loadfile", vp->current_url, NULL};
    int rc = mpv_command_async(vp->mpv, 0, cmd);
    if (rc < 0) {
        ciao_error("mpv loadfile failed: %s", mpv_error_string(rc));
        return;
    }
    ciao_info("mpv loadfile %s", vp->current_url);
}

static gboolean spawn_external_mpv(const char *url)
{
    if (url == NULL || url[0] == '\0') {
        return FALSE;
    }
    gchar *bin = g_find_program_in_path("mpv");
    if (bin == NULL) {
        ciao_error("mpv program not found; cannot play %s", url);
        return FALSE;
    }
    gchar *keep_open = g_strdup("--keep-open=yes");
    gchar *force_window = g_strdup("--force-window=yes");
    gchar *target = g_strdup(url);
    gchar *argv[] = {bin, keep_open, force_window, target, NULL};
    GError *error = NULL;
    gboolean ok = g_spawn_async(NULL, argv, NULL, static_cast<GSpawnFlags>(G_SPAWN_SEARCH_PATH),
                                NULL, NULL, NULL, &error);
    if (!ok) {
        ciao_error("failed to start mpv: %s", error != NULL ? error->message : "unknown");
        if (error != NULL) {
            g_error_free(error);
        }
    } else {
        ciao_info("Opened the mpv window for %s", url);
    }
    g_free(bin);
    g_free(keep_open);
    g_free(force_window);
    g_free(target);
    return ok;
}

gboolean video_player_url_is_mp4(const char *url)
{
    if (url == NULL) {
        return FALSE;
    }
    gchar *copy = g_strdup(url);
    g_strstrip(copy);
    if (copy[0] == '\0') {
        g_free(copy);
        return FALSE;
    }

    gchar *cut = std::strchr(copy, '#');
    if (cut != NULL) {
        *cut = '\0';
    }
    cut = std::strchr(copy, '?');
    if (cut != NULL) {
        *cut = '\0';
    }

    gsize length = std::strlen(copy);
    while (length > 0 && copy[length - 1] == '/') {
        copy[--length] = '\0';
    }

    gboolean match = FALSE;
    if (length >= 4) {
        match = g_ascii_strcasecmp(copy + (length - 4), ".mp4") == 0;
    }
    g_free(copy);
    return match;
}

VideoPlayer *video_player_new(WebKitWebView *web_view)
{
    if (g_player.gl_area != NULL) {
        g_player.web_view = web_view;
        return &g_player;
    }

    std::memset(&g_player, 0, sizeof(g_player));
    g_player.web_view = web_view;

    setlocale(LC_NUMERIC, "C");

    g_player.gl_area = gtk_gl_area_new();
    if (g_player.gl_area == NULL) {
        ciao_error("video_player_new: GtkGLArea creation failed");
        return NULL;
    }
    gtk_widget_set_hexpand(g_player.gl_area, TRUE);
    gtk_widget_set_vexpand(g_player.gl_area, TRUE);
    gtk_gl_area_set_use_es(GTK_GL_AREA(g_player.gl_area), FALSE);
    gtk_gl_area_set_required_version(GTK_GL_AREA(g_player.gl_area), 3, 2);
    gtk_gl_area_set_has_alpha(GTK_GL_AREA(g_player.gl_area), TRUE);
    gtk_widget_set_can_focus(g_player.gl_area, TRUE);
    gtk_widget_add_events(g_player.gl_area,
                          GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                          GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK |
                          GDK_KEY_PRESS_MASK | GDK_FOCUS_CHANGE_MASK);

    g_signal_connect(g_player.gl_area, "realize", G_CALLBACK(on_gl_realize), &g_player);
    g_signal_connect(g_player.gl_area, "render", G_CALLBACK(on_gl_render), &g_player);
    g_signal_connect(g_player.gl_area, "resize", G_CALLBACK(on_gl_resize), &g_player);
    g_signal_connect(g_player.gl_area, "destroy", G_CALLBACK(on_gl_destroy), &g_player);
    g_signal_connect(g_player.gl_area, "button-press-event", G_CALLBACK(on_button_press), &g_player);
    g_signal_connect(g_player.gl_area, "motion-notify-event", G_CALLBACK(on_motion), &g_player);
    g_signal_connect(g_player.gl_area, "key-press-event", G_CALLBACK(on_key_press), &g_player);
    g_signal_connect(g_player.gl_area, "scroll-event", G_CALLBACK(on_scroll), &g_player);

    g_player.mpv = mpv_create();
    if (g_player.mpv == NULL) {
        ciao_error("mpv_create failed; Go on an mp4 will open the mpv program");
        g_player.embed_failed = TRUE;
        return &g_player;
    }

    gboolean options_ok = TRUE;
    options_ok = set_mpv_option(g_player.mpv, "vo", "libmpv") && options_ok;
    options_ok = set_mpv_option(g_player.mpv, "config", "no") && options_ok;
    options_ok = set_mpv_option(g_player.mpv, "load-scripts", "no") && options_ok;
    options_ok = set_mpv_option(g_player.mpv, "ytdl", "no") && options_ok;
    options_ok = set_mpv_option(g_player.mpv, "idle", "yes") && options_ok;
    options_ok = set_mpv_option(g_player.mpv, "keep-open", "yes") && options_ok;
    options_ok = set_mpv_option(g_player.mpv, "force-window", "no") && options_ok;
    options_ok = set_mpv_option(g_player.mpv, "osc", "yes") && options_ok;
    options_ok = set_mpv_option(g_player.mpv, "input-default-bindings", "yes") && options_ok;
    // Software decode. auto hwdec probes libcuda and prints an error on
    // machines that do not have it.
    options_ok = set_mpv_option(g_player.mpv, "hwdec", "no") && options_ok;
    options_ok = set_mpv_option(g_player.mpv, "video-timing-offset", "0") && options_ok;
    if (!options_ok) {
        mpv_terminate_destroy(g_player.mpv);
        g_player.mpv = NULL;
        g_player.embed_failed = TRUE;
        return &g_player;
    }

    int rc = mpv_initialize(g_player.mpv);
    if (rc < 0) {
        ciao_error("mpv_initialize failed: %s", mpv_error_string(rc));
        mpv_terminate_destroy(g_player.mpv);
        g_player.mpv = NULL;
        g_player.embed_failed = TRUE;
        return &g_player;
    }

    g_atomic_int_set(&g_player.alive, 1);
    mpv_request_log_messages(g_player.mpv, "error");
    mpv_set_wakeup_callback(g_player.mpv, mpv_wakeup_cb, &g_player);
    ciao_info("mpv client ready");
    return &g_player;
}

GtkWidget *video_player_widget(VideoPlayer *player)
{
    if (player == NULL) {
        return NULL;
    }
    return player->gl_area;
}

void video_player_set_stack(VideoPlayer *player, GtkWidget *stack)
{
    if (player == NULL) {
        return;
    }
    player->stack = stack;
}

gboolean video_player_is_showing(VideoPlayer *player)
{
    if (player == NULL || player->stack == NULL || player->gl_area == NULL) {
        return FALSE;
    }
    if (!GTK_IS_STACK(player->stack)) {
        return FALSE;
    }
    return gtk_stack_get_visible_child(GTK_STACK(player->stack)) == player->gl_area;
}

gboolean video_player_play(VideoPlayer *player, const char *url)
{
    if (player == NULL || url == NULL) {
        return FALSE;
    }
    gchar *clean = g_strdup(url);
    g_strstrip(clean);
    if (clean[0] == '\0') {
        g_free(clean);
        return FALSE;
    }

    if (!g_atomic_int_get(&player->alive) || player->mpv == NULL || player->embed_failed) {
        gboolean ok = spawn_external_mpv(clean);
        g_free(clean);
        return ok;
    }

    g_free(player->pending_url);
    player->pending_url = clean;
    ciao_info("Playing mp4 in the mpv view: %s", clean);

    if (player->stack != NULL && GTK_IS_STACK(player->stack)) {
        gtk_stack_set_visible_child(GTK_STACK(player->stack), player->gl_area);
    }
    if (player->gl_area != NULL && !gtk_widget_get_realized(player->gl_area)) {
        gtk_widget_realize(player->gl_area);
    }

    if (player->embed_failed || player->mpv == NULL) {
        if (player->stack != NULL && player->web_view != NULL && GTK_IS_STACK(player->stack)) {
            gtk_stack_set_visible_child(GTK_STACK(player->stack), GTK_WIDGET(player->web_view));
        }
        gchar *fallback = player->pending_url;
        player->pending_url = NULL;
        gboolean ok = spawn_external_mpv(fallback);
        g_free(fallback);
        return ok;
    }

    if (player->gl_area != NULL) {
        gtk_widget_grab_focus(player->gl_area);
    }
    if (player->render == NULL) {
        ciao_debug("mpv view will start when the OpenGL framebuffer is ready");
        return TRUE;
    }

    submit_pending(player);
    return TRUE;
}

void video_player_stop(VideoPlayer *player)
{
    if (player == NULL) {
        return;
    }
    g_free(player->pending_url);
    player->pending_url = NULL;
    if (!video_player_is_showing(player)) {
        return;
    }
    if (player->mpv != NULL && g_atomic_int_get(&player->alive)) {
        const char *cmd[] = {"stop", NULL};
        send_mpv_command(player, cmd);
    }
    if (player->stack != NULL && player->web_view != NULL && GTK_IS_STACK(player->stack)) {
        gtk_stack_set_visible_child(GTK_STACK(player->stack), GTK_WIDGET(player->web_view));
    }
    ciao_info("mpv view hidden; web view restored");
}

double video_player_time_pos(VideoPlayer *player)
{
    if (player == NULL || player->mpv == NULL || !g_atomic_int_get(&player->alive)) {
        return -1.0;
    }
    double pos = -1.0;
    if (mpv_get_property(player->mpv, "time-pos", MPV_FORMAT_DOUBLE, &pos) < 0) {
        return -1.0;
    }
    return pos;
}
