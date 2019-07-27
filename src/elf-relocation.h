#pragma once

#include <stddef.h>
#include <elf.h>

#include "externc.h"
#include "elf-defs.h"

EXTERNC int elf_relocation_read_table(int descriptor, const Elf_Shdr *section, Elf_Rel **table);
EXTERNC void elf_relocation_destroy(Elf_Rel *relocation);
