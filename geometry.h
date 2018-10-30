#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "vrms_runtime.h"
#include "vrms_render_vm.h"

typedef struct geometry_object {
    int32_t fd;
    uint8_t* mem;
    uint32_t memory_id;
    uint32_t vertex_id;
    uint32_t normal_id;
    uint32_t index_id;
    uint32_t uv_id;
    uint32_t geometry_id;
    uint32_t matrix_id;
    uint32_t program_memory_id;
    uint32_t program_id;
    uint32_t register_id;
    uint32_t mesh_id;
} geometry_object_t;

geometry_object_t* geometry_create_screen(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t texture_id, uint16_t width, uint16_t height);
void geometry_destroy_screen(vrms_runtime_t* vrms_runtime, uint32_t scene_id, geometry_object_t* geometry);
#endif
