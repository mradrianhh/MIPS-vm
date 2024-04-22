#ifndef _8BITVM_VCONTROL_UNIT_H_
#define _8BITVM_VCONTROL_UNIT_H_

#include "device_table/device_table.h"
#include "logger/logger.h"
#include "vregisters.h"
#include "vmemory_controller.h"

#define OPCODE_MASK (uint8_t)(0b11110000)

struct CU_REGISTERS
{
    REGISTER_t PC;
    REGISTER_t LR;
    REGISTER_t SP;
    REGISTER_t IR;
};
typedef struct CU_REGISTERS CU_REGISTERS_t;

struct vCONTROL_UNIT
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    CU_REGISTERS_t cu_registers;
};
typedef struct vCONTROL_UNIT vCONTROL_UNIT_t;

int vcontrol_unit_init(vCONTROL_UNIT_t *vcu);

int vcontrol_unit_shutdown(vCONTROL_UNIT_t *vcu);

void fetch_instruction(vCONTROL_UNIT_t *vcu, vMEMORY_CONTROLLER_t *controller);

void decode_instruction(vCONTROL_UNIT_t *vcu);

void execute_instruction(vCONTROL_UNIT_t *vcu);

#endif