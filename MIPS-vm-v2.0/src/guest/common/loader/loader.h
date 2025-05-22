#ifndef _MIPSVM_GUEST_COMMON_LOADER_H_
#define _MIPSVM_GUEST_COMMON_LOADER_H_

void loader_load_elf32(const char *filename);

void loader_flash_rom(const char *filename);

#endif
