#ifndef _8BITVM_VCLOCK_H_
#define _8BITVM_VCLOCK_H_

#include <stdint.h>
#include "../device_table/device_table.h"

#define MAX_FREQ 255 // hz

struct vCLOCK {
    DEVICE_TABLE_ENTRY_t device_info;
    uint8_t freq; // max frequency: 255 hz
};
typedef struct vCLOCK vCLOCK_t;

int vclock_init(vCLOCK_t* vclock);

int vclock_start(vCLOCK_t* vclock);

#endif