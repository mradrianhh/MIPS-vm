#include <stdlib.h>
#include <string.h>

#include "vmemory.h"
#include "../common/common.h"

void vmemory_init(vMEMORY_t *vmemory)
{
    vmemory->device_id = device_id_count++;
    vmemory->device_type = DEVICE_TYPE_MEMORY;
    vmemory->memory = malloc(MEMORY_SIZE);
    vmemory->start = vmemory->memory;
    memset(vmemory->memory, 0, MEMORY_SIZE);
}

void vmemory_example_load(vMEMORY_t *vmemory)
{
    for(int i = 0; i < MEMORY_SIZE; i++)
    {
        vmemory->memory[i] = i;
    }
}
