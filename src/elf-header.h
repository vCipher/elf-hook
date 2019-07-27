#pragma once

#include <stddef.h>
#include <elf.h>

#include "externc.h"
#include "elf-defs.h"

EXTERNC int elf_header_read(int descriptor, Elf_Ehdr **header);
EXTERNC void elf_header_destroy(Elf_Ehdr *header);