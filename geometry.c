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
#include "esm.h"

typedef struct memory_layout_item {
    uint32_t id;
    uint8_t* mem;
    uint32_t memory_offset;
    uint32_t memory_size;
    uint32_t item_length;
    uint32_t data_length;
    vrms_data_type_t type;
} memory_layout_item_t;

typedef struct memory_layout {
    uint32_t id;
    int32_t fd;
    uint8_t* mem;
    size_t total_size;
    uint32_t nr_items;
    memory_layout_item_t* items;
} memory_layout_t;

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

void geometry_realise_memory(vrms_runtime_t* vrms_runtime, uint32_t scene_id, memory_layout_t* layout) {
    uint32_t index;
    memory_layout_item_t* item;

    for (index = 0; index < layout->nr_items; index++) {
        item = &layout->items[index];
        layout->total_size += item->memory_size;
    }

    layout->fd = memfd_create("WAYVROOM surface", MFD_ALLOW_SEALING);
    if (layout->fd <= 0) {
        fprintf(stderr, "unable to create shared memory: %d\n", errno);
        return;
    }

    int32_t ret = ftruncate(layout->fd, layout->total_size);
    if (-1 == ret) {
        fprintf(stderr, "unable to truncate memfd to size %zd\n", layout->total_size);
        return;
    }

    ret = fcntl(layout->fd, F_ADD_SEALS, F_SEAL_SHRINK);
    if (-1 == ret) {
        fprintf(stderr, "failed to add seals to memfd\n");
        return;
    }

    layout->mem = mmap(NULL, layout->total_size, PROT_READ|PROT_WRITE, MAP_SHARED, layout->fd, 0);
    if (MAP_FAILED == layout->mem) {
        fprintf(stderr, "unable to attach address\n");
        return;
    }

    layout->id = vrms_runtime->interface->create_memory(vrms_runtime, scene_id, layout->fd, layout->total_size);
}

void geometry_realise_memory_item(vrms_runtime_t* vrms_runtime, uint32_t scene_id, memory_layout_t* layout, memory_layout_item_t* item) {
    fprintf(stderr, "realising from offset: %d\n", item->memory_offset);
    item->id = vrms_runtime->interface->create_object_data(vrms_runtime, scene_id, layout->id, item->memory_offset, item->memory_size, item->item_length, item->data_length, item->type);
    item->mem = &layout->mem[item->memory_offset];
}

geometry_object_t* geometry_create_screen(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t texture_id, uint16_t width, uint16_t height) {
    float x_min;
    float y_min;
    float x_max;
    float y_max;
    uint32_t size_of_verts;
    uint32_t size_of_indicies;
    uint32_t size_of_uvs;
    uint32_t offset;
    geometry_object_t* object;
    memory_layout_t* layout;
    memory_layout_item_t *item;
    float* verts;
    float* norms;
    uint16_t* indicies;
    float* uvs;
    float* matrix;
    uint8_t* program;
    uint32_t* registers;

    if (width > height) {
        x_max = 1.0f;
        y_max = (float)height / (float)width;
        x_min = -1.0f;
        y_min = 0.0f - y_max;
    }
    else {
        x_max = (float)width / (float)height;
        y_max = 1.0f;
        x_min = 0.0f - x_max;
        y_min = -1.0f;
    }

    size_of_verts = (sizeof(float) * 3) * 4;
    size_of_indicies = (sizeof(uint16_t)) * 6;
    size_of_uvs = (sizeof(float) * 2) * 4;

    layout = malloc(sizeof(memory_layout_t));
    memset(layout, 0, sizeof(memory_layout_t));

    layout->items = malloc(sizeof(memory_layout_item_t) * 7);
    memset(layout->items, 0, sizeof(memory_layout_item_t) * 7);

    layout->nr_items = 7;

    offset = 0;
    item = &layout->items[0];
    item->memory_offset = offset;
    item->memory_size = size_of_verts;
    item->item_length = sizeof(float) * 3;
    item->data_length = sizeof(float);
    item->type = VRMS_VERTEX;

    offset += size_of_verts;
    item = &layout->items[1];
    item->memory_offset = offset;
    item->memory_size = size_of_verts;
    item->item_length = sizeof(float) * 3;
    item->data_length = sizeof(float);
    item->type = VRMS_NORMAL;

    offset += size_of_verts;
    item = &layout->items[2];
    item->memory_offset = offset;
    item->memory_size = size_of_indicies;
    item->item_length = sizeof(uint16_t);
    item->data_length = sizeof(uint16_t);
    item->type = VRMS_INDEX;

    offset += size_of_indicies;
    item = &layout->items[3];
    item->memory_offset = offset;
    item->memory_size = size_of_uvs;
    item->item_length = sizeof(float) * 2;
    item->data_length = sizeof(float);
    item->type = VRMS_UV;

    offset += size_of_uvs;
    item = &layout->items[4];
    item->memory_offset = offset;
    item->memory_size = sizeof(uint32_t) * 8;
    item->item_length = sizeof(uint32_t);
    item->data_length = sizeof(uint32_t);
    item->type = VRMS_REGISTER;

    offset += sizeof(uint32_t) * 8;
    item = &layout->items[5];
    item->memory_offset = offset;
    item->memory_size = sizeof(uint8_t) * 10;
    item->item_length = sizeof(uint8_t);
    item->data_length = sizeof(uint8_t);
    item->type = VRMS_PROGRAM;

    offset += sizeof(uint8_t) * 10;
    item = &layout->items[6];
    item->memory_offset = offset;
    item->memory_size = sizeof(float) * 16;
    item->item_length = sizeof(float) * 16;
    item->data_length = sizeof(float);
    item->type = VRMS_MATRIX;

    geometry_realise_memory(vrms_runtime, scene_id, layout);
    if (layout->id == 0) {
        fprintf(stderr, "unable to realise memory\n");
        return NULL;
    }

    verts = ((float*)&layout->mem[layout->items[0].memory_offset]);
    norms = ((float*)&layout->mem[layout->items[1].memory_offset]);
    indicies = ((uint16_t*)&layout->mem[layout->items[2].memory_offset]);
    uvs = ((float*)&layout->mem[layout->items[3].memory_offset]);
    registers = ((uint32_t*)&layout->mem[layout->items[4].memory_offset]);
    program = ((uint8_t*)&layout->mem[layout->items[5].memory_offset]);
    matrix = ((float*)&layout->mem[layout->items[6].memory_offset]);

    fprintf(stderr, "generating plane: (%f,%f) (%f,%f)\n", x_min, y_min, x_max, y_max);
    std_plane_generate_verticies(verts, x_min, y_min, x_max, y_max);
    std_plane_generate_normals(norms);
    std_plane_generate_indicies(indicies);
    std_plane_generate_uvs(uvs);
    geometry_realise_memory_item(vrms_runtime, scene_id, layout, &layout->items[0]);
    geometry_realise_memory_item(vrms_runtime, scene_id, layout, &layout->items[1]);
    geometry_realise_memory_item(vrms_runtime, scene_id, layout, &layout->items[2]);
    geometry_realise_memory_item(vrms_runtime, scene_id, layout, &layout->items[3]);

    program[0] = VM_MATLM;
    program[1] = VM_REG0;
    program[2] = VM_REG1;
    program[3] = VM_REG2;
    program[4] = VM_DRAW;
    program[5] = VM_REG0;
    program[6] = VM_REG0;
    program[7] = VM_FRWAIT;
    program[8] = VM_JMP;
    program[9] = 0x04;
    geometry_realise_memory_item(vrms_runtime, scene_id, layout, &layout->items[5]);

    esmLoadIdentity(matrix);
    esmTranslatef(matrix, 0.0f, 0.0f, -4.0f);
    geometry_realise_memory_item(vrms_runtime, scene_id, layout, &layout->items[6]);

    object = malloc(sizeof(geometry_object_t));
    memset(object, 0, sizeof(geometry_object_t));

    object->geometry_id = vrms_runtime->interface->create_object_geometry(vrms_runtime, scene_id, layout->items[0].id, layout->items[1].id, layout->items[2].id);
    object->mesh_id = vrms_runtime->interface->create_object_mesh_texture(vrms_runtime, scene_id, object->geometry_id, texture_id, layout->items[3].id);

    registers[0] = object->mesh_id;
    registers[1] = layout->items[6].id;
    registers[2] = 0;
    registers[3] = 0;
    registers[4] = 0;
    registers[5] = 0;
    registers[6] = 0;
    registers[7] = 0;
    geometry_realise_memory_item(vrms_runtime, scene_id, layout, &layout->items[4]);

    object->program_id = vrms_runtime->interface->create_program(vrms_runtime, scene_id, layout->items[5].id);
    vrms_runtime->interface->run_program(vrms_runtime, scene_id, object->program_id, layout->items[4].id);

    return object;
}

