#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <stdbool.h>

static void destroy(struct wl_client *client, struct wl_resource *resource) {
    fprintf(stderr, "wayland_buffer.c: destroy()\n");
	wl_resource_destroy(resource);
}

static const struct wl_buffer_interface buffer_implementation = {
	.destroy = destroy,
};

static void destroy_buffer(struct wl_resource *resource) {
    fprintf(stderr, "wayland_buffer.c: destroy_buffer()\n");
}

struct wl_resource* wayland_buffer_create_resource(struct wl_client* client, uint32_t version, uint32_t id) {
	struct wl_resource *resource;

    fprintf(stderr, "wayland_buffer.c: wayland_buffer_create_resource(id:%d)\n", id);
	resource = wl_resource_create(client, &wl_buffer_interface, version, id);

	if (!resource) {
		wl_client_post_no_memory(client);
		return NULL;
	}

	wl_resource_set_implementation(resource, &buffer_implementation, NULL, &destroy_buffer);

	return resource;
}
