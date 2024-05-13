#ifndef _8BITVM_VMEMORY_H_
#define _8BITVM_VMEMORY_H_

#include <stdint.h>

#include "internal/device_table/device_table.h"
#include "internal/logger/logger.h"

#define PAGE_NUM                    256
#define MEMORY_SIZE                 sizeof(PAGE_t) * PAGE_NUM

typedef uint8_t PAGE_t;

struct vMEMORY 
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    PAGE_t* start;
};
typedef struct vMEMORY vMEMORY_t;

// Initializes virtual memory.
void vmemory_init();

void vmemory_shutdown();

PAGE_t vmemory_fetch(uint8_t address);

void vmemory_write(uint8_t address, PAGE_t data);

#endif