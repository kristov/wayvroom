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

static struct {
    struct wl_global *global;
} shm;

struct pool {
    struct wl_resource *resource;
    void *data;
    uint32_t size;
    unsigned references;
};

struct pool_reference {
    struct pool *pool;
};

static void shm_create_buffer(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format) {
    struct pool* pool = wl_resource_get_user_data(resource);
    struct pool_reference* reference;
    struct wl_resource* buffer_resource;

    fprintf(stderr, "shm.c: shm_create_buffer(id:%d)\n", id);
    if (offset > pool->size || offset < 0) {
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_STRIDE, "offset is too big or negative");
        return;
    }

    fprintf(stderr, "shm.c: shm_create_buffer(details):\n");
    fprintf(stderr, "shm.c: pool->size: %d\n", pool->size);
    fprintf(stderr, "shm.c:     offset: %d\n", offset);
    fprintf(stderr, "shm.c:      width: %d\n", width);
    fprintf(stderr, "shm.c:     height: %d\n", height);
    fprintf(stderr, "shm.c:     stride: %d\n", stride);
    fprintf(stderr, "shm.c:     format: %d\n", format);
    //object.ptr = (void*)((uintptr_t)pool->data + offset);
    //buffer = wld_import_buffer(swc.shm->context, WLD_OBJECT_DATA, object, width, height, format_shm_to_wld(format), stride);

    buffer_resource = wayland_buffer_create_resource(client, wl_resource_get_version(resource), id);

    if (!buffer_resource) {
        wl_resource_post_no_memory(resource);
        return;
    }

    if (!(reference = malloc(sizeof(*reference)))) {
        wl_resource_destroy(buffer_resource);
        wl_resource_post_no_memory(resource);
        return;
    }

    reference->pool = pool;
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

static void shm_unref_pool(struct pool *pool) {
    fprintf(stderr, "shm.c: shm_unref_pool()\n");
    if (--pool->references > 0)
        return;

    munmap(pool->data, pool->size);
    free(pool);
}

static void shm_destroy_pool_resource(struct wl_resource *resource) {
    struct pool *pool = wl_resource_get_user_data(resource);
    fprintf(stderr, "shm.c: shm_destroy_pool_resource()\n");
    shm_unref_pool(pool);
}

static void shm_create_pool(struct wl_client *client, struct wl_resource *resource, uint32_t id, int32_t fd, int32_t size) {
    struct pool *pool;

    fprintf(stderr, "shm.c: shm_create_pool()\n");
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

    wl_resource_set_implementation(pool->resource, &shm_pool_implementation, pool, &shm_destroy_pool_resource);
    pool->data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (pool->data == MAP_FAILED) {
        fprintf(stderr, "shm.c: shm_create_pool() [mmap failed]\n");
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FD, "mmap failed: %s", strerror(errno));
        wl_resource_destroy(pool->resource);
        free(pool);
        close(fd);
        return;
    }

    close(fd);
    pool->size = size;
    pool->references = 1;
    return;
}

static struct wl_shm_interface shm_implementation = {
    .create_pool = &shm_create_pool
};

static void shm_bind_shm(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
    struct wl_resource *resource;

    fprintf(stderr, "shm.c: shm_bind_shm()\n");
    if (version > 1)
        version = 1;

    resource = wl_resource_create(client, &wl_shm_interface, version, id);
    wl_resource_set_implementation(resource, &shm_implementation, NULL, NULL);

    wl_shm_send_format(resource, WL_SHM_FORMAT_XRGB8888);
    wl_shm_send_format(resource, WL_SHM_FORMAT_ARGB8888);
}

bool shm_initialize(wayvroom_server_t* server) {
    fprintf(stderr, "shm.c: shm_initialize()\n");
    shm.global = wl_global_create(server->wl_display, &wl_shm_interface, 1, NULL, &shm_bind_shm);

    if (!shm.global)
        return false;

    return true;
}

