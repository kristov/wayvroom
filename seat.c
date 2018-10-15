#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <stdbool.h>
#include <string.h>

#include "seat.h"

static struct {
	char *name;
	struct wl_global *global;
} seat;

static void get_pointer(struct wl_client *client, struct wl_resource *resource, uint32_t id) {
    fprintf(stderr, "seat.c: get_pointer()\n");
}

static void get_keyboard(struct wl_client *client, struct wl_resource *resource, uint32_t id) {
    fprintf(stderr, "seat.c: get_keyboard()\n");
}

static void get_touch(struct wl_client *client, struct wl_resource *resource, uint32_t id) {
    fprintf(stderr, "seat.c: get_touch()\n");
}

void remove_resource(struct wl_resource *resource) {
    fprintf(stderr, "seat.c: remove_resource()\n");
}

static struct wl_seat_interface seat_implementation = {
	.get_pointer = get_pointer,
	.get_keyboard = get_keyboard,
	.get_touch = get_touch,
};

static void bind_seat(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
	struct wl_resource *resource;

    fprintf(stderr, "seat.c: bind_seat()\n");
	if (version > 4)
		version = 4;

	resource = wl_resource_create(client, &wl_seat_interface, version, id);
	wl_resource_set_implementation(resource, &seat_implementation, NULL, &remove_resource);
	//wl_list_insert(&seat.resources, wl_resource_get_link(resource));

	if (version >= 2)
		wl_seat_send_name(resource, seat.name);

	//wl_seat_send_capabilities(resource, seat.capabilities);
}

bool seat_initialize(wayvroom_server_t* server, const char *seat_name) {
    fprintf(stderr, "seat.c: seat_initialize()\n");

	if (!(seat.name = strdup(seat_name))) {
		return false;
	}

	seat.global = wl_global_create(server->wl_display, &wl_seat_interface, 4, NULL, &bind_seat);

	if (!seat.global)
		return false;

    return true;
}
