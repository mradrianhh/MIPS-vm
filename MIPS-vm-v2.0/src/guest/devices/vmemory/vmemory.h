#ifndef _MIPSVM_GUEST_DEVICES_VMEMORY_H_
#define _MIPSVM_GUEST_DEVICES_VMEMORY_H_

#include <stdint.h>

#include "guest/common/device_table/device_table.h"
#include "guest/common/logger/logger.h"

#define VMEMORY_SIZE                 sizeof(uint8_t) * 1000000000

typedef struct  
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    uint8_t* start;
    size_t size;
} vMemory_t;

typedef union 
{
    uint8_t bytes[4];
    uint32_t word;
} Word_t;


// Initializes virtual vmemory.
void vmemory_init();

void vmemory_shutdown();

uint32_t vmemory_read_word(uint32_t address);

uint8_t vmemory_read_byte(uint32_t address);

void vmemory_write_word(uint32_t word, uint32_t address);

void vmemory_write_byte(uint8_t byte, uint32_t address);

vMemory_t *vmemory_get_memory_ref();

#endif
