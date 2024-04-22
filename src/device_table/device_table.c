#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "device_table.h"

static uint8_t device_id_count = 1;
static DEVICE_TABLE_t device_table;

int device_table_init()
{
    device_table.start = malloc(DEVICE_TABLE_SIZE);
    device_table.next = device_table.start;
    memset(device_table.start, 0, DEVICE_TABLE_SIZE);
    pthread_mutex_init(&device_table.mutex, NULL);

    return 0;
}

DEVICE_TABLE_ENTRY_t *device_table_add(uint8_t device_type)
{
    device_table.next->device_id = device_id_count++;
    device_table.next->device_type = device_type;
    device_table.next->device_running = DEVICE_STOPPED;
    DEVICE_TABLE_ENTRY_t *ret = device_table.next;
    device_table.next++;
    return ret;
}

/// @brief
/// Returns device_info for device with @param device_id.
/// Assumes only one entry per device_id.
/// TODO: Implement SORT and bubble_search to increase look-up speed.
/// @param device_id
/// @return
/// Creates a DEVICE_TABLE_ENTRY_t* ptr, and returns DEVICE_TABLE_READ_RETURN_t which contains the pointer to the array/entry and the array size.
/// If not found, returns NULL.
DEVICE_TABLE_READ_RETURN_t device_table_read_id(uint8_t device_id)
{
    DEVICE_TABLE_READ_RETURN_t ret = {
        .array_size = 0,
        .entry_ptr = NULL,
    };

    pthread_mutex_lock(&device_table.mutex);
    for (int i = 0; &device_table.start[i] != device_table.next; i++)
    {
        if (device_table.next->device_id == device_id)
        {
            ret.array_size = 1;
            ret.entry_ptr = device_table.next;
            pthread_mutex_unlock(&device_table.mutex);
            return ret;
        }
    }
    pthread_mutex_unlock(&device_table.mutex);

    return ret;
}

/// @brief
/// Returns device_info for all devices with @param device_type.
/// TODO: Implement SORT and bubble_search to increase look-up speed.
/// @param device_type
/// @return
/// Creates an array of DEVICE_TABLE_ENTRY_t, and returns DEVICE_TABLE_READ_RETURN_t which contains the pointer to the array/entry and the array size.
/// If not found, returns NULL.
DEVICE_TABLE_READ_RETURN_t device_table_read_type(uint8_t device_type)
{
    DEVICE_TABLE_READ_RETURN_t ret = {
        .array_size = 0,
        .entry_ptr = NULL,
    };
    DEVICE_TABLE_ENTRY_t *found_entries = malloc(sizeof(DEVICE_TABLE_ENTRY_t) * 256);
    uint8_t num_found_entries = 0;

    pthread_mutex_lock(&device_table.mutex);
    for (int i = 0; &device_table.start[i] != device_table.next; i++)
    {
        if (device_table.start[i].device_type == device_type)
        {
            found_entries[num_found_entries] = device_table.start[i];
            num_found_entries++;
        }
    }
    pthread_mutex_unlock(&device_table.mutex);

    if (num_found_entries == 0)
    {
        return ret;
    }
    else
    {
        // Else, resize the array to remove extra padding, and return ptr to resized array.
        found_entries = realloc(found_entries, sizeof(DEVICE_TABLE_ENTRY_t) * num_found_entries);
        ret.array_size = num_found_entries;
        ret.entry_ptr = found_entries;
        return ret;
    }
}

char *convert_device_type_str(uint8_t device_type)
{
    char *ret;
    switch (device_type)
    {
    case DEVICE_TYPE_CPU:
        ret = "vCPU";
        break;
    case DEVICE_TYPE_MEMORY_CONTROLLER:
        ret = "vMEMORY_CONTROLLER";
        break;
    case DEVICE_TYPE_CLOCK:
        ret = "vCLOCK";
        break;
    case DEVICE_TYPE_MEMORY:
        ret = "vMEMORY";
        break;
    case DEVICE_TYPE_CONTROL_UNIT:
        ret = "vCONTROL_UNIT";
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

    pthread_mutex_lock(&device_table.mutex);
    for (int i = 0; &device_table.start[i] != device_table.next; i++)
    {
        printf("[%p] : { Device ID: %d Device Type: %s Running: %d } \n", (void *)&device_table.start[i], device_table.start[i].device_id, convert_device_type_str(device_table.start[i].device_type), device_table.start[i].device_running);
    }
    pthread_mutex_unlock(&device_table.mutex);

    char input;
    scanf("Press any key to continue. . . %c", &input);
}
