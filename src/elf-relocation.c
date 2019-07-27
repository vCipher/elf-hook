#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "errors.h"
#include "elf-section.h"
#include "elf-relocation.h"

static int elf_relocation_alloc_table(size_t size, Elf_Rel **table)
{

    *table = (Elf_Rel *)malloc(size);
    if(*table == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for ELF relocation table, error: %s\n", strerror(errno));
        return errno;
    }

    return SUCCESS;
}

void elf_relocation_destroy(Elf_Rel *relocation)
{
    if (relocation == NULL)
    {
        fprintf(stdout, "ELF relocation has been already destroyed\n");
        return;
    }

    free(relocation);
}

int elf_read_relocation_table(int descriptor, const Elf_Shdr *section, Elf_Rel **table)
{
    if (section == NULL)
        return EINVAL;

    Elf_Rel *local = NULL;

    TRY
    {
        CHECK_RESULT(elf_relocation_alloc_table(section->sh_size, &local));
        
        if (lseek(descriptor, section->sh_offset, SEEK_SET) < 0)
            THROW(errno);

        if (read(descriptor, *table, section->sh_size) <= 0)
            THROW(errno);
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to read ELF relocation table, error: %s\n", strerror(error));
        elf_relocation_destroy(local);

        return error;
    }

    *table = local;
    return SUCCESS;
}
