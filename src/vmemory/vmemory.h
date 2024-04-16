#ifndef _8BITVM_VMEMORY_H_
#define _8BITVM_VMEMORY_H_

#include <stdint.h>

#define PAGE_NUM 256
#define MEMORY_SIZE sizeof(PAGE_t) * PAGE_NUM

typedef uint8_t PAGE_t;

struct vMEMORY 
{
    PAGE_t* start;
    PAGE_t* memory;
};
typedef struct vMEMORY vMEMORY_t;

// Initializes virtual memory.
int vmemory_init(vMEMORY_t* vmemory);

// Loads virtual memory with example data.
void vmemory_example_load(vMEMORY_t* vmemory);

#endif