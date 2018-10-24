#include <stdio.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdbool.h>

#include "wayvroom.h"
#include "wayland_buffer.h"

#include "shm.h"

#include "vrms_runtime.h"

static struct {
    struct wl_global *global;
} shm;

struct pool {
    wayvroom_server_t* server;
    struct wl_resource *resource;
    void* data;
    uint32_t size;
    unsigned references;
    uint32_t memory_id;
};

static void shm_create_buffer(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t wl_format) {
    struct pool* pool = wl_resource_get_user_data(resource);
    buffer_reference_t* reference;
    struct wl_resource* buffer_resource;
    wayvroom_server_t* server;
    vrms_runtime_t* vrms_runtime;
    uint32_t data_id;
    uint32_t texture_id;
    uint32_t memory_length;
    uint32_t item_length;
    uint32_t data_length;
    vrms_texture_format_t format;

    fprintf(stderr, "shm.c: shm_create_buffer(id:%d)\n", id);
    if (offset > pool->size || offset < 0) {
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_STRIDE, "offset is too big or negative");
        return;
    }

    server = pool->server;
    vrms_runtime = server->vrms_runtime;

    fprintf(stderr, "shm.c: shm_create_buffer(details):\n");
    fprintf(stderr, "shm.c: pool->size: %d\n", pool->size);
    fprintf(stderr, "shm.c:     offset: %d\n", offset);
    fprintf(stderr, "shm.c:      width: %d\n", width);
    fprintf(stderr, "shm.c:     height: %d\n", height);
    fprintf(stderr, "shm.c:     stride: %d\n", stride);
    fprintf(stderr, "shm.c:  wl_format: %d\n", wl_format);

    memory_length = height * stride;
    item_length = stride / width;
    data_length = item_length / 4;

    switch (wl_format) {
        case WL_SHM_FORMAT_XRGB8888:
            format = VRMS_RGB8;
            break;
        case WL_SHM_FORMAT_ARGB8888:
            format = VRMS_RGB8;
            break;
        default:
            wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FORMAT, "only WL_SHM_FORMAT_XRGB8888 supported");
            return;
            break;
    }

    buffer_resource = wayland_buffer_create_resource(client, wl_resource_get_version(resource), id);
    if (!buffer_resource) {
        wl_resource_post_no_memory(resource);
        return;
    }

    data_id = vrms_runtime->interface->create_object_data(vrms_runtime, server->scene_id, pool->memory_id, offset, memory_length, item_length, data_length, VRMS_TEXTURE);
    texture_id = vrms_runtime->interface->create_object_texture(vrms_runtime, server->scene_id, data_id, width, height, format, VRMS_TEXTURE_2D);

    if (!(reference = malloc(sizeof(*reference)))) {
        wl_resource_destroy(buffer_resource);
        wl_resource_post_no_memory(resource);
        return;
    }

    reference->server = server;
    reference->data_id = data_id;
    reference->texture_id = texture_id;
    reference->pool = pool;
    reference->width = width;
    reference->height = height;

    wl_resource_set_user_data(buffer_resource, reference);
    //reference->destructor.destroy = &handle_buffer_destroy;
    //wld_buffer_add_destructor(buffer, &reference->destructor);
    ++pool->references;

    return;
}

static void shm_destroy(struct wl_client *client, struct wl_resource *resource) {
    fprintf(stderr, "shm.c: shm_destroy()\n");
    wl_resource_destroy(resource);
}

static void shm_resize(struct wl_client *client, struct wl_resource *resource, int32_t size) {
    struct pool *pool = wl_resource_get_user_data(resource);
    void *data;

    fprintf(stderr, "shm.c: shm_resize()\n");
    if (!(data = malloc(size))) {
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FD, "mremap failed: %s", strerror(errno));
        return;
    }
    memset(data, 0, size);
    memcpy(data, pool->data, pool->size);
    free(pool->data);

    pool->data = data;
    pool->size = size;
}

static struct wl_shm_pool_interface shm_pool_implementation = {
    .create_buffer = shm_create_buffer,
    .destroy = shm_destroy,
    .resize = shm_resize,
};

static void shm_pool_unref(struct pool *pool) {
    fprintf(stderr, "shm.c: shm_pool_unref()\n");
    if (--pool->references > 0)
        return;

    // vrms_runtime_destroy_memory(...);
    free(pool);
}

static void shm_pool_resource_destroy(struct wl_resource *resource) {
    struct pool *pool = wl_resource_get_user_data(resource);
    fprintf(stderr, "shm.c: shm_pool_resource_destroy()\n");
    shm_pool_unref(pool);
}

static void shm_pool_create(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t fd, int32_t size) {
    struct pool* pool;
    uint32_t memory_id;
    wayvroom_server_t* server;
    vrms_runtime_t* vrms_runtime;

    fprintf(stderr, "shm.c: shm_pool_create()\n");
    if (!(pool = malloc(sizeof(*pool)))) {
        wl_resource_post_no_memory(resource);
        close(fd);
        return;
    }

    pool->resource = wl_resource_create(client, &wl_shm_pool_interface, wl_resource_get_version(resource), id);
    if (!pool->resource) {
        wl_resource_post_no_memory(resource);
        free(pool);
        close(fd);
        return;
    }

    server = wl_resource_get_user_data(resource);
    pool->server = server;
    vrms_runtime = server->vrms_runtime;

    wl_resource_set_implementation(pool->resource, &shm_pool_implementation, pool, &shm_pool_resource_destroy);

    memory_id = vrms_runtime->interface->create_memory(server->vrms_runtime, server->scene_id, fd, size);

    if (memory_id == 0) {
        fprintf(stderr, "shm.c: shm_create_pool() [vrms_runtime_create_memory failed]\n");
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FD, "vrms_runtime_create_memory failed: %s", strerror(errno));
        wl_resource_destroy(pool->resource);
        free(pool);
        close(fd);
        return;
    }

    close(fd);
    pool->size = size;
    pool->references = 1;
    pool->memory_id = memory_id;
    return;
}

static struct wl_shm_interface shm_implementation = {
    .create_pool = &shm_pool_create
};

static void shm_bind_shm(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
    struct wl_resource *resource;
    wayvroom_server_t* server;

    fprintf(stderr, "shm.c: shm_bind_shm()\n");
    if (version > 1)
        version = 1;

    server = (wayvroom_server_t*)data;

    resource = wl_resource_create(client, &wl_shm_interface, version, id);
    wl_resource_set_implementation(resource, &shm_implementation, server, NULL);

    wl_shm_send_format(resource, WL_SHM_FORMAT_XRGB8888);
    wl_shm_send_format(resource, WL_SHM_FORMAT_ARGB8888);
}

bool shm_initialize(wayvroom_server_t* server) {
    fprintf(stderr, "shm.c: shm_initialize()\n");
    shm.global = wl_global_create(server->wl_display, &wl_shm_interface, 1, server, &shm_bind_shm);

    if (!shm.global)
        return false;

    return true;
}

