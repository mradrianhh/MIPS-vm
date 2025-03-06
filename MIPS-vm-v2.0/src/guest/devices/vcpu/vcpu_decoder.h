#ifndef _MIPSVM_GUEST_DEVICES_VCPU_DECODER_H_
#define _MIPSVM_GUEST_DEVICES_VCPU_DECODER_H_

#include <stdint.h>
#include "vcpu_registers.h"

#define OPCODE_MASK         (uint32_t)(0xFC000000) // (0b1111 1100 0000 0000 0000 0000 0000 0000)
#define OPCODE_OFFSET       (uint8_t)(0x1A)        // (26) 
#define RS_MASK             (uint32_t)(0x03E00000) // (0b0000 0011 1110 0000 0000 0000 0000 0000)
#define RS_OFFSET           (uint32_t)(0x15)       // (21)
#define RT_MASK             (uint32_t)(0x001F0000) // (0b0000 0000 0001 1111 0000 0000 0000 0000)
#define RT_OFFSET           (uint8_t)(0x10)        // (16)
#define RD_MASK             (uint32_t)(0x0000F800) // (0b0000 0000 0000 0000 1111 1000 0000 0000)
#define RD_OFFSET           (uint8_t)(0x0B)        // (11)
#define SHAMT_MASK          (uint32_t)(0x000007C0) // (0b0000 0000 0000 0000 0000 0111 1100 0000)
#define SHAMT_OFFSET        (uint8_t)(0x06)        // (6)
#define FUNCT_MASK          (uint32_t)(0x0000003F) // (0b0000 0000 0000 0000 0000 0000 0011 1111)
#define FUNCT_OFFSET        (uint8_t)(0x00)        // (0)
#define IMMED_MASK          (uint32_t)(0x0000FFFF) // (0b0000 0000 0000 0000 1111 1111 1111 1111)
#define IMMED_OFFSET        (uint8_t)(0x00)        // (0)
#define ADDRESS_MASK        (uint32_t)(0x03FFFFFF) // (0b0000 0011 1111 1111 1111 1111 1111 1111)
#define ADDRESS_OFFSET      (uint8_t)(0x00)        // (0)
#define OPCODE_MAX_RANGE    (uint32_t)(0x0000003F) // (0b0011 1111) = 64
#define FUNCT_MAX_RANGE     (uint32_t)(0x0000003F) // (0b0011 1111) = 64
#define INSN15_MASK         (uint16_t)(0x8000)     // (0b1000 0000 0000 0000)
#define WORD_HIGH_VALUES    (uint32_t)(0xFFFFFFFF) // (0b1111 1111 1111 1111 1111 1111 1111 1111)
#define WORD_LOW_VALUES     (uint32_t)(0x00000000) // (0b0000 0000 0000 0000 0000 0000 0000 0000)
#define PC_31_28_MASK       (uint32_t)(0xF0000000) // (0b1111 0000 0000 0000 0000 0000 0000 0000)
    
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
void vcpu_decoder_init(REGISTER_t *pc_ref, vcpu_insn_execute_t *executers, vcpu_insn_execute_t *rformat_executers);

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
    uint8_t rs;
    uint8_t rt;
    uint8_t rd;
    uint8_t shamt;
    uint16_t immed;
    uint32_t address;
};

struct vCPU_INSN
{
    vcpu_insn_execute_t execute;
    vCPU_INSN_OPERANDS_t operands;
};

enum vCPU_INSN_OPCODE_TYPE
{
    RFORMAT = 0,
    J = 2,
    JAL = 3,
    BEQ = 4,
    ADDIU = 9,
    ORI = 13,
    LUI = 15,
    LW = 35,
    SW = 43,
    SUB,
    SUBU,
    MUL, 
    MULU,
    DIV,
    DIVU,
    SLT,
    SLTU,
    AND,
    XOR,
    NOR,
    ADDI,
    SLTI,
    SLTIU,
    ANDI,
    XORI,
    LBU,
    LB,
    SB,
    BNE,
    BLEZ,
    BGTZ,
    BLTZ,
    MFHI,
    MFLO,
    // TODO: Floating point and exception handling.
};

enum vCPU_INSN_FUNCT_TYPE
{
    NOP = 0,
    JR = 8,
    JALR = 9,
    ADD = 32,
    ADDU = 33,
    OR = 37,
};

#endif
