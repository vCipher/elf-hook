#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "errors.h"
#include "elf-header.h"
#include "elf-section.h"
#include "elf-string.h"
#include "elf-symbol.h"


static int elf_symbol_alloc_table(size_t size, Elf_Sym **table)
{
    *table = (Elf_Sym *)malloc(size);
    if(*table == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for ELF symbol table, error: %s\n", strerror(errno));
        return errno;
    }

    return SUCCESS;
}

// static int elf_symbol_alloc(Elf_Sym **table)
// {
//     *table = (Elf_Sym *)malloc(sizeof(Elf_Sym));
//     if(*table == NULL)
//     {
//         fprintf(stderr, "Failed to allocate memory for ELF symbol, error: %s\n", strerror(errno));
//         return errno;
//     }

//     return SUCCESS;
// }

void elf_symbol_destroy(Elf_Sym *symbol)
{
    if (symbol == NULL)
    {
        fprintf(stdout, "ELF symbol has been already destroyed\n");
        return;
    }

    free(symbol);
}

int elf_symbol_read_table(int descriptor, const Elf_Shdr *section, Elf_Sym **table)
{
    if (section == NULL)
        return EINVAL;

    Elf_Sym *local = NULL;

    TRY
    {
        CHECK_RESULT(elf_symbol_alloc_table(section->sh_size, &local));
        
        if (lseek(descriptor, section->sh_offset, SEEK_SET) < 0)
            THROW(errno);

        if (read(descriptor, local, section->sh_size) <= 0)
            THROW(errno);
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to read ELF symbol table, error: %s\n", strerror(error));
        elf_symbol_destroy(local);

        return error;
    }

    *table = local;
    return SUCCESS;
}

int elf_symbol_find_index_by_name(int descriptor, Elf_Shdr *section, char const *name, size_t *index)
{
    if (section == NULL)
        return EINVAL;

    Elf_Shdr *strings_section = NULL;
    char const *strings = NULL;
    Elf_Sym *symbols = NULL;
    int result = ENODATA;

    TRY
    {
        CHECK_RESULT(elf_section_find_by_index(descriptor, section->sh_link, &strings_section));
        CHECK_RESULT(elf_string_read_table(descriptor, strings_section, &strings));
        CHECK_RESULT(elf_symbol_read_table(descriptor, section, &symbols));

        size_t amount = section->sh_size / sizeof(Elf_Sym);

        for (size_t i = 0; i < amount; ++i)
        {
            if (!strcmp(name, &strings[symbols[i].st_name]))
            {
                *index = i;
                result = SUCCESS;

                break;
            }
        }
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to find ELF symbol by name %s, error: %s\n", name, strerror(error));
        result = error;
    }

    elf_section_destroy(strings_section);
    elf_string_destroy(strings);
    elf_symbol_destroy(symbols);

    return result;
}

int elf_symbol_find_index_by_address(int descriptor, Elf_Shdr *section, Elf_Addr address, size_t *index)
{
    if (section == NULL)
        return EINVAL;

    Elf_Sym *symbols = NULL;
    int result = ENODATA;

    TRY
    {
        CHECK_RESULT(elf_symbol_read_table(descriptor, section, &symbols));

        size_t amount = section->sh_size / sizeof(Elf_Sym);

        for (size_t i = 0; i < amount; ++i)
        {
            if (address == symbols[i].st_value)
            {
                *index = i;
                result = SUCCESS;
                
                break;
            }
        }
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to find ELF symbol by address %p, error: %s\n", (void*)address, strerror(error));
        result = error;
    }

    elf_symbol_destroy(symbols);

    return result;
}
