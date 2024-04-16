#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vcpu/vcpu.h"
#include "vclock/vclock.h"
#include "vsysbus/vsysbus.h"
#include "vmemory/vmemory.h"
#include "common/common.h"

static void init();
static void start();
static void stop();

static void memory_dump(vMEMORY_t *vmemory);
static void register_dump(vCPU_t *vcpu);
static void device_table_dump(DEVICE_TABLE_t *device_table);

static int rc;
static char input;
static int running;

static vSYSBUS_t sysbus;
static vCPU_t vcpu;
static vMEMORY_t vmemory;
static vCLOCK_t vclock;
static DEVICE_TABLE_t device_table;

int main()
{
    // init();
    // start();
    device_table_init(&device_table);

    DEVICE_TABLE_ENTRY_t entry;
    entry.device_id = 1;
    entry.device_type = DEVICE_TYPE_CLOCK;
    device_table_add(&device_table, entry);

    entry.device_id = 2;
    entry.device_type = DEVICE_TYPE_CPU;
    device_table_add(&device_table, entry);

    device_table_dump(&device_table);

    return 0;
}

void init()
{
    if (rc = vsysbus_init(&sysbus))
    {
        printf("Error: vsysbus_init() returned with rc=%d", rc);
        exit(1);
    }

    if (rc = vcpu_init(&vcpu))
    {
        printf("Error: vcpu_init() returned with rc=%d", rc);
        exit(1);
    }
}

void start()
{
    if (rc = vsysbus_start(&sysbus))
    {
        printf("Error: vsysbus_start() returned with rc=%d", rc);
        exit(1);
    }
}

void stop()
{
    if (rc = vsysbus_stop(&sysbus))
    {
        printf("Error: vsysbus_stop() returned with rc=%d", rc);
        exit(1);
    }
}

static void memory_dump(vMEMORY_t *vmemory)
{
    printf("*\n");
    printf("* Memory Dump\n");
    printf("*\n");
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        if (i % 8 == 0)
        {
            printf("\n");
        }
        printf(" [0x%02x] ", vmemory->memory[i]);
    }
    printf("\n");
}

static void register_dump(vCPU_t *vcpu)
{
    printf("*\n");
    printf("* Register Dump\n");
    printf("*\n");
    printf("R0:  [0x%02x]\n", vcpu->registers.R0);
    printf("R1:  [0x%02x]\n", vcpu->registers.R1);
    printf("R2:  [0x%02x]\n", vcpu->registers.R2);
    printf("R3:  [0x%02x]\n", vcpu->registers.R3);
    printf("R4:  [0x%02x]\n", vcpu->registers.R4);
    printf("R5:  [0x%02x]\n", vcpu->registers.R5);
    printf("R6:  [0x%02x]\n", vcpu->registers.R6);
    printf("R7:  [0x%02x]\n", vcpu->registers.R7);
    printf("R8:  [0x%02x]\n", vcpu->registers.R8);
    printf("R9:  [0x%02x]\n", vcpu->registers.R9);
    printf("R10: [0x%02x]\n", vcpu->registers.R10);
    printf("R11: [0x%02x]\n", vcpu->registers.R11);
    printf("R12: [0x%02x]\n", vcpu->registers.R12);
    printf("PC:  [0x%02x]\n", vcpu->registers.PC);
    printf("LR:  [0x%02x]\n", vcpu->registers.LR);
    printf("SP:  [0x%02x]\n", vcpu->registers.SP);
}

static void device_table_dump(DEVICE_TABLE_t *device_table)
{
    printf("*\n");
    printf("* Device Table Dump\n");
    printf("*\n");
    device_table->table = device_table->start;
    printf("Start *: %d\n", (int)device_table->start);
    printf("Current *: %d\n", (int)device_table->current_entry);
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        printf("Device table *: %d\t", (int)&device_table->table[i]);
        char* type_str = convert_device_type_str(device_table->table[i].device_type);
        printf("Device ID: %d | Device Type: %s\n", device_table->table[i].device_id, type_str);
    }
}