#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "elf-reader.h"

int elf_read_header(int descriptor, Elf_Ehdr **header)
{
    *header = (Elf_Ehdr *)malloc(sizeof(Elf_Ehdr));
    if(NULL == *header)
    {
        return errno;
    }

    if (lseek(descriptor, 0, SEEK_SET) < 0)
    {
        free(*header);
        return errno;
    }

    if (read(descriptor, *header, sizeof(Elf_Ehdr)) <= 0)
    {
        free(*header);
        return errno = EINVAL;
    }

    return 0;
}

int elf_read_section_table(int descriptor, Elf_Ehdr const *header, Elf_Shdr **table)
{
    size_t size;

    if (NULL == header)
        return EINVAL;

    size = header->e_shnum * sizeof(Elf_Shdr);
    *table = (Elf_Shdr *)malloc(size);
    if(NULL == *table)
    {
        return errno;
    }

    if (lseek(descriptor, header->e_shoff, SEEK_SET) < 0)
    {
        free(*table);
        return errno;
    }

    if (read(descriptor, *table, size) <= 0)
    {
        free(*table);
        return errno = EINVAL;
    }

    return 0;
}

int elf_read_string_table(int descriptor, const Elf_Shdr *section, const char **strings)
{
    if (NULL == section)
        return EINVAL;

    *strings = (char const *)malloc(section->sh_size);
    if(NULL == *strings)
    {
        return errno;
    }

    if (lseek(descriptor, section->sh_offset, SEEK_SET) < 0)
    {
        free((void *)*strings);
        return errno;
    }

    if (read(descriptor, (char *)*strings, section->sh_size) <= 0)
    {
        free((void *)*strings);
        return errno = EINVAL;
    }

    return 0;
}

int elf_read_symbol_table(int descriptor, const Elf_Shdr *section, Elf_Sym **table)
{
    if (NULL == section)
        return EINVAL;

    *table = (Elf_Sym *)malloc(section->sh_size);
    if(NULL == *table)
    {
        return errno;
    }

    if (lseek(descriptor, section->sh_offset, SEEK_SET) < 0)
    {
        free(*table);
        return errno;
    }

    if (read(descriptor, *table, section->sh_size) <= 0)
    {
        free(*table);
        return errno = EINVAL;
    }

    return 0;
}

int elf_read_relocation_table(int descriptor, const Elf_Shdr *section, Elf_Rel **table)
{
    if (NULL == section)
        return EINVAL;

    *table = (Elf_Rel *)malloc(section->sh_size);
    if(NULL == *table)
    {
        return errno;
    }

    if (lseek(descriptor, section->sh_offset, SEEK_SET) < 0)
    {
        free(*table);
        return errno;
    }

    if (read(descriptor, *table, section->sh_size) <= 0)
    {
        free(*table);
        return errno = EINVAL;
    }

    return 0;
}

int elf_section_by_index(int descriptor, const size_t index, Elf_Shdr **section)
{
    Elf_Ehdr *header = NULL;
    Elf_Shdr *sections = NULL;

    *section = NULL;

    if (
        elf_read_header(descriptor, &header) ||
        elf_read_section_table(descriptor, header, &sections)
        )
        return errno;

    if (index < header->e_shnum)
    {
        *section = (Elf_Shdr *)malloc(sizeof(Elf_Shdr));

        if (NULL == *section)
        {
            free(header);
            free(sections);

            return errno;
        }

        memcpy(*section, sections + index, sizeof(Elf_Shdr));
    }
    else
        return EINVAL;

    free(header);
    free(sections);

    return 0;
}

int elf_section_by_type(int descriptor, const size_t section_type, Elf_Shdr **section)
{
    Elf_Ehdr *header = NULL;
    Elf_Shdr *sections = NULL;
    size_t i;

    *section = NULL;

    if (
        elf_read_header(descriptor, &header) ||
        elf_read_section_table(descriptor, header, &sections)
        )
        return errno;

    for (i = 0; i < header->e_shnum; ++i)
    {
        if (section_type == sections[i].sh_type)
        {
            *section = (Elf_Shdr *)malloc(sizeof(Elf_Shdr));

            if (NULL == *section)
            {
                free(header);
                free(sections);

                return errno;
            }

            memcpy(*section, sections + i, sizeof(Elf_Shdr));

            break;
        }
    }

    free(header);
    free(sections);

    return 0;
}

int elf_section_by_name(int descriptor, const char *section_name, Elf_Shdr **section)
{
    Elf_Ehdr *header = NULL;
    Elf_Shdr *sections = NULL;
    char const *strings = NULL;
    size_t i;

    *section = NULL;

    if (
        elf_read_header(descriptor, &header) ||
        elf_read_section_table(descriptor, header, &sections) ||
        elf_read_string_table(descriptor, &sections[header->e_shstrndx], &strings)
        )
        return errno;

    for (i = 0; i < header->e_shnum; ++i)
    {
        if (!strcmp(section_name, &strings[sections[i].sh_name]))
        {
            *section = (Elf_Shdr *)malloc(sizeof(Elf_Shdr));

            if (NULL == *section)
            {
                free(header);
                free(sections);
                free((void *)strings);

                return errno;
            }

            memcpy(*section, sections + i, sizeof(Elf_Shdr));

            break;
        }
    }

    free(header);
    free(sections);
    free((void *)strings);

    return 0;
}

int elf_symbol_index_by_name(int descriptor, Elf_Shdr *section, char const *name, size_t *index)
{
    Elf_Shdr *strings_section = NULL;
    char const *strings = NULL;
    Elf_Sym *symbols = NULL;
    size_t i, amount;

    *index = 0;

    if (
        elf_section_by_index(descriptor, section->sh_link, &strings_section) ||
        elf_read_string_table(descriptor, strings_section, &strings) ||
        elf_read_symbol_table(descriptor, section, &symbols)
        )
        return errno;

    amount = section->sh_size / sizeof(Elf_Sym);

    for (i = 0; i < amount; ++i)
    {
        if (!strcmp(name, &strings[symbols[i].st_name]))
        {
            *index = i;
            break;
        }
    }

    free(strings_section);
    free((void *)strings);
    free(symbols);

    return *index == 0;
}

int elf_symbol_index_by_address(int descriptor, Elf_Shdr *section, Elf_Addr address, size_t *index)
{
    Elf_Sym *symbols = NULL;
    *index = 0;

    if (elf_read_symbol_table(descriptor, section, &symbols))
        return errno;

    size_t amount = section->sh_size / sizeof(Elf_Sym);
    for (size_t i = 0; i < amount; ++i)
    {
        if (address == symbols[i].st_value)
        {
            *index = i;
            break;
        }
    }

    free(symbols);

    return *index == 0;
}
