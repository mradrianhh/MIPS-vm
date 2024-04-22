#ifndef _8BITVM_VCU_DECODING_H_
#define _8BITVM_VCU_DECODING_H_

#include <stdint.h>

#define OPCODE_MASK         (uint8_t)(0b11110000)
#define DEST_MASK           (uint8_t)(0b00001100)
#define SOURCE_MASK         (uint8_t)(0b00000011)
#define OPCODE_MAX_RANGE    (uint8_t)(0b1111)

struct vCPU_INSN_OPERANDS
{
    uint8_t operand1;
    uint8_t operand2;
    uint8_t operand3;
};
typedef struct vCPU_INSN_OPERANDS vCPU_INSN_OPERANDS_t;

typedef void(*_vcpu_insn_func_t)(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers, GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands);

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

#endif