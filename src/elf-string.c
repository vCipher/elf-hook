#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "errors.h"
#include "elf-section.h"
#include "elf-string.h"

static int elf_string_alloc_table(size_t size, const char **strings)
{
    *strings = (const char *)malloc(size);
    if(*strings == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for ELF string table, error: %s\n", strerror(errno));
        return errno;
    }

    return SUCCESS;
}

void elf_string_destroy(const char *string)
{
    if (string == NULL)
    {
        fprintf(stdout, "ELF string has been already destroyed\n");
        return;
    }

    free((void*)string);
}

int elf_string_read_table(int descriptor, const Elf_Shdr *section, const char **strings)
{
    if (section == NULL)
        return EINVAL;

    char const *local = NULL;

    TRY
    {
        CHECK_RESULT(elf_string_alloc_table(section->sh_size, &local));
        
        if (lseek(descriptor, section->sh_offset, SEEK_SET) < 0)
            THROW(errno);

        if (read(descriptor, (char *)local, section->sh_size) <= 0)
            THROW(errno);
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to read ELF string table, error: %s\n", strerror(error));
        elf_string_destroy(local);

        return error;
    }

    *strings = local;
    return SUCCESS;
}