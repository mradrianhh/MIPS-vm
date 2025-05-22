#ifndef _MIPSVM_GUEST_DEVICES_VCPU_H_
#define _MIPSVM_GUEST_DEVICES_VCPU_H_

#define ICACHE_SIZE 8192 // 8kB - 8192 bytes
#define DCACHE_SIZE 8192 // 8kB - 8192 bytes

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

#include "guest/common/device_table/device_table.h"
#include "guest/common/logger/logger.h"
#include "guest/devices/vmemory/vcache.h"

typedef struct vCPU vCPU_t;

typedef void (*vcpu_insn_execute_t)();

#define OPCODE_RANGE (64)
#define INSN_RANGE (26)

#define GPR_COUNT (32)

enum GPRS
{
    REGISTER_ZERO = 0,
    REGISTER_AT = 1,
    REGISTER_V0 = 2,
    REGISTER_V1 = 3,
    REGISTER_A0 = 4,
    REGISTER_A1 = 5,
    REGISTER_A2 = 6,
    REGISTER_A3 = 7,
    REGISTER_T0 = 8,
    REGISTER_T1 = 9,
    REGISTER_T2 = 10,
    REGISTER_T3 = 11,
    REGISTER_T4 = 12,
    REGISTER_T5 = 13,
    REGISTER_T6 = 14,
    REGISTER_T7 = 15,
    REGISTER_S0 = 16,
    REGISTER_S1 = 17,
    REGISTER_S2 = 18,
    REGISTER_S3 = 19,
    REGISTER_S4 = 20,
    REGISTER_S5 = 21,
    REGISTER_S6 = 22,
    REGISTER_S7 = 23,
    REGISTER_T8 = 24,
    REGISTER_T9 = 25,
    REGISTER_K0 = 26,
    REGISTER_K1 = 27,
    REGISTER_GP = 28,
    REGISTER_SP = 29,
    REGISTER_FP = 30,
    REGISTER_RA = 31
};

typedef enum
{
    INSN_FORMAT_UNDEFINED = 0,
    INSN_FORMAT_RFORMAT,
    INSN_FORMAT_IFORMAT,
    INSN_FORMAT_JFORMAT,
} INSN_FORMAT;

typedef enum 
{
    INSN_TYPE_UNDEFINED = 0,
    INSN_TYPE_BRANCH,
    INSN_TYPE_ALU,
    INSN_TYPE_MEM_LOAD,
    INSN_TYPE_MEM_STORE
} INSN_TYPE;

typedef enum
{
    NOP = 0,
    ADD,
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
    NOR,
    XOR,
    ADDI,
    ADDIU,
    JR,
    LUI,
    BEQ,
    J,
    JAL,
    SW,
    LW,
    JALR,
    ORI
} INSN_MNEMONIC;

typedef struct INSN
{
    union
    {
        struct
        {
            uint8_t opcode;
            uint8_t function;
        };
        uint16_t insn_id;
    };
    uint8_t format;
    uint8_t insn_type;
    char mnemonic[6];
} INSN_entry;

void vcpu_init();

void vcpu_reset();

void vcpu_update(const void *args);

void vcpu_shutdown();

uint32_t *vcpu_get_pc_ref();

struct vCPU
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    uint8_t turned_on;
    uint32_t gprs[GPR_COUNT];
    uint32_t pc;
    vCache_t icache;
    vCache_t dcache;
    // Buffers
    struct
    {
        // insn is forwarded through all stages.
        uint32_t insn;
        // pc must be forwarded so it can be properly computed in case of a branch.
        uint32_t pc;
    } IF_ID;
    struct
    {
        // Forwarding
        uint32_t insn;
        // Update in current stage.
        // Used to easy identification of instruction.
        union
        {
            struct
            {
                uint8_t opcode;
                uint8_t function;
            };
            uint16_t insn_id;
        }; 
        uint8_t rs;
        uint8_t rt;
        uint8_t rd;
        uint32_t immed;
        uint32_t pc;
    } ID_EX;
    struct
    {
        // Forwarding.
        uint8_t dest_reg;
        uint32_t insn;
        // Updated in current stage.
        uint32_t result;
        uint32_t store_value;
        uint32_t target_address;
        uint8_t condition_flag;
        INSN_entry insn_info;
    } EX_MEM;
    struct
    {
        // Forwarding.
        uint8_t dest_reg;
        uint32_t insn;
        // This might be the result from the ALU or the result of a memory load.
        uint32_t result;
    } MEM_WB;
};

#endif
