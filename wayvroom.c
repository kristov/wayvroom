#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <wayland-server.h>
#include <stdbool.h>

#include "wayvroom.h"
#include "shm.h"
#include "compositor.h"
#include "shell.h"
#include "seat.h"

#include "vrms_runtime.h"

#include <unistd.h> // sleep() - remove when not needed

void* run_module(vrms_runtime_t* vrms_runtime) {
    wayvroom_server_t server;

    server.vrms_runtime = vrms_runtime;
    server.scene_id = vrms_runtime_create_scene(vrms_runtime, "wayvroom");

    server.wl_display = wl_display_create();
    assert(server.wl_display);
    server.wl_event_loop = wl_display_get_event_loop(server.wl_display);
    assert(server.wl_event_loop);

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    assert(socket);

    shm_initialize(&server);
    compositor_initialize(&server);
    shell_initialize(&server);
    seat_initialize(&server, "Wayvroom Seat");

    printf("Running compositor on wayland display '%s'\n", socket);
    setenv("WAYLAND_DISPLAY", socket, true);

    wl_display_run(server.wl_display);
    wl_display_destroy(server.wl_display);

    return NULL;
}
