#ifndef _8BITVM_VCPU_DECODER_H_
#define _8BITVM_VCPU_DECODER_H_

#include <stdint.h>
#include "vcpu_registers.h"

#define OPCODE_MASK         (uint32_t)(0xFC000000) // (0b1111 1100 0000 0000 0000 0000 0000 0000)
#define RS_MASK             (uint32_t)(0x03E00000) // (0b0000 0011 1110 0000 0000 0000 0000 0000)
#define RT_MASK             (uint32_t)(0x001F0000) // (0b0000 0000 0001 1111 0000 0000 0000 0000)
#define RD_MASK             (uint32_t)(0x0000F800) // (0b0000 0000 0000 0000 1111 1000 0000 0000)
#define SHAMT_MASK          (uint32_t)(0x000007C0) // (0b0000 0000 0000 0000 0000 0111 1100 0000)
#define FUNCT_MASK          (uint32_t)(0x0000003F) // (0b0000 0000 0000 0000 0000 0000 0011 1111)
#define IMMED_MASK          (uint32_t)(0x0000FFFF) // (0b0000 0000 0000 0000 1111 1111 1111 1111)
#define ADDRESS_MASK        (uint32_t)(0x03FFFFFF) // (0b0000 0011 1111 1111 1111 1111 1111 1111)
#define OPCODE_MAX_RANGE    (uint32_t)(0x0000003F) // (0b0011 1111) = 64

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
typedef vCPU_INSN_t (*vcpu_insn_decode_t)(uint32_t instruction);

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
    };
    vcpu_insn_decode_t decoders[OPCODE_MAX_RANGE];
};

union vCPU_INSN_EXECUTER_MAP
{
    struct
    {
        vcpu_insn_execute_t nop;
    };
    vcpu_insn_execute_t executers[OPCODE_MAX_RANGE];
};

enum vCPU_INSN_TYPE
{
    ADD = 0,
    ADDU,
    SUB,
    SUBU,
    MUL, 
    MULU,
    DIV,
    DIVU,
    SLT,
    SLTU,
    AND,
    OR,
    XOR,
    NOR,
    ADDI,
    ADDIU,
    SLTI,
    SLTIU,
    ANDI,
    ORI,
    XORI,
    LW,
    SW,
    LBU,
    LB,
    SB,
    LUI,
    BEQ,
    BNE,
    BLEZ,
    BGTZ,
    BLTZ,
    J,
    JAL,
    JR,
    JALR,
    NOP,
    MFHI,
    MFLO,
    // TODO: Floating point and exception handling.
};

#endif