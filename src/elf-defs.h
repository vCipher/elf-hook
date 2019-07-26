#pragma once

// rename standart types for convenience
#if defined __x86_64 || defined __aarch64__
    #define Elf_Ehdr Elf64_Ehdr
    #define Elf_Shdr Elf64_Shdr
    #define Elf_Sym Elf64_Sym
    #define Elf_Rel Elf64_Rela
    #define Elf_Addr Elf64_Addr
    #define ELF_R_SYM ELF64_R_SYM
    #define ELF_R_TYPE ELF64_R_TYPE
    #define REL_DYN ".rela.dyn"
    #define REL_PLT ".rela.plt"
    #define REL_TEXT ".rela.text"
#else
    #define Elf_Ehdr Elf32_Ehdr
    #define Elf_Shdr Elf32_Shdr
    #define Elf_Sym Elf32_Sym
    #define Elf_Rel Elf32_Rel
    #define Elf_Addr Elf32_Addr
    #define ELF_R_SYM ELF32_R_SYM
    #define ELF_R_TYPE ELF32_R_TYPE
    #define REL_DYN ".rel.dyn"
    #define REL_PLT ".rel.plt"
    #define REL_TEXT ".rel.text"
#endif