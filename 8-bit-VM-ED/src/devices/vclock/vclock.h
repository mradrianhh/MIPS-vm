#ifndef _8BITVM_VCLOCK_H_
#define _8BITVM_VCLOCK_H_

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

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

void vclock_init(uint8_t freq);

void vclock_start();

void vclock_shutdown();

#endif