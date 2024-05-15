#ifndef _MIPSVM_VMEMORY_H_
#define _MIPSVM_VMEMORY_H_

#include <stdint.h>

#include "internal/device_table/device_table.h"
#include "internal/logger/logger.h"

#define PAGE_NUM                    4000000000
#define MEMORY_SIZE                 sizeof(uint8_t) * PAGE_NUM

typedef struct  
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    uint8_t* start;
} vMEMORY_t;

typedef union 
{
    uint8_t bytes[4];
    uint32_t word;
} Word_t;


// Initializes virtual memory.
void vmemory_init();

void vmemory_shutdown();

Word_t vmemory_fetch_instruction(Word_t address);

uint8_t *vmemory_get_memory_ref();

#endif