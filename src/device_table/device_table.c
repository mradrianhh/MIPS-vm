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
    pthread_mutex_init(&device_table.mutex, NULL);

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

/// @brief
/// Returns device_info for device with @param device_id.
/// Assumes only one entry per device_id.
/// TODO: Implement SORT and bubble_search to increase look-up speed.
/// @param device_id
/// @return
/// Creates a DEVICE_TABLE_ENTRY_t* ptr, and returns DEVICE_TABLE_READ_RETURN_t which contains the pointer to the array/entry and the array size.
/// If not found, returns NULL.
DEVICE_TABLE_READ_RETURN_t *device_table_read_id(uint8_t device_id)
{
    DEVICE_TABLE_READ_RETURN_t *ret;
    ret->array_size=0;
    device_table.table = device_table.start;
    for (int i = 0; &device_table.table[i] != device_table.current_entry; i++)
    {
        if (device_table.table[i].device_id == device_id)
        {
            ret->entry_ptr = &device_table.table[i];
            pthread_mutex_unlock(&device_table.mutex);
            return ret;
            //pthread_exit((void *)ret);
        }
    }
    pthread_mutex_unlock(&device_table.mutex);

    return NULL;
    //pthread_exit((void *)NULL);
}

/// @brief
/// Returns device_info for all devices with @param device_type.
/// TODO: Implement SORT and bubble_search to increase look-up speed.
/// @param device_type
/// @return
/// Creates an array of DEVICE_TABLE_ENTRY_t, and returns DEVICE_TABLE_READ_RETURN_t which contains the pointer to the array/entry and the array size.
/// If not found, returns NULL.
DEVICE_TABLE_READ_RETURN_t *device_table_read_type(uint8_t device_type)
{
    DEVICE_TABLE_READ_RETURN_t *ret;
    DEVICE_TABLE_ENTRY_t *temp_array = malloc(sizeof(DEVICE_TABLE_ENTRY_t) * 256);
    uint8_t actual_amount = 0;
    pthread_mutex_lock(&device_table.mutex);
    device_table.table = device_table.start;
    for (int i = 0; &device_table.table[i] != device_table.current_entry; i++)
    {
        if (device_table.table[i].device_type == device_type)
        {
            temp_array[i] = device_table.table[i];
            actual_amount++;
            pthread_mutex_unlock(&device_table.mutex);
        }
    }
    pthread_mutex_unlock(&device_table.mutex);

    if (actual_amount == 0)
    {
        // If no devices match, return NULL-ptr.
        return NULL;
        //pthread_exit((void *)NULL);
    }
    else
    {
        // Else, resize the array to remove extra padding, and return ptr to resized array.
        temp_array = realloc(temp_array, sizeof(DEVICE_TABLE_ENTRY_t) * actual_amount);
        ret->array_size = actual_amount;
        ret->entry_ptr = temp_array;
        return ret;
        //pthread_exit((void *)ret);
    }
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
}
