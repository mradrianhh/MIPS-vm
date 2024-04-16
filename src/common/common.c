#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

uint8_t device_id_count = 1;

void device_table_init(DEVICE_TABLE_t* device_table)
{
    // Allocate table and point current_entry and start to first element.
    device_table->table = malloc(DEVICE_TABLE_SIZE);
    device_table->current_entry = device_table->table;
    device_table->start = device_table->table;
    memset(device_table->table, 0, DEVICE_TABLE_SIZE);
}

void device_table_add(DEVICE_TABLE_t* device_table, DEVICE_TABLE_ENTRY_t entry)
{
    device_table->table = device_table->current_entry;
    device_table->table->device_id = entry.device_id;
    device_table->table->device_type = entry.device_type;
    device_table->current_entry++;
}

void device_table_sort(DEVICE_TABLE_t *device_table)
{
}

char* convert_device_type_str(uint8_t device_type)
{
    char* ret;
    switch(device_type)
    {
    case DEVICE_TYPE_CPU:
        ret = "CPU";
        break;
    case DEVICE_TYPE_MEMORY:
        ret = "MEMORY";
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
