#ifndef _8BITVM_VCPU_H_
#define _8BITVM_VCPU_H_

#include <stdint.h>
#include "device_table/device_table.h"
#include "vmemory_controller.h"
#include "logger/logger.h"
#include "vregisters.h"
#include "vcontrol_unit.h"

struct GP_REGISTERS
{
    REGISTER_t R0;
    REGISTER_t R1;
    REGISTER_t R2;
    REGISTER_t R3;
    REGISTER_t R4;
    REGISTER_t R5;
    REGISTER_t R6;
    REGISTER_t R7;
    REGISTER_t R8;
    REGISTER_t R9;
    REGISTER_t R10;
    REGISTER_t R11;
    REGISTER_t R12;
};
typedef struct GP_REGISTERS GP_REGISTERS_t;

struct vCPU
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    GP_REGISTERS_t gp_registers;
    REGISTER_t ACC;
    vCONTROL_UNIT_t vcontrol_unit;
    vMEMORY_CONTROLLER_t vmemory_controller;
};
typedef struct vCPU vCPU_t;

int vcpu_init(vCPU_t *vcpu);

int vcpu_start(vCPU_t *vcpu);

int vcpu_shutdown(vCPU_t *vcpu);

void registers_dump(vCPU_t *vcpu);

#endif