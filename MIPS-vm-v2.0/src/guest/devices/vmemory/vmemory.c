#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "vmemory.h"

static vMEMORY_t vmemory;

void vmemory_init()
{
    vmemory.device_info = device_table_add(DEVICE_TYPE_MEMORY);

    vmemory.logger.device_info = vmemory.device_info;
    vmemory.logger.file_name = "../logs/vmemory.txt";
    logger_init(&vmemory.logger);

    log_info(&vmemory.logger, "Initializing.\n");

    vmemory.start = malloc(MEMORY_SIZE);
    memset(vmemory.start, 0, MEMORY_SIZE);
}

void vmemory_shutdown()
{
    log_info(&vmemory.logger, "Shutting down.\n");
    logger_shutdown(&vmemory.logger);
}

uint32_t vmemory_read_word(uint32_t address)
{
    uint32_t phys_addr = vmemory_map_pgm_to_phys_addr(address);
    log_debug(&vmemory.logger, "vMemory read word: Translating address from 0x%08x to 0x%08x\n", address, phys_addr);

    Word_t instruction;
    for (int i = phys_addr; i < phys_addr + 4; i++)
    {
        instruction.bytes[3 - (i - phys_addr)] = vmemory.start[i];
    }
    return instruction.word;
}

uint8_t vmemory_read_byte(uint32_t address)
{
    uint32_t phys_addr = vmemory_map_pgm_to_phys_addr(address);
    log_debug(&vmemory.logger, "vMemory read byte: Translating address from 0x%08x to 0x%08x\n", address, phys_addr);

    return vmemory.start[phys_addr];
}

void vmemory_write_word(uint32_t word, uint32_t address)
{
    uint32_t phys_addr = vmemory_map_pgm_to_phys_addr(address);
    log_debug(&vmemory.logger, "vMemory write word: Translating address from 0x%08x to 0x%08x\n", address, phys_addr);

    Word_t internal;
    internal.word = word;

    for (int i = phys_addr; i < phys_addr + 4; i++)
    {
        vmemory.start[i] = internal.bytes[3 - (i - phys_addr)];
    }
}

void vmemory_write_byte(uint8_t byte, uint32_t address)
{
    uint32_t phys_addr = vmemory_map_pgm_to_phys_addr(address);
    log_debug(&vmemory.logger, "vMemory write byte: Translating address from 0x%08x to 0x%08x\n", address, phys_addr);

    vmemory.start[phys_addr] = byte;
}

uint32_t vmemory_map_pgm_to_phys_addr(uint32_t pgm_address)
{
    if (pgm_address >= KSEG1_PGM_START_VADDR && pgm_address <= KSEG1_PGM_END_VADDR)
    {
        return pgm_address & KSEG1_PGM_TO_PHYS_MASK;
    }

    return pgm_address;
}

uint8_t *vmemory_get_memory_ref()
{
    return vmemory.start;
}
