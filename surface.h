struct surface {
    struct wl_resource* resource;
    struct wl_resource* buffer_resource;
    uint32_t vertex_id;
    uint32_t normal_id;
    uint32_t index_id;
    uint32_t geometry_id;
    uint32_t mesh_id;
};

struct surface* surface_new(struct wl_client *client, uint32_t version, uint32_t id);
