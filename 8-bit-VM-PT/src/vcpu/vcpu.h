#ifndef _8BITVM_VCPU_H_
#define _8BITVM_VCPU_H_

#include <stdint.h>

#include "device_table/device_table.h"
#include "vmemory_controller/vmemory_controller.h"
#include "logger/logger.h"
#include "vregisters/vregisters.h"
#include "vnvic/interrupt_event.h"

#define OPCODE_MASK         (uint8_t)(0b11110000)
#define DEST_MASK           (uint8_t)(0b00001100)
#define SOURCE_MASK         (uint8_t)(0b00000011)
#define OPCODE_MAX_RANGE    (uint8_t)(0b1111)

typedef struct vCPU vCPU_t;

struct vCPU_INSN_OPERANDS
{
    uint8_t operand1;
    uint8_t operand2;
    uint8_t operand3;
};
typedef struct vCPU_INSN_OPERANDS vCPU_INSN_OPERANDS_t;

typedef void(*_vcpu_insn_func_t)(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands);

struct vCPU_INSN_DECODING
{
    _vcpu_insn_func_t opcode_func;
    vCPU_INSN_OPERANDS_t operands;

};
typedef struct vCPU_INSN_DECODING vCPU_INSN_DECODING_t;

typedef void(*_vcpu_insn_dec_func_t)(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);

union vCPU_INSN_DECODING_MAP
{
    struct
    {
        _vcpu_insn_dec_func_t add;
        _vcpu_insn_dec_func_t mov;
        _vcpu_insn_dec_func_t ldr;
        _vcpu_insn_dec_func_t str;
        _vcpu_insn_dec_func_t hlt;
        _vcpu_insn_dec_func_t _buf[11];
    };
    _vcpu_insn_dec_func_t decoding_functions[OPCODE_MAX_RANGE];
};
typedef union vCPU_INSN_DECODING_MAP vCPU_INSN_DECODING_MAP_t;

struct vCPU
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    GP_REGISTERS_t gp_registers;
    SPECIAL_REGISTERS_t special_registers;
    vMEMORY_CONTROLLER_t vmemory_controller;
    InterruptEvent_t *interrupt_event;
    vCPU_INSN_DECODING_t insn_decoding;
    vCPU_INSN_DECODING_MAP_t decoding_map;
};

int vcpu_init(vCPU_t *vcpu);

int vcpu_start(vCPU_t *vcpu);

int vcpu_shutdown(vCPU_t *vcpu);

void registers_dump(vCPU_t *vcpu);

#endif