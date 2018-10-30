#include "wayvroom.h"
#include "geometry.h"

struct surface {
    struct wl_resource* resource;
    struct wl_resource* buffer_resource;
    wayvroom_server_t* server;
    geometry_object_t* geometry;
};

struct surface* surface_new(struct wl_client *client, uint32_t version, uint32_t id);
