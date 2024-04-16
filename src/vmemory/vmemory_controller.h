#ifndef _8BITVM_VMEMORY_CONTROLLER_H_
#define _8BITVM_VMEMORY_CONTROLLER_H_

#include <stdint.h>

#include "vmemory.h"
#include "../device_table/device_table.h"

struct vMEMORY_CONTROLLER 
{
    DEVICE_TABLE_ENTRY_t device_info;
    vMEMORY_t vmemory;
};
typedef struct vMEMORY_CONTROLLER vMEMORY_CONTROLLER_t;

int vmemory_controller_init(vMEMORY_CONTROLLER_t* controller);

void memory_dump(vMEMORY_CONTROLLER_t *controller);

#endif