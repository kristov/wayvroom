#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <syscall.h>
#include <sys/un.h>
#include <linux/memfd.h>
#include "memfd.h"
#include <string.h>

#include "geometry.h"

void std_plane_generate_verticies(float* verts, float x_min, float y_min, float x_max, float y_max) {
    uint32_t off = 0;

    verts[off + 0] = x_min; // 0
    verts[off + 1] = y_min;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x_max; // 1
    verts[off + 1] = y_min;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x_min; // 2
    verts[off + 1] = y_max;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x_max; // 3
    verts[off + 1] = y_max;
    verts[off + 2] = 0.0f;
}

void std_plane_generate_normals(float* norms) {
    uint32_t off = 0;

    norms[off + 0] = 0.0f; // 0
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 1
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 2
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 3
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
}

void std_plane_generate_indicies(uint16_t* indicies) {
    uint32_t off = 0;

    indicies[off + 0] = 0;
    indicies[off + 1] = 1;
    indicies[off + 2] = 2;
    indicies[off + 3] = 1;
    indicies[off + 4] = 2;
    indicies[off + 5] = 3;
    off += 6;
}

void std_plane_generate_uvs(float* uvs) {
    uint32_t off = 0;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;
}

typedef struct geometry_layout {
    uint32_t vertex_offset;
    uint32_t vertex_size;
    uint32_t normal_offset;
    uint32_t normal_size;
    uint32_t index_offset;
    uint32_t index_size;
    uint32_t uv_offset;
    uint32_t uv_size;
    size_t size;
} geometry_layout_t;

geometry_layout_t geometry_plane_calculate_layout() {
    uint32_t size_of_verts;
    uint32_t size_of_indicies;
    uint32_t size_of_uvs;
    geometry_layout_t layout;

    size_of_verts = (sizeof(float) * 3) * 4;
    size_of_indicies = (sizeof(uint16_t)) * 6;
    size_of_uvs = (sizeof(float) * 2) * 4;

    layout.vertex_offset = 0;
    layout.vertex_size = size_of_verts;
    layout.normal_offset = size_of_verts;
    layout.normal_size = size_of_verts;
    layout.index_offset = size_of_verts + size_of_verts;
    layout.index_size = size_of_indicies;
    layout.uv_offset = size_of_verts + size_of_verts + size_of_indicies;
    layout.uv_size = size_of_uvs;
    layout.size = size_of_verts + size_of_verts + size_of_indicies + size_of_uvs;

    return layout;
}

void geometry_plane_generate(geometry_layout_t* layout, uint16_t width, uint16_t height, uint8_t* dest) {
    float x_min;
    float y_min;
    float x_max;
    float y_max;

    if (width > height) {
        x_max = 1.0f;
        y_max = (float)height / (float)width;
        x_min = -1.0f;
        y_min = 0.0f - y_max;
    }
    else {
        x_min = (float)width / (float)height;
        y_max = 1.0f;
        x_min = 0.0f - x_min;
        y_min = -1.0f;
    }

    std_plane_generate_verticies((float*)&dest[layout->index_offset], x_min, y_min, x_max, y_max);
    std_plane_generate_normals((float*)&dest[layout->normal_offset]);
    std_plane_generate_indicies((uint16_t*)&dest[layout->index_offset]);
    std_plane_generate_uvs((float*)&dest[layout->uv_offset]);
}

geometry_object_t* geometry_plane_create(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint16_t width, uint16_t height) {
    int32_t fd = -1;
    int32_t ret;
    uint8_t* mem;
    geometry_layout_t layout;
    geometry_object_t* object;

    fd = memfd_create("WAYVROOM surface", MFD_ALLOW_SEALING);
    if (fd <= 0) {
        fprintf(stderr, "unable to create shared memory: %d\n", errno);
        return NULL;
    }

    layout = geometry_plane_calculate_layout();

    ret = ftruncate(fd, layout.size);
    if (-1 == ret) {
        fprintf(stderr, "unable to truncate memfd to size %zd\n", layout.size);
        return NULL;
    }

    ret = fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK);
    if (-1 == ret) {
        fprintf(stderr, "failed to add seals to memfd\n");
        return NULL;
    }

    mem = mmap(NULL, layout.size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (MAP_FAILED == mem) {
        fprintf(stderr, "unable to attach address\n");
        return NULL;
    }

    geometry_plane_generate(&layout, width, height, mem);

    object = malloc(sizeof(geometry_object_t));
    memset(object, 0, sizeof(geometry_object_t));

    object->memory_id = vrms_runtime->interface->create_memory(vrms_runtime, scene_id, fd, layout.size);
    object->vertex_id = vrms_runtime->interface->create_object_data(vrms_runtime, scene_id, object->memory_id, layout.vertex_offset, layout.vertex_size, sizeof(float) * 3, sizeof(float), VRMS_VERTEX);
    object->normal_id = vrms_runtime->interface->create_object_data(vrms_runtime, scene_id, object->memory_id, layout.normal_offset, layout.normal_size, sizeof(float) * 3, sizeof(float), VRMS_NORMAL);
    object->index_id = vrms_runtime->interface->create_object_data(vrms_runtime, scene_id, object->memory_id, layout.index_offset, layout.index_size, sizeof(uint16_t), sizeof(uint16_t), VRMS_INDEX);
    object->uv_id = vrms_runtime->interface->create_object_data(vrms_runtime, scene_id, object->memory_id, layout.uv_offset, layout.uv_size, sizeof(float) * 2, sizeof(float), VRMS_UV);
    object->geometry_id = vrms_runtime->interface->create_object_geometry(vrms_runtime, scene_id, object->vertex_id, object->normal_id, object->index_id);

    object->fd = fd;
    object->mem = mem;

    return object;
}
