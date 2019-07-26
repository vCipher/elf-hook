#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "dl-addr.h"
#include "elf-defs.h"
#include "elf-reader.h"

int get_module_base_address(void *address, char const **module_filename, void **module_base_address)
{
    Dl_info info;
    void *extra;
    if (dladdr1(address, &info, &extra, RTLD_DL_LINKMAP) == 0)
    {
        fprintf(stderr, "Failed to get Dl_info by function %p", address);
        return ELIBACC;
    }

    struct link_map *map = (struct link_map*)extra;
    *module_filename = info.dli_fname;
    *module_base_address = (void*)map->l_addr;

    return 0;
}

int get_module_base_address_dl(void *handle, char const *module_filename, void **module_base_address)
{
    int descriptor;  //file descriptor of shared module
    Elf_Shdr *dynsym = NULL, *strings_section = NULL;
    char const *strings = NULL;
    Elf_Sym *symbols = NULL;
    size_t i, amount;
    Elf_Sym *found = NULL;

    *module_base_address = NULL;

    descriptor = open(module_filename, O_RDONLY);

    if (descriptor < 0)
        return errno;

    if (elf_section_by_type(descriptor, SHT_DYNSYM, &dynsym) ||  //get ".dynsym" section
        elf_section_by_index(descriptor, dynsym->sh_link, &strings_section) ||
        elf_read_string_table(descriptor, strings_section, &strings) ||
        elf_read_symbol_table(descriptor, dynsym, &symbols))
    {
        free(strings_section);
        free((void *)strings);
        free(symbols);
        free(dynsym);
        close(descriptor);

        return errno;
    }

    amount = dynsym->sh_size / sizeof(Elf_Sym);

    /* Trick to get the module base address in a portable way:
     *   Find the first GLOBAL or WEAK symbol in the symbol table,
     *   look this up with dlsym, then return the difference as the base address
     */
    for (i = 0; i < amount; ++i)
    {
        switch(ELF32_ST_BIND(symbols[i].st_info)) {
        case STB_GLOBAL:
        case STB_WEAK:
            found = &symbols[i];
            break;
        default: // Not interested in this symbol
            break;
        }
    }

    if(found != NULL)
    {
        const char *name = &strings[found->st_name];
        void *sym = dlsym(handle, name); 
        if(sym != NULL)
            *module_base_address = (void*)((size_t)sym - found->st_value);
    }

    free(strings_section);
    free((void *)strings);
    free(symbols);
    free(dynsym);
    close(descriptor);

    return *module_base_address != NULL;
}