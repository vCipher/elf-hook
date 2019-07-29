#pragma once

#include "externc.h"
#include "dl-info.h"

#define ELF_HOOK(dl_info, name, substitution) \
    elf_hook(dl_info, name, (void*)substitution)

EXTERNC void *elf_hook(dl_info_t *dl_info, const char *function_name, void *substitution_address);