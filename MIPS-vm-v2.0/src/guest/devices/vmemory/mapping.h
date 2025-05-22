#ifndef _MIPSVM_GUEST_DEVICES_VMEMORY_MAPPING_H_
#define _MIPSVM_GUEST_DEVICES_VMEMORY_MAPPING_H_

#include <stdint.h>

// Options.
#define TLB_PRESENT (0)

// Virtual memory map constants.
#define KUSEG_START_VADDR       (0x00000000)
#define KUSEG_END_VADDR         (0x7FFFFFFF)

#define KSEG0_START_VADDR       (0x80000000)
#define KSEG0_END_VADDR         (0x9FFFFFFF)
#define KSEG0_TRANSFORM_MASK    (0x7FFFFFFF)

#define KSEG1_START_VADDR       (0xA0000000)
#define KSEG1_END_VADDR         (0xBFFFFFFF)
#define KSEG1_TRANSFORM_MASK    (0x1FFFFFFF)

#define KSEG2_START_VADDR       (0xC0000000)
#define KSEG2_END_VADDR         (0xFFFFFFFF)

#define RESET_VECTOR_VADDR      (uint32_t)(0xBFC00000)
#define RESET_VECTOR_PADDR      (uint32_t)(0x1FC00000)

typedef enum {
    ACCESS_MODE_UNDEFINED = 0,
    ACCESS_MODE_STORE,
    ACCESS_MODE_LOAD,
} ACCESS_MODE;

uint32_t address_translation(uint32_t vaddr, ACCESS_MODE access_mode, uint8_t *cacheable);

#endif
