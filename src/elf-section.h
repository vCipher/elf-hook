#pragma once

#include <stddef.h>
#include <elf.h>

#include "externc.h"
#include "elf-defs.h"

EXTERNC int elf_section_read_table(int descriptor, const Elf_Ehdr *header, Elf_Shdr **table);
EXTERNC int elf_section_find_by_index(int descriptor, const size_t index, Elf_Shdr **section);
EXTERNC int elf_section_find_by_type(int descriptor, const size_t section_type, Elf_Shdr **section);
EXTERNC int elf_section_find_by_name(int descriptor, const char *section_name, Elf_Shdr **section);
EXTERNC void elf_section_destroy(Elf_Shdr *section);