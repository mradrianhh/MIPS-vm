#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vmemory.h"
#include "../device_table/device_table.h"

int vmemory_init(vMEMORY_t *vmemory)
{
    printf("\tvMEMORY initializing...\n");
    vmemory->memory = malloc(MEMORY_SIZE);
    vmemory->start = vmemory->memory;
    memset(vmemory->memory, 0, MEMORY_SIZE);

    return 0;
}

PAGE_t vmemory_read(vMEMORY_t *vmemory, uint8_t address)
{
    printf("Debug: [0x%02x + 0x%02x] = [0x%02x]: [0x%02x]\n", (int)(vmemory->start), (int)(address), (int)(vmemory->start + address), *(vmemory->start + address));
    return *(vmemory->start + address); 
}

int vmemory_write(vMEMORY_t *vmemory, PAGE_t page, uint8_t address)
{
    printf("Debug: [0x%02x + 0x%02x] = [0x%02x]: [0x%02x] -> ", (int)(vmemory->start), (int)(address), (int)(vmemory->start + address), *(vmemory->start + address));
    *(vmemory->start + address) = page;
    printf("[0x%02x]\n", *(vmemory->start + address));
    
    return 0;
}

void vmemory_example_load(vMEMORY_t *vmemory)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        vmemory->memory[i] = i;
    }
}
