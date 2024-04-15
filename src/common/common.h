#ifndef _8BITVM_COMMON_H_
#define _8BITVM_COMMON_H_

#include <stdint.h>

#define MAX_DEVICES 256
#define DEVICE_TABLE_SIZE sizeof(DEVICE_TABLE_ENTRY_t) * MAX_DEVICES

extern uint8_t device_id_count;

typedef enum 
{
    DEVICE_TYPE_CPU = 1,
    DEVICE_TYPE_MEMORY = 2,
    DEVICE_TYPE_CLOCK = 3,
} DEVICE_TYPE_t;

struct DEVICE_TABLE 
{
    uint8_t current_entry;
    DEVICE_TABLE_ENTRY_t* device_table;
};
typedef struct DEVICE_TABLE DEVICE_TABLE_t;

struct DEVICE_TABLE_ENTRY 
{
    uint8_t device_id;
    DEVICE_TYPE_t device_type;
};
typedef struct DEVICE_TABLE_ENTRY DEVICE_TABLE_ENTRY_t;

void device_table_init(DEVICE_TABLE_t* device_table);

void device_table_add(DEVICE_TABLE_t* device_table, DEVICE_TABLE_ENTRY_t entry);

void device_table_sort(DEVICE_TABLE_t* device_table);

#endif