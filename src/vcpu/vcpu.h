#ifndef _8BITVM_VCPU_H_
#define _8BITVM_VCPU_H_

#include <stdint.h>
#include "../device_table/device_table.h"

#define REG_NUM 16
#define REGISTERS_SIZE sizeof(REGISTER_t) * REG_NUM

#define BIT0 (uint8_t)(0b00000001)
#define BIT1 (uint8_t)(0b00000010)
#define BIT2 (uint8_t)(0b00000100)
#define BIT3 (uint8_t)(0b00001000)
#define BIT4 (uint8_t)(0b00010000)
#define BIT5 (uint8_t)(0b00100000)
#define BIT6 (uint8_t)(0b01000000)
#define BIT7 (uint8_t)(0b10000000)
#define BIT(x) ((uint8_t)1 << (x))

typedef uint8_t REGISTER_t;

struct REGISTERS
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
    REGISTER_t PC;
    REGISTER_t LR;
    REGISTER_t SP;
};
typedef struct REGISTERS REGISTERS_t;

struct vCPU {
    DEVICE_TABLE_ENTRY_t device_info;
    REGISTERS_t registers;
};
typedef struct vCPU vCPU_t;

int vcpu_init(vCPU_t* vcpu);

void registers_dump(vCPU_t *vcpu);

#endif