#ifndef _MIPSVM_CONVERT_H_
#define _MIPSVM_CONVERT_H_

#include <elf.h>

void convert_elf32_ehdr_be_le(Elf32_Ehdr *ehdr);
void convert_elf32_shdr_be_le(Elf32_Shdr *shdr);
void convert_elf32_phdr_be_le(Elf32_Phdr *phdr);

#endif