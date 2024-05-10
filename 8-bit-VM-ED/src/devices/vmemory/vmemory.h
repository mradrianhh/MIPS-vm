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
int vmemory_init(vMEMORY_t* const vmemory);

int vmemory_start(vMEMORY_t* const vmemory);

int vmemory_shutdown(vMEMORY_t* const vmemory);

void memory_dump(vMEMORY_t* const vmemory);

#endif