#include <assert.h>
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
#include "elf-relocation.h"
#include "elf-symbol.h"
#include "elf-hook.h"
#include "elf-file.h"
#include "elf-section.h"
#include "elf-string.h"
#include "errors.h"

typedef void* (*relocation_fixup_t)(Elf_Rel *rel, void *module_address, void *substitution);

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

static relocation_fixup_t get_rel_plt_fixup(Elf_Rel *rel)
{
    switch (ELF_R_TYPE(rel->r_info))
    {
        case R_X86_64_JUMP_SLOT: return rel_jump_slot_fixup;
        default: break;
    }

    return rel_empty_fixup;
}

static relocation_fixup_t get_rel_dyn_fixup(Elf_Rel *rel)
{
    switch (ELF_R_TYPE(rel->r_info))
    {
        case R_X86_64_PC32:     return rel_pc32_fixup;
        case R_X86_64_GLOB_DAT: return rel_glob_dat_fixup;
        default: break;
    }

    return rel_empty_fixup;
}

static void *elf_hook_internal(int descriptor, void *module_address, size_t symbol_index, void *substitution)
{
    Elf_Shdr *rel_plt = NULL;
    Elf_Shdr *rel_dyn = NULL;

    Elf_Rel *rel_plt_table = NULL;
    Elf_Rel *rel_dyn_table = NULL;

    size_t rel_plt_amount;
    size_t rel_dyn_amount;

    void *original = NULL;

    if (
        elf_section_find_by_name(descriptor, REL_PLT, &rel_plt) ||  //get ".rel.plt" (for 32-bit) or ".rela.plt" (for 64-bit) section
        elf_section_find_by_name(descriptor, REL_DYN, &rel_dyn)  //get ".rel.dyn" (for 32-bit) or ".rela.dyn" (for 64-bit) section
       )
    {
        free(rel_plt);
        free(rel_dyn);

        return original;
    }

    rel_plt_table = (Elf_Rel *)(((size_t)module_address) + rel_plt->sh_addr);  //init the ".rel.plt" array
    rel_plt_amount = rel_plt->sh_size / sizeof(Elf_Rel);  //and get its size

    rel_dyn_table = (Elf_Rel *)(((size_t)module_address) + rel_dyn->sh_addr);  //init the ".rel.dyn" array
    rel_dyn_amount = rel_dyn->sh_size / sizeof(Elf_Rel);  //and get its size

    //release the data used
    free(rel_plt);
    free(rel_dyn);

    //now we've got ".rel.plt" (needed for PIC) table and ".rel.dyn" (for non-PIC) table and the symbol's index
    for (size_t i = 0; i < rel_plt_amount; ++i)  //lookup the ".rel.plt" table
    {
        Elf_Rel rel = rel_plt_table[i];
        size_t rel_symbol_index = ELF_R_SYM(rel.r_info);
        
        if (rel_symbol_index == symbol_index)  //if we found the symbol to substitute in ".rel.plt"
        {
            relocation_fixup_t fixup = get_rel_plt_fixup(&rel);
            original = fixup(&rel, module_address, substitution);

            break;  //the target symbol appears in ".rel.plt" only once
        }
    }

    if (original)
    {
        return original;
    }

    //we will get here only with 32-bit non-PIC module
    for (size_t i = 0; i < rel_dyn_amount; ++i)  //lookup the ".rel.dyn" table
    {
        Elf_Rel rel = rel_dyn_table[i];
        size_t rel_symbol_index = ELF_R_SYM(rel.r_info);

        if (rel_symbol_index == symbol_index)  //if we found the symbol to substitute in ".rel.dyn"
        {
            relocation_fixup_t fixup = get_rel_dyn_fixup(&rel);
            original = fixup(&rel, module_address, substitution);
        }
    }

    return original;
}

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

static int elf_dynsymbol_find_index_by_address(int descriptor, Elf_Addr address, size_t *index)
{
    int result = SUCCESS;
    Elf_Shdr *dynsym = NULL;

    TRY
    {
        CHECK_RESULT(elf_section_find_by_type(descriptor, SHT_DYNSYM, &dynsym));
        CHECK_RESULT(elf_symbol_find_index_by_address(descriptor, dynsym, address, index));
    }
    CATCH(error)
    {
        fprintf(stderr, "Failed to get .dynsym symbol for %p, error: %s\n", (void*)address, strerror(error));
        result = error;
    }

    elf_section_destroy(dynsym);

    return result;
}

void *elf_hook(void *function_address, void *substitution_address)
{
    if (function_address == NULL) return NULL;
    if (substitution_address == NULL) return NULL;

    int descriptor = -1;
    void *result = NULL;

    TRY
    {
        char const *module_filename = NULL;
        void *module_address = NULL;

        CHECK_RESULT(get_module_base_address(function_address, &module_filename, &module_address));
        CHECK_RESULT(elf_file_open(module_filename, &descriptor));

        size_t symbol_index = -1;
        Elf_Addr symbol_address = function_address - module_address;
        CHECK_RESULT(elf_dynsymbol_find_index_by_address(descriptor, symbol_address, &symbol_index));

        result = elf_hook_internal(descriptor, module_address, symbol_index, substitution_address);
    }
    CATCH()
    {
        fprintf(stderr, "Failed to hook function %p with %p\n", function_address, substitution_address);
    }

    if (descriptor != -1)
        close(descriptor);

    return result;
}

void *elf_hook_dl(const char *module_filename, void *module_address, const char *name, void *substitution)
{
    if (module_filename == NULL) return NULL;
    if (name == NULL) return NULL;
    if (substitution == NULL) return NULL;

    int descriptor = -1;
    void *result = NULL;

    TRY
    {
        size_t symbol_index = -1;
        CHECK_RESULT(elf_file_open(module_filename, &descriptor));
        CHECK_RESULT(elf_dynsymbol_find_index_by_name(descriptor, name, &symbol_index));

        result = elf_hook_internal(descriptor, module_address, symbol_index, substitution);
    }
    CATCH()
    {
        fprintf(stderr, "Failed to hook function %s with %p\n", name, substitution);
    }

    if (descriptor != -1)
        close(descriptor);

    return result;
}
