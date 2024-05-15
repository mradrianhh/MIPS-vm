#ifndef _MIPSVM_ELF_TYPES_H_
#define _MIPSVM_ELF_TYPES_H_

#include <elf.h>

typedef union
{
    Elf32_Ehdr ehdr;
    uint8_t bytes[sizeof(Elf32_Ehdr)];
} Elf32_Ehdr_t;

typedef union
{
    Elf32_Shdr shdr;
    uint8_t bytes[sizeof(Elf32_Shdr)];
} Elf32_Shdr_t;

typedef union
{
    Elf32_Phdr phdr;
    uint8_t bytes[sizeof(Elf32_Phdr)];
} Elf32_Phdr_t;

#endif