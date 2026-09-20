#include "mbr/application/application.h"
#include "mbr/renderer/renderer.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    MbrRegistry registry;
    MbrTarget custom[5];
    MbrTarget draft[5];
    bool dirty;
} SimPersistentState;

static MbrApp app;
static SimPersistentState persistent;
static uint16_t pixels[MBR_LCD_WIDTH * MBR_LCD_HEIGHT];
static uint32_t now_ms;
static unsigned backlight_gain_percent = 300u;

static unsigned effective_backlight_percent(void)
{
    return app.locked ? 0u : backlight_gain_percent;
}

static unsigned char apply_backlight(unsigned char value, unsigned gain_percent)
{
    unsigned scaled = ((unsigned)value * gain_percent + 50u) / 100u;
    return (unsigned char)(scaled > 255u ? 255u : scaled);
}

static const char *search_name(MbrSearch search)
{
    switch (search) {
    case MBR_SEARCH_NONE: return "NONE";
    case MBR_SEARCH_FIRST: return "FIRST";
    case MBR_SEARCH_SAVED: return "SAVED";
    case MBR_SEARCH_NEW: return "NEW";
    default: return "?";
    }
}

static bool parse_control(const char *name, MbrControl *out)
{
    static const char *names[MBR_CONTROL_COUNT] = {
        "up", "down", "left", "right", "press", "a", "b", "x", "y"
    };
    for (unsigned i = 0; i < MBR_CONTROL_COUNT; ++i) {
        if (strcmp(name, names[i]) == 0) {
            *out = (MbrControl)i;
            return true;
        }
    }
    return false;
}

static int find_screen(const char *id)
{
    for (int i = 0; i < MBR_SCREEN_COUNT; ++i) {
        if (strcmp(mbr_screens[i].id, id) == 0) return i;
    }
    return -1;
}

static void persist_from_app(void)
{
    persistent.registry = app.registry;
    memcpy(persistent.custom, app.custom, sizeof(persistent.custom));
    memcpy(persistent.draft, app.draft, sizeof(persistent.draft));
    persistent.dirty = app.custom_dirty;
}

static void restore_to_app(void)
{
    app.registry = persistent.registry;
    memcpy(app.custom, persistent.custom, sizeof(app.custom));
    memcpy(app.draft, persistent.draft, sizeof(app.draft));
    app.custom_dirty = persistent.dirty;
    mbr_app_home(&app);
}

static void reboot_app(bool keep_storage)
{
    if (keep_storage) persist_from_app();
    else memset(&persistent, 0, sizeof(persistent));
    mbr_app_init(&app, now_ms);
    if (keep_storage) restore_to_app();
}

static void tap(MbrControl control)
{
    mbr_app_event(&app, control, true);
    mbr_app_event(&app, control, false);
}

static void settle_requests(void)
{
    while (app.request.kind != MBR_OP_NONE && app.request.kind != MBR_OP_HANDOFF) {
        const uint32_t token = app.request.token;
        if (!mbr_app_confirm(&app, token, true)) break;
        persist_from_app();
    }
}

static bool connect_mouse(MouseId id, const char *name)
{
    if (id == 0 || name == NULL || *name == '\0') return false;
    MbrMouse mouse;
    memset(&mouse, 0, sizeof(mouse));
    mouse.id = id;
    mouse.profile = MBR_PASSTHROUGH;
    mouse.bonded = true;
    mouse.address_type = 0;
    for (unsigned i = 0; i < sizeof(mouse.address); ++i)
        mouse.address[i] = (uint8_t)((id >> ((i % 4u) * 8u)) + i + 1u);
    strncpy(mouse.name, name, sizeof(mouse.name) - 1u);
    if (!mbr_app_ready(&app, app.search.generation, &mouse)) return false;
    const uint32_t token = app.request.token;
    if (!mbr_app_confirm(&app, token, true)) return false;
    persist_from_app();
    return true;
}

static void disconnect_mouse(void)
{
    if (app.sessions.live.ready)
        mbr_app_disconnected(&app, app.sessions.live.generation);
}

static bool write_ppm(const char *path)
{
    MbrView view;
    MbrFrame frame;
    mbr_app_view(&app, &view);
    mbr_project(&view, &frame);
    mbr_render(&frame, pixels);
    const unsigned effective_gain = effective_backlight_percent();

    FILE *file = fopen(path, "wb");
    if (file == NULL) return false;
    if (fprintf(file, "P6\n%u %u\n255\n", MBR_LCD_WIDTH, MBR_LCD_HEIGHT) < 0) {
        fclose(file);
        return false;
    }
    for (unsigned i = 0; i < MBR_LCD_WIDTH * MBR_LCD_HEIGHT; ++i) {
        const uint16_t pixel = pixels[i];
        const unsigned char native_rgb[3] = {
            (unsigned char)(((pixel >> 11u) & 31u) * 255u / 31u),
            (unsigned char)(((pixel >> 5u) & 63u) * 255u / 63u),
            (unsigned char)((pixel & 31u) * 255u / 31u)
        };
        const unsigned char rgb[3] = {
            apply_backlight(native_rgb[0], effective_gain),
            apply_backlight(native_rgb[1], effective_gain),
            apply_backlight(native_rgb[2], effective_gain)
        };
        if (fwrite(rgb, 1u, sizeof(rgb), file) != sizeof(rgb)) {
            fclose(file);
            return false;
        }
    }
    return fclose(file) == 0;
}

static void print_state(const char *result)
{
    MbrView view;
    mbr_app_view(&app, &view);
    printf("%s\t%s\tlocked=%u\tbacklight=%u\teffective_backlight=%u\tsearch=%s\tcount=%u\tconnected=%u\tselection=%u\tpage=%u\tname=%s\n",
           result,
           app.screen < MBR_SCREEN_COUNT ? mbr_screens[app.screen].id : "invalid",
           app.locked ? 1u : 0u,
           backlight_gain_percent,
           effective_backlight_percent(),
           search_name(app.search.purpose),
           (unsigned)app.registry.count,
           app.sessions.live.ready ? 1u : 0u,
           (unsigned)app.selection,
           (unsigned)app.page,
           view.name[0] ? view.name : "-");
    fflush(stdout);
}

static char *skip_space(char *s)
{
    while (*s != '\0' && isspace((unsigned char)*s)) ++s;
    return s;
}

static void trim_end(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1u])) s[--n] = '\0';
}

static const char *execute(char *line)
{
    trim_end(line);
    char *command = skip_space(line);
    if (*command == '\0') return "OK";

    char *arg = command;
    while (*arg != '\0' && !isspace((unsigned char)*arg)) ++arg;
    if (*arg != '\0') *arg++ = '\0';
    arg = skip_space(arg);

    if (strcmp(command, "tap") == 0 || strcmp(command, "down") == 0 || strcmp(command, "up") == 0) {
        MbrControl control;
        if (!parse_control(arg, &control)) return "ERR unknown-control";
        if (strcmp(command, "tap") == 0) tap(control);
        else mbr_app_event(&app, control, strcmp(command, "down") == 0);
        return "OK";
    }

    if (strcmp(command, "tick") == 0) {
        errno = 0;
        char *end = NULL;
        const unsigned long delta = strtoul(arg, &end, 10);
        if (errno != 0 || end == arg || *skip_space(end) != '\0') return "ERR invalid-time";
        now_ms += (uint32_t)delta;
        mbr_app_tick(&app, now_ms);
        return "OK";
    }

    if (strcmp(command, "connect") == 0) {
        char *id_text = arg;
        char *name = id_text;
        while (*name != '\0' && !isspace((unsigned char)*name)) ++name;
        if (*name == '\0') return "ERR connect-needs-id-and-name";
        *name++ = '\0';
        name = skip_space(name);
        errno = 0;
        char *end = NULL;
        const unsigned long id = strtoul(id_text, &end, 10);
        if (errno != 0 || end == id_text || *end != '\0' || id == 0 || id > UINT32_MAX)
            return "ERR invalid-mouse-id";
        return connect_mouse((MouseId)id, name) ? "OK" : "ERR mouse-not-eligible-for-current-search";
    }

    if (strcmp(command, "disconnect") == 0) {
        disconnect_mouse();
        return "OK";
    }

    if (strcmp(command, "brightness") == 0) {
        errno = 0;
        char *end = NULL;
        const unsigned long gain = strtoul(arg, &end, 10);
        if (errno != 0 || end == arg || *skip_space(end) != '\0' || gain > 1000u)
            return "ERR brightness-must-be-0-to-1000";
        backlight_gain_percent = (unsigned)gain;
        return "OK";
    }

    if (strcmp(command, "home") == 0) {
        mbr_app_home(&app);
        return "OK";
    }

    if (strcmp(command, "reboot") == 0) {
        reboot_app(true);
        return "OK";
    }

    if (strcmp(command, "factory-reset") == 0) {
        reboot_app(false);
        return "OK";
    }

    if (strcmp(command, "screen") == 0) {
        const int screen = find_screen(arg);
        if (screen < 0) return "ERR unknown-screen";
        app.screen = (MbrScreen)screen;
        app.selection = 0;
        ++app.epoch;
        return "OK";
    }

    if (strcmp(command, "status") == 0) return "OK";
    return "ERR unknown-command";
}

static int smoke_test(void)
{
    now_ms = 0;
    backlight_gain_percent = 300u;
    memset(&persistent, 0, sizeof(persistent));
    mbr_app_init(&app, now_ms);
    assert(app.screen == MBR_SCREEN_SEARCHING_FIRST);
    assert(apply_backlight(8u, 100u) == 8u);
    assert(apply_backlight(8u, 300u) == 24u);
    assert(apply_backlight(200u, 200u) == 255u);
    assert(apply_backlight(0u, 1000u) == 0u);
    assert(apply_backlight(8u, 1000u) == 80u);
    assert(apply_backlight(30u, 1000u) == 255u);
    assert(effective_backlight_percent() == 300u);
    assert(connect_mouse(1u, "LOGITECH LIFT"));
    assert(app.screen == MBR_SCREEN_FIRST_MOUSE_CONNECTED);
    tap(MBR_Y);
    assert(app.screen == MBR_SCREEN_HOME_CONNECTED);
    tap(MBR_DOWN);
    tap(MBR_PRESS);
    assert(app.screen == MBR_SCREEN_REMAPPER_OPTIONS);
    tap(MBR_DOWN);
    tap(MBR_PRESS);
    assert(app.screen == MBR_SCREEN_STANDARD_NOT_ACTIVE);
    tap(MBR_A);
    settle_requests();
    assert(app.screen == MBR_SCREEN_STANDARD_ACTIVE);
    mbr_app_home(&app);
    app.selection = 0;
    tap(MBR_PRESS);
    assert(app.screen == MBR_SCREEN_PAIR_NEW);
    tap(MBR_Y);
    assert(app.locked);
    assert(effective_backlight_percent() == 0u);
    tap(MBR_X);
    assert(!app.locked);
    assert(effective_backlight_percent() == 300u);
    assert(connect_mouse(2u, "GENERIC MOUSE"));
    assert(app.screen == MBR_SCREEN_HOME_CONNECTED);
    assert(app.registry.count == 2u);
    disconnect_mouse();
    assert(app.screen == MBR_SCREEN_HOME_SEARCHING);
    now_ms += MBR_SAVED_MS;
    mbr_app_tick(&app, now_ms);
    assert(app.screen == MBR_SCREEN_HOME_RETRY);
    reboot_app(true);
    assert(app.registry.count == 2u);
    assert(app.screen == MBR_SCREEN_HOME_SEARCHING);
    puts("lcd simulator smoke PASS");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--smoke") == 0) return smoke_test();
    if (argc != 2) {
        fprintf(stderr, "usage: %s FRAME.ppm | --smoke\n", argv[0]);
        return 2;
    }

    const char *frame_path = argv[1];
    now_ms = 0;
    backlight_gain_percent = 300u;
    memset(&persistent, 0, sizeof(persistent));
    mbr_app_init(&app, now_ms);
    if (!write_ppm(frame_path)) {
        perror("write initial frame");
        return 1;
    }
    setvbuf(stdout, NULL, _IOLBF, 0);
    print_state("OK");

    char line[512];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        trim_end(line);
        if (strcmp(skip_space(line), "quit") == 0) break;
        const char *result = execute(line);
        if (strncmp(result, "OK", 2u) == 0) settle_requests();
        if (!write_ppm(frame_path)) {
            print_state("ERR frame-write");
            continue;
        }
        print_state(result);
    }
    return 0;
}
