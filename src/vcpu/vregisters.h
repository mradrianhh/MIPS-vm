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

#endif