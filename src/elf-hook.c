#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "dl-info.h"
#include "elf-defs.h"
#include "elf-relocation.h"
#include "elf-symbol.h"
#include "elf-hook.h"
#include "elf-file.h"
#include "elf-section.h"
#include "elf-string.h"
#include "errors.h"

typedef void* (*elf_relocation_fixup_t)(Elf_Rel *rel, void *module_address, void *substitution);

static int elf_dynsymbol_find_index_by_name(int descriptor, char const *name, size_t *index)
{
    int result = SUCCESS;
    Elf_Shdr *dynsym = NULL;

    TRY
    {
        CHECK_RESULT(elf_section_find_by_type(descriptor, SHT_DYNSYM, &dynsym));
        CHECK_RESULT(elf_symbol_find_index_by_name(descriptor, dynsym, name, index));
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to get .dynsym symbol for %s, error: %s\n", name, strerror(error));
        result = error;
    }

    elf_section_destroy(dynsym);

    return result;
}

static void* rel_jump_slot_fixup(Elf_Rel *rel, void *module_address, void *substitution)
{
    void **name_address = (void **)(module_address + rel->r_offset);
    void *original = *name_address;
    *name_address = substitution;

    return original;
}

static void* rel_pc32_fixup(Elf_Rel *rel, void *module_address, void *substitution)
{
    static size_t page_size;

    if (!page_size)
        page_size = sysconf(_SC_PAGESIZE);

    void **name_address = (void **)(module_address + rel->r_offset);
    void *original = *name_address + (size_t)name_address + sizeof(size_t);
    void *page_address = (void *)(((size_t)name_address) & (((size_t)-1) ^ (page_size - 1)));

    if(mprotect(page_address, page_size, PROT_READ | PROT_WRITE) < 0)
    {
        return NULL;
    }

    *name_address = substitution - (size_t)name_address - sizeof(size_t);
    if(mprotect(page_address, page_size, PROT_READ | PROT_EXEC) < 0)
    {
        *name_address = original - (size_t)name_address - sizeof(size_t);
        return NULL;
    }

    return original;
}

static void* rel_glob_dat_fixup(Elf_Rel *rel, void *module_address, void *substitution)
{
    static size_t page_size;

    if (!page_size)
        page_size = sysconf(_SC_PAGESIZE);

    void **name_address = (void **)(module_address + rel->r_offset);
    void *original = *name_address;
    void *page_address = (void *)(((size_t)name_address) & (((size_t)-1) ^ (page_size - 1)));

    if(mprotect(page_address, page_size, PROT_READ | PROT_WRITE) < 0)
    {
        return NULL;
    }

    *name_address = substitution;
    if(mprotect(page_address, page_size, PROT_READ | PROT_EXEC) < 0)
    {
        *name_address = original;
        return NULL;
    }

    return original;
}

static void* rel_empty_fixup(Elf_Rel *rel, void *module_address, void *substitution)
{
    (void) rel;
    (void) module_address;
    (void) substitution;
    
    return NULL;
}

static elf_relocation_fixup_t get_rel_plt_fixup(Elf_Rel *rel)
{
    switch (ELF_R_TYPE(rel->r_info))
    {
        case R_X86_64_JUMP_SLOT: return rel_jump_slot_fixup;
        default: break;
    }

    return rel_empty_fixup;
}

static elf_relocation_fixup_t get_rel_dyn_fixup(Elf_Rel *rel)
{
    switch (ELF_R_TYPE(rel->r_info))
    {
        case R_X86_64_PC32:     return rel_pc32_fixup;
        case R_X86_64_GLOB_DAT: return rel_glob_dat_fixup;
        default: break;
    }

    return rel_empty_fixup;
}

static void *elf_hook_plt(int descriptor, void *module_address, size_t symbol_index, void *substitution)
{
    Elf_Shdr *rel_plt = NULL;
    void *result = NULL;

    TRY
    {
        CHECK_RESULT(elf_section_find_by_name(descriptor, REL_PLT, &rel_plt))

        Elf_Rel *rel_plt_table = (Elf_Rel *)(((size_t)module_address) + rel_plt->sh_addr);
        size_t rel_plt_amount = rel_plt->sh_size / sizeof(Elf_Rel);

        for (size_t index = 0; index < rel_plt_amount; ++index)
        {
            Elf_Rel rel = rel_plt_table[index];
            size_t rel_symbol_index = ELF_R_SYM(rel.r_info);
            
            if (rel_symbol_index == symbol_index)
            {
                elf_relocation_fixup_t fixup = get_rel_plt_fixup(&rel);
                result = fixup(&rel, module_address, substitution);

                break;
            }
        }
    }
    CATCH(error)
    { 
        fprintf(stderr, "Failed to hook symbol %lu with %p, error: %s\n", symbol_index, substitution, strerror(error));
    }

    elf_section_destroy(rel_plt);

    return result;
}

static void *elf_hook_dyn(int descriptor, void *module_address, size_t symbol_index, void *substitution)
{
    Elf_Shdr *rel_dyn = NULL;
    void *result = NULL;

    TRY
    {
        CHECK_RESULT(elf_section_find_by_name(descriptor, REL_DYN, &rel_dyn))

        Elf_Rel *rel_dyn_table = (Elf_Rel *)(((size_t)module_address) + rel_dyn->sh_addr);
        size_t rel_dyn_amount = rel_dyn->sh_size / sizeof(Elf_Rel);

        for (size_t index = 0; index < rel_dyn_amount; ++index)
        {
            Elf_Rel rel = rel_dyn_table[index];
            size_t rel_symbol_index = ELF_R_SYM(rel.r_info);

            if (rel_symbol_index == symbol_index)
            {
                elf_relocation_fixup_t fixup = get_rel_dyn_fixup(&rel);
                result = fixup(&rel, module_address, substitution);
            }
        }
    }
    CATCH(error)
    { 
        fprintf(stderr, "Failed to hook symbol %lu with %p, error: %s\n", symbol_index, substitution, strerror(error));
    }

    elf_section_destroy(rel_dyn);

    return result;
}

static void *elf_hook_internal(int descriptor, void *module_address, size_t symbol_index, void *substitution)
{
    void *original = NULL;
    original = original == NULL ? elf_hook_plt(descriptor, module_address, symbol_index, substitution) : original;
    original = original == NULL ? elf_hook_dyn(descriptor, module_address, symbol_index, substitution) : original;

    return original;
}

void *elf_hook(dl_info_t *dl_info, const char *function_name, void *substitution_address)
{
    if (dl_info == NULL) return NULL;
    if (function_name == NULL) return NULL;
    if (substitution_address == NULL) return NULL;

    int descriptor = -1;
    void *result = NULL;

    TRY
    {
        size_t symbol_index = -1;

        CHECK_RESULT(elf_file_open(dl_info->file_name, &descriptor));
        CHECK_RESULT(elf_dynsymbol_find_index_by_name(descriptor, function_name, &symbol_index));

        result = elf_hook_internal(descriptor, dl_info->base_address, symbol_index, substitution_address);
    }
    CATCH()
    {
        fprintf(stderr, "Failed to hook function %s with %p\n", function_name, substitution_address);
    }

    if (descriptor != -1)
        close(descriptor);

    return result;
}
