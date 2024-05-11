#ifndef _8BITVM_VCLOCK_H_
#define _8BITVM_VCLOCK_H_
#define _POSIX_C_SOURCE 199309L

#include <stdint.h>
#include <time.h>

#include "internal/device_table/device_table.h"
#include "internal/logger/logger.h"

#define MAX_FREQ 255 // hz

struct vCLOCK {
    LOGGER_t logger;
    DEVICE_TABLE_ENTRY_t *device_info;
};
typedef struct vCLOCK vCLOCK_t;

int vclock_init(vCLOCK_t* vclock);

int vclock_start(vCLOCK_t* vclock);

int vclock_shutdown(vCLOCK_t *vclock);

#endif