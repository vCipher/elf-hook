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

static int elf_section_alloc_table(size_t size, Elf_Shdr **table)
{
    *table = (Elf_Shdr *)malloc(size);
    if(*table == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for ELF section table, error: %s\n", strerror(errno));
        return errno;
    }

    return SUCCESS;
}

static int elf_section_alloc(Elf_Shdr **table)
{
    *table = (Elf_Shdr *)malloc(sizeof(Elf_Shdr));
    if(*table == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for ELF section, error: %s\n", strerror(errno));
        return errno;
    }

    return SUCCESS;
}

void elf_section_destroy(Elf_Shdr *section)
{
    if (section == NULL)
    {
        fprintf(stdout, "ELF section has been already destroyed\n");
        return;
    }

    free(section);
}

int elf_section_read_table(int descriptor, const Elf_Ehdr *header, Elf_Shdr **table)
{
    if (header == NULL)
        return EINVAL;

    Elf_Shdr *local = NULL;

    TRY
    {
        size_t size = header->e_shnum * sizeof(Elf_Shdr);
        CHECK_RESULT(elf_section_alloc_table(size, &local));
        
        if (lseek(descriptor, header->e_shoff, SEEK_SET) < 0)
            THROW(errno);

        if (read(descriptor, local, size) <= 0)
            THROW(errno);
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to read ELF section table, error: %s\n", strerror(error));
        elf_section_destroy(local);

        return error;
    }

    *table = local;
    return SUCCESS;
}

int elf_section_find_by_index(int descriptor, const size_t index, Elf_Shdr **section)
{
    Elf_Ehdr *header = NULL;
    Elf_Shdr *sections = NULL;
    int result = SUCCESS;

    *section = NULL;

    TRY
    {
        CHECK_RESULT(elf_header_read(descriptor, &header));
        CHECK_RESULT(elf_section_read_table(descriptor, header, &sections));

        if (index >= header->e_shnum)
            THROW(EINVAL);

        CHECK_RESULT(elf_section_alloc(section));
        memcpy(*section, sections + index, sizeof(Elf_Shdr));
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to find ELF section by index %lu, error: %s\n", index, strerror(error));
        result = error;
    }

    elf_header_destroy(header);
    elf_section_destroy(sections);

    return result;
}

int elf_section_find_by_type(int descriptor, const size_t section_type, Elf_Shdr **section)
{
    Elf_Ehdr *header = NULL;
    Elf_Shdr *sections = NULL;
    int result = ENODATA;

    TRY
    {
        CHECK_RESULT(elf_header_read(descriptor, &header));
        CHECK_RESULT(elf_section_read_table(descriptor, header, &sections));

        for (size_t index = 0; index < header->e_shnum; ++index)
        {
            if (section_type != sections[index].sh_type)
                continue;

            CHECK_RESULT(elf_section_alloc(section));
            memcpy(*section, sections + index, sizeof(Elf_Shdr));
            result = SUCCESS;
            
            break;
        }
        
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to find ELF section by type %lu, error: %s\n", section_type, strerror(error));
        result = error;
    }

    elf_header_destroy(header);
    elf_section_destroy(sections);

    return result;
}

int elf_section_find_by_name(int descriptor, const char *section_name, Elf_Shdr **section)
{
    Elf_Ehdr *header = NULL;
    Elf_Shdr *sections = NULL;
    char const *strings = NULL;
    int result = ENODATA;

    TRY
    {
        CHECK_RESULT(elf_header_read(descriptor, &header));
        CHECK_RESULT(elf_section_read_table(descriptor, header, &sections));
        CHECK_RESULT(elf_string_read_table(descriptor, &sections[header->e_shstrndx], &strings));

        for (size_t index = 0; index < header->e_shnum; ++index)
        {
            if (strcmp(section_name, &strings[sections[index].sh_name]))
                continue;

            CHECK_RESULT(elf_section_alloc(section));
            memcpy(*section, sections + index, sizeof(Elf_Shdr));
            result = SUCCESS;
            
            break;
        }        
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to find ELF section by name %s, error: %s\n", section_name, strerror(error));
        result = error;
    }

    elf_header_destroy(header);
    elf_section_destroy(sections);
    elf_string_destroy(strings);

    return result;
}