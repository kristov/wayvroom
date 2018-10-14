#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <stdbool.h>

#include "surface.h"

static void destroy(struct wl_client *client, struct wl_resource *resource) {
    fprintf(stderr, "surface.c: destroy()\n");
    wl_resource_destroy(resource);
}

static void attach(struct wl_client *client, struct wl_resource *resource, struct wl_resource *buffer_resource, int32_t x, int32_t y) {
    fprintf(stderr, "surface.c: attach()\n");
}

static void damage(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) {
    fprintf(stderr, "surface.c: damage()\n");
}

static void frame(struct wl_client *client, struct wl_resource *resource, uint32_t id) {
    fprintf(stderr, "surface.c: frame(id:%d)\n", id);
}

static void set_opaque_region(struct wl_client *client, struct wl_resource *resource, struct wl_resource *region_resource) {
    fprintf(stderr, "surface.c: set_opaque_region()\n");
}

static void set_input_region(struct wl_client *client, struct wl_resource *resource, struct wl_resource *region_resource) {
    fprintf(stderr, "surface.c: set_input_region()\n");
}

static void commit(struct wl_client *client, struct wl_resource *resource) {
    fprintf(stderr, "surface.c: commit()\n");
}

void set_buffer_transform(struct wl_client *client, struct wl_resource *surface, int32_t transform) {
    fprintf(stderr, "surface.c: set_buffer_transform()\n");
}

void set_buffer_scale(struct wl_client *client, struct wl_resource *surface, int32_t scale) {
    fprintf(stderr, "surface.c: set_buffer_scale()\n");
}

static struct wl_surface_interface surface_implementation = {
    .destroy = destroy,
    .attach = attach,
    .damage = damage,
    .frame = frame,
    .set_opaque_region = set_opaque_region,
    .set_input_region = set_input_region,
    .commit = commit,
    .set_buffer_transform = set_buffer_transform,
    .set_buffer_scale = set_buffer_scale,
};

static void surface_destroy(struct wl_resource *resource) {
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
    wl_resource_set_implementation(surface->resource, &surface_implementation, surface, &surface_destroy);

    return surface;
}
