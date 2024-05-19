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

Word_t vmemory_fetch_instruction(uint32_t address)
{
    Word_t instruction;
    for(int i = address; i < address + 4; i++)
    {
        instruction.bytes[i - address] = vmemory.start[i];
    }
    return instruction;
}

uint8_t *vmemory_get_memory_ref()
{
    return vmemory.start;
}
