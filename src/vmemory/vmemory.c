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

void vmemory_example_load(vMEMORY_t *vmemory)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        vmemory->memory[i] = i;
    }
}
