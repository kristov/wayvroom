#ifndef WAYVROOM_H
#define WAYVROOM_H

#include "vrms_runtime.h"

typedef struct wayvroom_server {
    vrms_runtime_t* vrms_runtime;
    uint32_t scene_id;
    struct wl_display* wl_display;
    struct wl_event_loop* wl_event_loop;
} wayvroom_server_t;

#endif
