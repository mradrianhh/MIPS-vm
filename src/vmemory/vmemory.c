#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "vmemory.h"
#include "vmemorybus.h"
#include "device_table/device_table.h"

static void *vmemory_loop(void *vargp);
static void poll_vmemorybus(vMEMORY_t *vmemory);
static void process_vmemorybus_packets(vMEMORY_t *vmemory, vMEMORYBUS_PACKETS_t *packets);
static void process_vmemorybus_ctl_read(vMEMORY_t *vmemory, uint8_t address);
static void process_vmemorybus_ctl_write(vMEMORY_t *vmemory, PAGE_t data, uint8_t address);
static PAGE_t vmemory_read(vMEMORY_t *vmemory, uint8_t address);
static int vmemory_write(vMEMORY_t *vmemory, PAGE_t page, uint8_t address);
static void vmemory_example_load(vMEMORY_t *vmemory);

int vmemory_init(vMEMORY_t *vmemory)
{
    vmemory->device_info = device_table_add(DEVICE_TYPE_MEMORY);

    vmemory->logger.device_info = vmemory->device_info;
    vmemory->logger.file_name = "../logs/vmemory.txt";
    logger_init(&vmemory->logger);

    log_info(&vmemory->logger, "Initializing.\n");

    vmemory->start = malloc(MEMORY_SIZE);
    memset(vmemory->start, 0, MEMORY_SIZE);

    vmemory_example_load(vmemory);

    return 0;
}

int vmemory_start(vMEMORY_t *vmemory)
{
    log_info(&vmemory->logger, "Starting.\n");
    vmemory->device_info->device_running = DEVICE_RUNNING;

    pthread_create(&(vmemory->device_info->device_tid), NULL, vmemory_loop, (void *)vmemory);

    return 0;
}

int vmemory_shutdown(vMEMORY_t *vmemory)
{
    log_info(&vmemory->logger, "Shutting down.\n");
    vmemory->device_info->device_running = DEVICE_STOPPED;
    logger_shutdown(&vmemory->logger);

    return 0;
}

void memory_dump(vMEMORY_t *vmemory)
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
        printf(" [0x%02x] ", vmemory->start[i]);
    }
    printf("\n");

    char input;
    scanf("Press any key to continue. . . %c", &input);
}

///
/// Private
///

static void *vmemory_loop(void *vargp)
{
    vMEMORY_t *vmemory = (vMEMORY_t *)vargp;

    log_info(&vmemory->logger, "Running on thread [0x%016lx].\n", vmemory->device_info->device_tid);

    while (1)
    {
        poll_vmemorybus(vmemory);
        sleep(1);
    }

    pthread_exit(NULL);
}

static void poll_vmemorybus(vMEMORY_t *vmemory)
{
    int rc = 0;
    vMEMORYBUS_PACKETS_t packets;
    rc = vmemorybus_read(vMEMORYBUS_SEL_IN, &packets);
    if (!rc && packets.control.unit_used)
    {
        log_trace(&vmemory->logger, "Package {Data: 0x%02x Access: 0x%02x Control 0x%02x} read from vMEMORYBUS_IN.\n", packets.data, packets.access, packets.control.control_field);
        process_vmemorybus_packets(vmemory, &packets);
    }
}

static void process_vmemorybus_packets(vMEMORY_t *vmemory, vMEMORYBUS_PACKETS_t *packets)
{
    switch (packets->control.access_op)
    {
    case vMEMORYBUS_CONTROL_READ:
        process_vmemorybus_ctl_read(vmemory, packets->access);
        break;
    case vMEMORYBUS_CONTROL_WRITE:
        process_vmemorybus_ctl_write(vmemory, packets->data, packets->access);
        break;
    default:
        break;
    }
}

static void process_vmemorybus_ctl_read(vMEMORY_t *vmemory, uint8_t address)
{
    PAGE_t read_page = vmemory_read(vmemory, address);
    vMEMORYBUS_PACKETS_t write_packet =
        {
            .data = read_page,
            .control.unit_used = vMEMORYBUS_CONTROL_USED,
        };
    vmemorybus_write(vMEMORYBUS_SEL_OUT, &write_packet);
    log_trace(&vmemory->logger, "Package {Data: 0x%02x Access: 0x%02x Control 0x%02x} written to vMEMORYBUS_OUT.\n",
              write_packet.data, write_packet.access, write_packet.control.control_field);
}

static void process_vmemorybus_ctl_write(vMEMORY_t *vmemory, uint8_t data, uint8_t address)
{
    vmemory_write(vmemory, data, address);
}

static PAGE_t vmemory_read(vMEMORY_t *vmemory, uint8_t address)
{
    log_trace(&vmemory->logger, "Read from vMEMORY. [%p]: [0x%02x].\n", (void *)(&vmemory->start[address]), vmemory->start[address]);
    return vmemory->start[address];
}

static int vmemory_write(vMEMORY_t *vmemory, PAGE_t page, uint8_t address)
{
    vmemory->start[address] = page;
    log_trace(&vmemory->logger, "Write to vMEMORY. [%p]: [0x%02x].\n", (void *)(&vmemory->start[address]), vmemory->start[address]);
    return 0;
}

static void vmemory_example_load(vMEMORY_t *vmemory)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        vmemory->start[i] = 0;
    }

    for (int i = 0; i < 5; i++)
    {
        vmemory->start[i] = i << 4;
    }
}