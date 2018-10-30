#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <stdbool.h>

#include "shm.h"
#include "surface.h"

static void surface_destroy(struct wl_client *client, struct wl_resource *resource) {
    fprintf(stderr, "surface.c: surface_destroy()\n");
    struct surface* surface = wl_resource_get_user_data(resource);
    wayvroom_server_t* server = surface->server;
    geometry_object_t* geometry = surface->geometry;
    vrms_runtime_t* vrms_runtime = server->vrms_runtime;

    geometry_destroy_screen(vrms_runtime, server->scene_id, geometry);

    wl_resource_destroy(resource);
}

static void surface_attach(struct wl_client* client, struct wl_resource* resource, struct wl_resource* buffer_resource, int32_t x, int32_t y) {
    fprintf(stderr, "surface.c: surface_attach()\n");

    struct surface* surface = wl_resource_get_user_data(resource);
    buffer_reference_t* reference = wl_resource_get_user_data(buffer_resource);
    wayvroom_server_t* server = reference->server;
    vrms_runtime_t* vrms_runtime = server->vrms_runtime;

    surface->server = server;
    surface->geometry = geometry_create_screen(vrms_runtime, server->scene_id, reference->texture_id, reference->width, reference->height);

    if (surface->buffer_resource) {
        fprintf(stderr, "surface.c: surface_attach(): buffer_resource already attached\n");
    }
    surface->buffer_resource = buffer_resource;
}

static void surface_damage(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) {
    fprintf(stderr, "surface.c: surface_damage()\n");
}

static void surface_frame(struct wl_client *client, struct wl_resource *resource, uint32_t id) {
    fprintf(stderr, "surface.c: surface_frame(id:%d)\n", id);
}

static void surface_set_opaque_region(struct wl_client *client, struct wl_resource *resource, struct wl_resource *region_resource) {
    fprintf(stderr, "surface.c: surface_set_opaque_region()\n");
}

static void surface_set_input_region(struct wl_client *client, struct wl_resource *resource, struct wl_resource *region_resource) {
    fprintf(stderr, "surface.c: surface_set_input_region()\n");
}

static void surface_commit(struct wl_client *client, struct wl_resource *resource) {
    fprintf(stderr, "surface.c: surface_commit()\n");
}

void surface_set_buffer_transform(struct wl_client *client, struct wl_resource *surface, int32_t transform) {
    fprintf(stderr, "surface.c: surface_set_buffer_transform()\n");
}

void surface_set_buffer_scale(struct wl_client *client, struct wl_resource *surface, int32_t scale) {
    fprintf(stderr, "surface.c: surface_set_buffer_scale()\n");
}

static struct wl_surface_interface surface_implementation = {
    .destroy = surface_destroy,
    .attach = surface_attach,
    .damage = surface_damage,
    .frame = surface_frame,
    .set_opaque_region = surface_set_opaque_region,
    .set_input_region = surface_set_input_region,
    .commit = surface_commit,
    .set_buffer_transform = surface_set_buffer_transform,
    .set_buffer_scale = surface_set_buffer_scale,
};

static void surface_resource_destroy(struct wl_resource *resource) {
    fprintf(stderr, "surface.c: surface_resource_destroy()\n");
    struct surface* surface = wl_resource_get_user_data(resource);
    wayvroom_server_t* server = surface->server;
    if (!server) {
        free(surface);
        return;
    }

    geometry_object_t* geometry = surface->geometry;
    vrms_runtime_t* vrms_runtime = server->vrms_runtime;

    geometry_destroy_screen(vrms_runtime, server->scene_id, geometry);

    free(surface);
}

struct surface* surface_new(struct wl_client *client, uint32_t version, uint32_t id) {
    fprintf(stderr, "surface.c: surface_new(id:%d)\n", id);

    struct surface *surface;
    if (!(surface = malloc(sizeof(*surface))))
        return NULL;

    surface->resource = wl_resource_create(client, &wl_surface_interface, version, id);
    wl_resource_set_implementation(surface->resource, &surface_implementation, surface, &surface_resource_destroy);

    return surface;
}
