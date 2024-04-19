#ifndef _8BITVM_VMEMORY_CONTROLLER_H_
#define _8BITVM_VMEMORY_CONTROLLER_H_

#include <stdint.h>

#include "vmemory.h"
#include "device_table/device_table.h"
#include "logger/logger.h"

struct vMEMORY_CONTROLLER 
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
};
typedef struct vMEMORY_CONTROLLER vMEMORY_CONTROLLER_t;

int vmemory_controller_init(vMEMORY_CONTROLLER_t* controller);

int vmemory_controller_shutdown(vMEMORY_CONTROLLER_t* controller);

int vmemory_controller_fetch_page(vMEMORY_CONTROLLER_t *controller, uint8_t address, PAGE_t *page);

#endif