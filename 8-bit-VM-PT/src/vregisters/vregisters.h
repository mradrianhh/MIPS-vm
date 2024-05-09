#ifndef _8BITVM_VREGISTERS_H_
#define _8BITVM_VREGISTERS_H_

#include <stdint.h>

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

union vCPU_CONTROL_REGISTER
{
    struct
    {
        uint8_t halt_exec : 1;
        uint8_t : 7;
    };
    REGISTER_t control_register;
};
typedef union vCPU_CONTROL_REGISTER vCPU_CONTROL_REGISTER_t;

union GP_REGISTERS
{
    struct
    {
        REGISTER_t R0;
        REGISTER_t R1;
        REGISTER_t R2;
        REGISTER_t R3;
    };
    REGISTER_t registers[4];
};
typedef union GP_REGISTERS GP_REGISTERS_t;

union SPECIAL_REGISTERS
{
    struct
    {
        REGISTER_t PC;
        REGISTER_t LR;
        REGISTER_t SP;
        REGISTER_t IR;
        REGISTER_t MAR;
        REGISTER_t MDR;
        vCPU_CONTROL_REGISTER_t CTL;
    };
    REGISTER_t registers[7];
};
typedef union SPECIAL_REGISTERS SPECIAL_REGISTERS_t;

#endif