#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "vmemory.h"

static vMemory_t memory;

void vmemory_init()
{
    memory.device_info = device_table_add(DEVICE_TYPE_MEMORY);

    memory.logger.device_info = memory.device_info;
    memory.logger.file_name = "../logs/memory.txt";
    logger_init(&memory.logger);

    log_info(&memory.logger, "Initializing.\n");

    memory.start = calloc(VMEMORY_SIZE, sizeof(uint8_t));
    memory.size = VMEMORY_SIZE;

    if (!memory.start) {
        log_error(&memory.logger, "Failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }
}

void vmemory_shutdown()
{
    log_info(&memory.logger, "Shutting down.\n");
    logger_shutdown(&memory.logger);
}

uint32_t vmemory_read_word(uint32_t address)
{
    Word_t instruction;
    for (int i = address; i < address + 4; i++)
    {
        instruction.bytes[3 - (i - address)] = memory.start[i];
    }
    return instruction.word;
}

uint8_t vmemory_read_byte(uint32_t address)
{
    return memory.start[address];
}

void vmemory_write_word(uint32_t word, uint32_t address)
{
    Word_t internal;
    internal.word = word;

    for (int i = address; i < address + 4; i++)
    {
        memory.start[i] = internal.bytes[3 - (i - address)];
    }
}

void vmemory_write_byte(uint8_t byte, uint32_t address)
{
    memory.start[address] = byte;
}

vMemory_t *vmemory_get_memory_ref()
{
    return &memory;
}
