#ifndef _MIPSVM_GUEST_DEVICES_VCPU_H_
#define _MIPSVM_GUEST_DEVICES_VCPU_H_

#include "guest/common/device_table/device_table.h"
#include "guest/common/logger/logger.h"
#include "vcpu_state.h"

typedef struct vCPU vCPU_t;

void vcpu_init();

void vcpu_update(const void *args);

void vcpu_shutdown();

void vcpu_dump_state();

uint32_t *vcpu_get_pc_ref();

struct vCPU
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    vCPU_state_t state;
};

#endif