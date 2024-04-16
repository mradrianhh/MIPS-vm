#include <stdio.h>

#include "vmemory_controller.h"
#include "../device_table/device_table.h"

int vmemory_controller_init(vMEMORY_CONTROLLER_t *controller)
{
    controller->device_info.device_id = device_id_count++;
    controller->device_info.device_type = DEVICE_TYPE_MEMORY_CONTROLLER;
    device_table_add(controller->device_info);
    printf("Device vMEMORY_CONTROLLER(%d) initializing...\n", controller->device_info.device_id);
    vmemory_init(&controller->vmemory);
    vmemory_example_load(&controller->vmemory);

    return 0;
}

void memory_dump(vMEMORY_CONTROLLER_t *controller)
{
    printf("*\n");
    printf("* Memory Dump\n");
    printf("*");
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        if (i % 8 == 0)
        {
            printf("\n");
        }
        printf(" [0x%02x] ", controller->vmemory.memory[i]);
    }
    printf("\n");
}