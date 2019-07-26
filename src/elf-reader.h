#pragma once

#include <stddef.h>
#include <elf.h>

#include "externc.h"
#include "elf-defs.h"

EXTERNC int elf_read_header(int d, Elf_Ehdr **header);
EXTERNC int elf_read_section_table(int descriptor, const Elf_Ehdr *header, Elf_Shdr **table);
EXTERNC int elf_read_string_table(int descriptor, const Elf_Shdr *section, const char **strings);
EXTERNC int elf_read_symbol_table(int descriptor, const Elf_Shdr *section, Elf_Sym **table);
EXTERNC int elf_read_relocation_table(int descriptor, const Elf_Shdr *section, Elf_Rel **table);
EXTERNC int elf_section_by_index(int descriptor, const size_t index, Elf_Shdr **section);
EXTERNC int elf_section_by_type(int descriptor, const size_t section_type, Elf_Shdr **section);
EXTERNC int elf_section_by_name(int descriptor, const char *section_name, Elf_Shdr **section);
EXTERNC int elf_symbol_index_by_name(int descriptor, Elf_Shdr *section, char const *name, size_t *index);
EXTERNC int elf_symbol_index_by_address(int descriptor, Elf_Shdr *section, Elf_Addr address, size_t *index);