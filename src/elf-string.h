#pragma once

#include <stddef.h>
#include <elf.h>

#include "externc.h"
#include "elf-defs.h"

EXTERNC int elf_string_read_table(int descriptor, const Elf_Shdr *section, const char **strings);
EXTERNC void elf_string_destroy(const char *string);