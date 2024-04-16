#ifndef _8BITVM_DEVICE_TABLE_H_
#define _8BITVM_DEVICE_TABLE_H_

#include <stdint.h>

#define MAX_DEVICES 256
#define DEVICE_TABLE_SIZE sizeof(DEVICE_TABLE_ENTRY_t) * MAX_DEVICES

#define DEVICE_TYPE_CPU     (uint8_t)(0x01)
#define DEVICE_TYPE_MEMORY_CONTROLLER  (uint8_t)(0x02)
#define DEVICE_TYPE_CLOCK   (uint8_t)(0x03)

struct DEVICE_TABLE_ENTRY 
{
    uint8_t device_id;
    uint8_t device_type;
};
typedef struct DEVICE_TABLE_ENTRY DEVICE_TABLE_ENTRY_t;

struct DEVICE_TABLE 
{
    DEVICE_TABLE_ENTRY_t* current_entry;
    DEVICE_TABLE_ENTRY_t* start;
    DEVICE_TABLE_ENTRY_t* table;
};
typedef struct DEVICE_TABLE DEVICE_TABLE_t;

extern DEVICE_TABLE_t device_table;

extern uint8_t device_id_count;

int device_table_init();

void device_table_add(DEVICE_TABLE_ENTRY_t entry);

void device_table_sort();

char* convert_device_type_str(uint8_t device_type);

void device_table_dump();

#endif