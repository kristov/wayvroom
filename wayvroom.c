#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <wayland-server.h>
#include <stdbool.h>

#include "shm.h"
#include "compositor.h"
#include "shell.h"
#include "seat.h"

struct wvr_server {
    struct wl_display* wl_display;
    struct wl_event_loop* wl_event_loop;
};

void* run_module(void* data) {
    struct wvr_server server;

    server.wl_display = wl_display_create();
    assert(server.wl_display);
    server.wl_event_loop = wl_display_get_event_loop(server.wl_display);
    assert(server.wl_event_loop);

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    assert(socket);

    shm_initialize(server.wl_display);
    compositor_initialize(server.wl_display);
    shell_initialize(server.wl_display);
    seat_initialize(server.wl_display, "Test 123");

    printf("Running compositor on wayland display '%s'\n", socket);
    setenv("WAYLAND_DISPLAY", socket, true);

    wl_display_run(server.wl_display);
    wl_display_destroy(server.wl_display);

    return NULL;
}
