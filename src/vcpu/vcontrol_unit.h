#ifndef _8BITVM_VCONTROL_UNIT_H_
#define _8BITVM_VCONTROL_UNIT_H_

#include "device_table/device_table.h"
#include "logger/logger.h"
#include "vregisters.h"
#include "vcu_decoding.h"
#include "vmemory_controller.h"

struct vCONTROL_UNIT
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    vCPU_INSN_DECODING_t insn_decoding;
    vCPU_INSN_DECODING_MAP_t decoding_map;
    GP_REGISTERS_t *gp_registers;
    SPECIAL_REGISTERS_t *special_registers;
    vMEMORY_CONTROLLER_t *vmemory_controller;
};
typedef struct vCONTROL_UNIT vCONTROL_UNIT_t;

int vcontrol_unit_init(vCONTROL_UNIT_t *vcu, GP_REGISTERS_t *gp_registers,
                       SPECIAL_REGISTERS_t *special_registers, vMEMORY_CONTROLLER_t *controller);

int vcontrol_unit_shutdown(vCONTROL_UNIT_t *vcu);

void fetch_instruction(vCONTROL_UNIT_t *vcu);

void decode_instruction(vCONTROL_UNIT_t *vcu);

void execute_instruction(vCONTROL_UNIT_t *vcu);

#endif
