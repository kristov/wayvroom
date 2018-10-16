#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <stdbool.h>

#include "surface.h"

static void surface_destroy(struct wl_client *client, struct wl_resource *resource) {
    fprintf(stderr, "surface.c: surface_destroy()\n");
    wl_resource_destroy(resource);
}

static void surface_attach(struct wl_client* client, struct wl_resource* resource, struct wl_resource* buffer_resource, int32_t x, int32_t y) {
    struct surface* surface = wl_resource_get_user_data(resource);
    //buffer_reference = wl_resource_get_user_data(buffer_resource);
    //buffer_reference->texture_id

    //create vertex_id, normal_id, index_id
    //vrms_runtime_create_object_geometry();
    //vrms_runtime_create_object_mesh_texture();

    fprintf(stderr, "surface.c: surface_attach()\n");
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
    struct surface* surface = wl_resource_get_user_data(resource);
    fprintf(stderr, "surface.c: surface_destroy()\n");
    free(surface);
}

struct surface* surface_new(struct wl_client *client, uint32_t version, uint32_t id) {
    struct surface *surface;

    fprintf(stderr, "surface.c: surface_new(id:%d)\n", id);
    if (!(surface = malloc(sizeof(*surface))))
        return NULL;

    surface->resource = wl_resource_create(client, &wl_surface_interface, version, id);
    wl_resource_set_implementation(surface->resource, &surface_implementation, surface, &surface_resource_destroy);

    return surface;
}
