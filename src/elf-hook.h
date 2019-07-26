#pragma once

#include "externc.h"

#define ELF_HOOK(function, substitution) \
    elf_hook((void*)function, (void*)substitution)

#define ELF_RESTORE(function) \
    elf_hook((void*)function, (void*)function)

#define ELF_HOOK_DL(lib_name, lib_addr, function, substitution) \
    elf_hook_dl(lib_name, lib_addr, #function, (void*)substitution)

#define ELF_RESTORE_DL(lib_name, lib_addr, function) \
    elf_hook_dl(lib_name, lib_addr, #function, (void*)function)

EXTERNC void *elf_hook(void *function_address, void *substitution_address);
EXTERNC void *elf_hook_dl(const char *library_filename, void *library_address, const char *function_name, void *substitution_address);