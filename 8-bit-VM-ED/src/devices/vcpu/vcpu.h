#ifndef _8BITVM_VCPU_H_
#define _8BITVM_VCPU_H_

#include "internal/device_table/device_table.h"
#include "internal/logger/logger.h"

typedef struct vCPU vCPU_t;

void vcpu_init(const void *args);

void vcpu_update(const void *args);

void vcpu_shutdown(const void *args);

struct vCPU
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
};

#endif