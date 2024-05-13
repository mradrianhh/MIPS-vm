#ifndef _8BITVM_VCPU_DECODER_H_
#define _8BITVM_VCPU_DECODER_H_

#include <stdint.h>
#include "vcpu_registers.h"

#define OPCODE_MASK (uint8_t)(0b11110000)
#define DEST_MASK (uint8_t)(0b00001100)
#define SOURCE_MASK (uint8_t)(0b00000011)
#define OPCODE_MAX_RANGE (uint8_t)(0b1111)

// Summary:
//   Struct with three indexes for the registers to use.
//
// Details:
//   Contains three indexes that shows which registers the
//   decoded function will use.
typedef struct vCPU_INSN_OPERANDS vCPU_INSN_OPERANDS_t;

// Summary:
//   Struct with function pointer and operands.
//
// Details:
//   Contains a pointer to the decoded instruction function
//   and the collection of operands that the function will use.
typedef struct vCPU_INSN vCPU_INSN_t;

// Summary:
//   Union containing all decoders.
//
// Details:
//   Contains all decoders with their entries mapped
//   according to the values in vCPU_INSN_TYPE_t.
//
//   Decoder map is used by vcpu_decoder to map instructions to decoders.
typedef union vCPU_INSN_DECODER_MAP vCPU_INSN_DECODER_MAP_t;

// Summary:
//   Union containing all executers.
//
// Details:
//   Contains all executers with their entries mapped
//   according to the values in vCPU_INSN_TYPE_t.
//
//   Executer map is used by vcpu to implement functions returned by decoders.
typedef union vCPU_INSN_EXECUTER_MAP vCPU_INSN_EXECUTER_MAP_t;

// Summary:
//   Enum of all instructions supported by the CPU.
//
// Details:
//   The values of the enum types are configured so they point to the
//   appropriate entry in the decoder map and the executer map.
typedef enum vCPU_INSN_TYPE vCPU_INSN_TYPE_t;

// Summary:
//   The decoded function to execute.
//
// Params:
//   vCPU_INSN_OPERANDS_t *: operands:
//     A collection of pointers to the registers that the function will use.
typedef void (*vcpu_insn_execute_t)(vCPU_INSN_OPERANDS_t operands);

// Summary:
//   The function used to decode the instruction.
//
// Params:
//   uint8_t: instruction:
//     The instruction to decode.
typedef vCPU_INSN_t (*vcpu_insn_decode_t)(uint8_t instruction);

// Summary:
//   Initializes the decoders.
void vcpu_decoder_init(vCPU_INSN_EXECUTER_MAP_t executer_map);

// Summary:
//   Gets the decoder for the instruction provided.
//
// Details:
//   Maps the instructions opcode to an entry in the decoder-map and returns it.
//
// Params:
//   REGISTER_t: ir:
//     The instruction to decode.
//
// Returns:
//   vcpu_insn_decoder_t:
//     Function used to decode the instruction.
vcpu_insn_decode_t vcpu_decoder_get(REGISTER_t ir);

// Summary:
//   Decodes the instruction provided.
//
// Details:
//   Retrieves the decoder from the decoder-map with vcpu_decoder_get,
//   then executes the decoder on the provided instruction and returns.
//
// Params:
//   REGISTER_t: ir:
//     The instruction to decode.
//
// Returns:
//   vCPU_INSN_t:
//     Structure containing the decoded instruction function and the operands.
vCPU_INSN_t vcpu_decoder_decode(REGISTER_t ir);

struct vCPU_INSN_OPERANDS
{
    uint8_t operand1;
    uint8_t operand2;
    uint8_t operand3;
};

struct vCPU_INSN
{
    REGISTER_t ir;
    vcpu_insn_execute_t execute;
    vCPU_INSN_OPERANDS_t operands;
};

union vCPU_INSN_DECODER_MAP
{
    struct
    {
        vcpu_insn_decode_t nop;
        vcpu_insn_decode_t add;
        vcpu_insn_decode_t mov;
        vcpu_insn_decode_t ldr;
        vcpu_insn_decode_t str;
        vcpu_insn_decode_t _buf[12];
    };
    vcpu_insn_decode_t decoders[OPCODE_MAX_RANGE];
};

union vCPU_INSN_EXECUTER_MAP
{
    struct
    {
        vcpu_insn_execute_t nop;
        vcpu_insn_execute_t add;
        vcpu_insn_execute_t mov;
        vcpu_insn_execute_t ldr;
        vcpu_insn_execute_t str;
        vcpu_insn_execute_t _buf[12];
    };
    vcpu_insn_execute_t executers[OPCODE_MAX_RANGE];
};

enum vCPU_INSN_TYPE
{
    NOP = 0,
    ADD,
    MOV,
    LDR,
    STR,
};

#endif