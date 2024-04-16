#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "device_table.h"

uint8_t device_id_count = 1;
DEVICE_TABLE_t device_table;

int device_table_init()
{
    // Allocate table and point current_entry and start to first element.
    printf("DEVICE_TABLE initializing...\n");
    device_table.table = malloc(DEVICE_TABLE_SIZE);
    device_table.current_entry = device_table.table;
    device_table.start = device_table.table;
    memset(device_table.table, 0, DEVICE_TABLE_SIZE);

    return 0;
}

void device_table_add(DEVICE_TABLE_ENTRY_t entry)
{
    device_table.table = device_table.current_entry;
    device_table.table->device_id = entry.device_id;
    device_table.table->device_type = entry.device_type;
    device_table.current_entry++;
}

void device_table_sort()
{
}

char *convert_device_type_str(uint8_t device_type)
{
    char *ret;
    switch (device_type)
    {
    case DEVICE_TYPE_CPU:
        ret = "CPU";
        break;
    case DEVICE_TYPE_MEMORY_CONTROLLER:
        ret = "MEMORY_CONTROLLER";
        break;
    case DEVICE_TYPE_CLOCK:
        ret = "CLOCK";
        break;
    default:
        ret = "";
        break;
    }
    return ret;
}

void device_table_dump()
{
    printf("*\n");
    printf("* Device Table Dump\n");
    printf("*\n");
    device_table.table = device_table.start;
    printf("Start *: %d\n", (int)device_table.start);
    printf("Current *: %d\n", (int)device_table.current_entry);

    for (int i = 0; &device_table.table[i] != device_table.current_entry; i++)
    {
        printf("Device table *: %d\t", (int)&device_table.table[i]);
        char *type_str = convert_device_type_str(device_table.table[i].device_type);
        printf("Device ID: %d | Device Type: %s\n", device_table.table[i].device_id, type_str);
    }

    /*
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        printf("Device table *: %d\t", (int)&device_table.table[i]);
        char* type_str = convert_device_type_str(device_table.table[i].device_type);
        printf("Device ID: %d | Device Type: %s\n", device_table.table[i].device_id, type_str);
    }
    */
}
