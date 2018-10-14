#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <stdbool.h>

#include "surface.h"

struct shell_surface {
    struct wl_resource *resource;
    struct wl_listener surface_destroy_listener;
};

static void pong(struct wl_client *client, struct wl_resource *resource, uint32_t serial) {
    fprintf(stderr, "shell_surface.c: pong()\n");
}

static void move(struct wl_client *client, struct wl_resource *resource, struct wl_resource *seat_resource, uint32_t serial) {
    fprintf(stderr, "shell_surface.c: move()\n");
}

static void resize(struct wl_client *client, struct wl_resource *resource, struct wl_resource *seat_resource, uint32_t serial, uint32_t edges) {
    fprintf(stderr, "shell_surface.c: resize()\n");
}

static void set_toplevel(struct wl_client *client, struct wl_resource *resource) {
    fprintf(stderr, "shell_surface.c: set_toplevel()\n");
}

static void set_transient(struct wl_client *client, struct wl_resource *resource, struct wl_resource *parent_resource, int32_t x, int32_t y, uint32_t flags) {
    fprintf(stderr, "shell_surface.c: set_transient()\n");
}

static void set_fullscreen(struct wl_client *client, struct wl_resource *resource, uint32_t method, uint32_t framerate, struct wl_resource *output_resource) {
    fprintf(stderr, "shell_surface.c: set_fullscreen()\n");
}

static void set_popup(struct wl_client *client, struct wl_resource *resource, struct wl_resource *seat_resource, uint32_t serial, struct wl_resource *parent_resource, int32_t x, int32_t y, uint32_t flags) {
    fprintf(stderr, "shell_surface.c: set_popup()\n");
}

static void set_maximized(struct wl_client *client, struct wl_resource *resource, struct wl_resource *output_resource) {
    fprintf(stderr, "shell_surface.c: set_maximized()\n");
}

static void set_title(struct wl_client *client, struct wl_resource *resource, const char *title) {
    fprintf(stderr, "shell_surface.c: set_title()\n");
}

static void set_class(struct wl_client *client, struct wl_resource *resource, const char *class) {
    fprintf(stderr, "shell_surface.c: set_class()\n");
}

static const struct wl_shell_surface_interface shell_surface_implementation = {
    .pong = pong,
    .move = move,
    .resize = resize,
    .set_toplevel = set_toplevel,
    .set_transient = set_transient,
    .set_fullscreen = set_fullscreen,
    .set_popup = set_popup,
    .set_maximized = set_maximized,
    .set_title = set_title,
    .set_class = set_class,
};

static void handle_surface_destroy(struct wl_listener *listener, void *data) {
    struct shell_surface *shell_surface = wl_container_of(listener, shell_surface, surface_destroy_listener);
    fprintf(stderr, "shell_surface.c: handle_surface_destroy()\n");
    wl_resource_destroy(shell_surface->resource);
}

static void destroy_shell_surface(struct wl_resource *resource) {
    struct shell_surface *shell_surface = wl_resource_get_user_data(resource);

    fprintf(stderr, "shell_surface.c: destroy_shell_surface()\n");
    free(shell_surface);
}

struct shell_surface* shell_surface_new(struct wl_client* client, uint32_t version, uint32_t id, struct surface* surface) {
    struct shell_surface *shell_surface;

    fprintf(stderr, "shell_surface.c: shell_surface_new(id:[%d])\n", id);
    shell_surface = malloc(sizeof(*shell_surface));

    if (!shell_surface)
        return NULL;

    shell_surface->resource = wl_resource_create(client, &wl_shell_surface_interface, version, id);

    if (!shell_surface->resource) {
        free(shell_surface);
        return NULL;
    }

    wl_resource_set_implementation(shell_surface->resource, &shell_surface_implementation, shell_surface, &destroy_shell_surface);
//    window_initialize(&shell_surface->window, &window_impl, surface);
    shell_surface->surface_destroy_listener.notify = &handle_surface_destroy;
    wl_resource_add_destroy_listener(surface->resource, &shell_surface->surface_destroy_listener);

    return shell_surface;
}
