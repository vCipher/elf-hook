#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "errors.h"
#include "elf-file.h"

int elf_file_open(const char* path, int *descriptor)
{
    *descriptor = open(path, O_RDONLY);
    if (*descriptor < 0)
    {
        fprintf(stderr, "Failed to open file %s, error: %s\n", path, strerror(errno));
        return errno;
    }
    
    return SUCCESS;
}

void elf_file_close(int descriptor)
{
    close(descriptor);
}