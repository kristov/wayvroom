#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <stdbool.h>

#include "surface.h"
#include "shell_surface.h"

static struct {
    struct wl_global *global;
} shell;

static void get_shell_surface(struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *surface_resource) {
    struct surface *surface = wl_resource_get_user_data(surface_resource);
    struct shell_surface *shell_surface;

    fprintf(stderr, "shell.c: get_shell_surface()\n");
    shell_surface = shell_surface_new(client, wl_resource_get_version(resource), id, surface);

    if (!shell_surface)
        wl_resource_post_no_memory(resource);
}

static const struct wl_shell_interface shell_implementation = {
    .get_shell_surface = get_shell_surface,
};

static void bind_shell(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
    struct wl_resource *resource;

    fprintf(stderr, "shell.c: bind_shell()\n");
    if (version > 1)
        version = 1;

    resource = wl_resource_create(client, &wl_shell_interface, version, id);
    wl_resource_set_implementation(resource, &shell_implementation, NULL, NULL);
}

bool shell_initialize(struct wl_display* wl_display) {
    fprintf(stderr, "shell.c: shell_initialize()\n");
    shell.global = wl_global_create(wl_display, &wl_shell_interface, 1, NULL, &bind_shell);
    return shell.global;
}
