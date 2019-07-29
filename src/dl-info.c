#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "errors.h"
#include "dl-info.h"

int dl_get_info_by_address(void *address, dl_info_t *info)
{
    Dl_info dl_info;
    void *extra;
    if (dladdr1(address, &dl_info, &extra, RTLD_DL_LINKMAP) == 0)
    {
        fprintf(stderr, "Failed to get dynamic library info by function %p", address);
        return ELIBACC;
    }

    struct link_map *map = (struct link_map*)extra;
    info->file_name = map->l_name;
    info->base_address = (void*)map->l_addr;

    return SUCCESS;
}

int dl_get_info_by_handle(void *handle, dl_info_t *info)
{
    void *extra;
    if (dlinfo(handle, RTLD_DL_LINKMAP, &extra) != 0)
    {
        fprintf(stderr, "Failed to get dynamic library info by handle %p, error: %s", handle, dlerror());
        return ELIBACC;
    }

    struct link_map *map = (struct link_map*)extra;
    info->file_name = map->l_name;
    info->base_address = (void*)map->l_addr;

    return SUCCESS;
}