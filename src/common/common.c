#include <stdint.h>
#include "common.h"

uint8_t device_id = 1;

void device_table_init(DEVICE_TABLE_t* device_table)
{
    device_table->device_table = malloc(DEVICE_TABLE_SIZE);
    device_table->current_entry = 0;
    memset(device_table->device_table, 0, DEVICE_TABLE_SIZE);
}

void device_table_add(DEVICE_TABLE_t* device_table, DEVICE_TABLE_ENTRY_t entry)
{
    device_table->device_table[device_table->current_entry] = entry;
    device_table->current_entry++;
}
