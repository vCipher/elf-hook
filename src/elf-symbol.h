#pragma once

#include <stddef.h>
#include <elf.h>

#include "externc.h"
#include "elf-defs.h"

EXTERNC int elf_symbol_read_table(int descriptor, const Elf_Shdr *section, Elf_Sym **table);
EXTERNC int elf_symbol_find_index_by_name(int descriptor, Elf_Shdr *section, char const *name, size_t *index);
EXTERNC int elf_symbol_find_index_by_address(int descriptor, Elf_Shdr *section, Elf_Addr address, size_t *index);
EXTERNC void elf_symbol_destroy(Elf_Sym *symbol);