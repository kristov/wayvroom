struct surface {
    struct wl_resource* resource;
    struct wl_resource* buffer_resource;
};

struct surface* surface_new(struct wl_client *client, uint32_t version, uint32_t id);
