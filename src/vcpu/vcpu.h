#ifndef _8BITVM_VCPU_H_
#define _8BITVM_VCPU_H_

#include <stdint.h>
#include "device_table/device_table.h"
#include "vmemory_controller.h"
#include "logger/logger.h"
#include "vregisters.h"
#include "vcontrol_unit.h"
#include "vcu_decoding.h"

struct vCPU
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    GP_REGISTERS_t gp_registers;
    SPECIAL_REGISTERS_t special_registers;
    vCONTROL_UNIT_t vcontrol_unit;
    vMEMORY_CONTROLLER_t vmemory_controller;
    vCPU_INSN_DECODING_t insn_decoding;
    vCPU_INSN_DECODING_MAP_t decoding_map;
};
typedef struct vCPU vCPU_t;

int vcpu_init(vCPU_t *vcpu);

int vcpu_start(vCPU_t *vcpu);

int vcpu_shutdown(vCPU_t *vcpu);

void registers_dump(vCPU_t *vcpu);

#endif