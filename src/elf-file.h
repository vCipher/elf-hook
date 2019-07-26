#pragma once

int elf_file_open(const char* path, int *descriptor);
void elf_file_close(int descriptor);