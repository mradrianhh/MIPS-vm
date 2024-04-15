#ifndef _8BITVM_VCLOCK_H_
#define _8BITVM_VCLOCK_H_

#include <stdint.h>

#include "../common/common.h"

#define MAX_FREQ 255 // hz

struct vCLOCK {
    uint8_t device_id;
    DEVICE_TYPE_t device_type;
    uint8_t freq; // max frequency: 255 hz
};
typedef struct vCLOCK vCLOCK_t;

int vclock_init(vCLOCK_t* vclock);

#endif