#ifndef _8BITVM_VCPU_H_
#define _8BITVM_VCPU_H_

#include "internal/device_table/device_table.h"
#include "internal/logger/logger.h"
#include "vcpu_state.h"

typedef struct vCPU vCPU_t;

void vcpu_init(const void *args);

void vcpu_update(const void *args);

void vcpu_shutdown(const void *args);

void vcpu_dump_state();

struct vCPU
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    vCPU_state_t state;
};

#endif