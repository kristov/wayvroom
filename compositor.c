#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <stdbool.h>

#include "surface.h"

static struct {
    struct wl_global* global;
    struct {
        struct wl_signal new_surface;
    } signal;
} compositor;

static void create_surface(struct wl_client *client, struct wl_resource *resource, uint32_t id) {
    struct surface *surface;

    fprintf(stderr, "compositor.c: create_surface()\n");
    surface = surface_new(client, wl_resource_get_version(resource), id);

    if (!surface) {
        wl_resource_post_no_memory(resource);
        return;
    }

    wl_signal_emit(&compositor.signal.new_surface, surface);
}

static void create_region(struct wl_client *client, struct wl_resource *resource, uint32_t id) {
    fprintf(stderr, "compositor.c: create_region()\n");
/*
    struct region *region;

    region = region_new(client, wl_resource_get_version(resource), id);

    if (!region)
        wl_resource_post_no_memory(resource);
*/
}

static struct wl_compositor_interface compositor_implementation = {
    .create_surface = create_surface,
    .create_region = create_region,
};

static void bind_compositor(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
    struct wl_resource *resource;

    if (version > 3)
        version = 3;

    resource = wl_resource_create(client, &wl_compositor_interface, version, id);
	wl_resource_set_implementation(resource, &compositor_implementation, NULL, NULL);
}

bool compositor_initialize(struct wl_display* wl_display) {
    //struct screen *screen;
    //uint32_t keysym;

	fprintf(stderr, "compositor.c: compositor_initialize()\n");
    compositor.global = wl_global_create(wl_display, &wl_compositor_interface, 3, NULL, &bind_compositor);

    if (!compositor.global) {
        return false;
    }

    wl_signal_init(&compositor.signal.new_surface);

	return true;
}
