#pragma once

#include "externc.h"

typedef struct dl_info
{
    const char *file_name;
    void *base_address;
} dl_info_t;

EXTERNC int dl_get_info_by_address(void *address, dl_info_t *info);
EXTERNC int dl_get_info_by_handle(void *handle, dl_info_t *info);