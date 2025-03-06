#ifndef _MIPSVM_GUEST_DEVICES_VMEMORY_H_
#define _MIPSVM_GUEST_DEVICES_VMEMORY_H_

#include <stdint.h>

#include "guest/common/device_table/device_table.h"
#include "guest/common/logger/logger.h"

#define PAGE_NUM                    4000000000
#define MEMORY_SIZE                 sizeof(uint8_t) * PAGE_NUM
#define KSEG1_PGM_TO_PHYS_MASK      (uint32_t)(0x1FFFFFFF) // (0b0001 1111 1111 1111 1111 1111 1111 1111)
#define KSEG1_PGM_START_VADDR       (uint32_t)(0xA0000000)
#define KSEG1_PGM_END_VADDR         (uint32_t)(0xBFFFFFFF)
#define RESET_VECTOR_VADDR          (uint32_t)(0xBFC00000)

typedef struct  
{
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
    uint8_t* start;
} vMEMORY_t;

typedef union 
{
    uint8_t bytes[4];
    uint32_t word;
} Word_t;


// Initializes virtual memory.
void vmemory_init();

void vmemory_shutdown();

uint32_t vmemory_read_word(uint32_t address);

uint8_t vmemory_read_byte(uint32_t address);

void vmemory_write_word(uint32_t word, uint32_t address);

void vmemory_write_byte(uint8_t byte, uint32_t address);

uint32_t vmemory_map_pgm_to_phys_addr(uint32_t pgm_address);

uint8_t *vmemory_get_memory_ref();

#endif
