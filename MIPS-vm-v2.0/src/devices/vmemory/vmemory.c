#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "vmemory.h"

static void vmemory_example_load();

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

    vmemory_example_load();
}

void vmemory_shutdown()
{
    log_info(&vmemory.logger, "Shutting down.\n");
    logger_shutdown(&vmemory.logger);
}

PAGE_t vmemory_fetch(uint8_t address)
{
    return vmemory.start[address];
}

void vmemory_write(uint8_t address, PAGE_t data)
{
    vmemory.start[address] = data;
}

static void vmemory_example_load()
{
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        vmemory.start[i] = 0;
    }
}