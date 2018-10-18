#ifndef SHM_H
#define SHM_H

#include "wayvroom.h"

typedef struct buffer_reference {
    wayvroom_server_t* server;
    struct pool* pool;
    uint32_t data_id;
    uint32_t texture_id;
    uint32_t width;
    uint32_t height;
} buffer_reference_t;

bool shm_initialize(wayvroom_server_t* server);

#endif
