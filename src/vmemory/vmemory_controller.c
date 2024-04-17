#include <stdio.h>
#include <unistd.h>

#include "vmemory_controller.h"
#include "../device_table/device_table.h"
#include "vsysbus/vsysbus.h"

static void *vmemory_controller_loop(void *vargp);
static void process_sysbus_packet(vMEMORY_CONTROLLER_t *controller, vSYSBUS_PACKET_t *packet);

static DEVICE_TABLE_ENTRY_t *device_info_cpus;
static uint8_t cpu_amount;

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

int vmemory_controller_start(vMEMORY_CONTROLLER_t *controller)
{
    printf("Device vMEMORY_CONTROLLER(%d) starting...\n", controller->device_info.device_id);

    // Fetch device_info for all CPUs from device_table.
    {
        DEVICE_TABLE_READ_RETURN_t *ret = device_table_read_type(DEVICE_TYPE_CPU);
        device_info_cpus = ret->entry_ptr;
        cpu_amount = ret->array_size;
    }
    
    pthread_create(&(controller->device_info.device_tid), NULL, vmemory_controller_loop, (void *)controller);

    return 0;
}

static void *vmemory_controller_loop(void *vargp)
{
    vMEMORY_CONTROLLER_t *controller = (vMEMORY_CONTROLLER_t *)vargp;
    vSYSBUS_PACKET_t *packet;
    while (1)
    {
        for (int i = 0; i < cpu_amount; i++)
        {
            packet = vsysbus_read(device_info_cpus[i].device_id);
            if (packet != NULL)
            {
                printf("vMEMORY_CONTROLLER(%d) - Package received from Device v%s(%d): [0x%02x]\n", controller->device_info.device_id, convert_device_type_str(device_info_cpus[i].device_type), packet->device_id, packet->data);
                process_sysbus_packet(controller, packet);
            }
        }
        sleep(3);
    }

    pthread_exit(NULL);
}

static void process_sysbus_packet(vMEMORY_CONTROLLER_t *controller, vSYSBUS_PACKET_t *packet)
{
    PAGE_t ret = vmemory_read(&controller->vmemory, packet->data);
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