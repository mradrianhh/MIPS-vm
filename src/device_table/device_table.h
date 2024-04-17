#ifndef _8BITVM_DEVICE_TABLE_H_
#define _8BITVM_DEVICE_TABLE_H_

#include <stdint.h>
#include <pthread.h>

#define MAX_DEVICES 256
#define DEVICE_TABLE_SIZE sizeof(DEVICE_TABLE_ENTRY_t) * MAX_DEVICES

#define DEVICE_TYPE_CPU     (uint8_t)(0x01)
#define DEVICE_TYPE_MEMORY_CONTROLLER  (uint8_t)(0x02)
#define DEVICE_TYPE_CLOCK   (uint8_t)(0x03)

struct DEVICE_TABLE_ENTRY 
{
    pthread_t device_tid;
    uint8_t device_id;
    uint8_t device_type;

};
typedef struct DEVICE_TABLE_ENTRY DEVICE_TABLE_ENTRY_t;

struct DEVICE_TABLE 
{
    DEVICE_TABLE_ENTRY_t* current_entry;
    DEVICE_TABLE_ENTRY_t* start;
    DEVICE_TABLE_ENTRY_t* table;
    pthread_mutex_t mutex;
};
typedef struct DEVICE_TABLE DEVICE_TABLE_t;

struct DEVICE_TABLE_READ_RETURN
{
    DEVICE_TABLE_ENTRY_t* entry_ptr;
    uint8_t array_size;
};
typedef struct DEVICE_TABLE_READ_RETURN DEVICE_TABLE_READ_RETURN_t;

extern DEVICE_TABLE_t device_table;

extern uint8_t device_id_count;

int device_table_init();

void device_table_add(DEVICE_TABLE_ENTRY_t entry);

void device_table_sort();

DEVICE_TABLE_READ_RETURN_t* device_table_read_id(uint8_t device_id);

DEVICE_TABLE_READ_RETURN_t* device_table_read_type(uint8_t device_type);

char* convert_device_type_str(uint8_t device_type);

void device_table_dump();

#endif