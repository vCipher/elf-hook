#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "errors.h"
#include "elf-header.h"

static int elf_header_alloc(Elf_Ehdr **header)
{
    *header = (Elf_Ehdr *)malloc(sizeof(Elf_Ehdr));
    if(*header == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for ELF header, error: %s\n", strerror(errno));
        return errno;
    }

    return SUCCESS;
}

void elf_header_destroy(Elf_Ehdr *header)
{
    if (header == NULL)
    {
        fprintf(stdout, "ELF header has been already destroyed\n");
        return;
    }

    free(header);
}

int elf_header_read(int descriptor, Elf_Ehdr **header)
{
    Elf_Ehdr *local = NULL;

    TRY
    {
        CHECK_RESULT(elf_header_alloc(&local));
        
        if (lseek(descriptor, 0, SEEK_SET) < 0)
            THROW(errno);

        if (read(descriptor, local, sizeof(Elf_Ehdr)) <= 0)
            THROW(errno);
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to read ELF header, error: %s\n", strerror(error));
        elf_header_destroy(local);

        return error;
    }

    *header = local;
    return SUCCESS;
}