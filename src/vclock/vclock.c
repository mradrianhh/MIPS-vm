#include <stdio.h>

#include "vclock.h"
#include "../common/common.h"

int vclock_init(vCLOCK_t *vclock)
{
    vclock->device_id = device_id_count++;
    vclock->device_type = DEVICE_TYPE_CLOCK;
    printf("Device vCLOCK(%d) initializing...\n", vclock->device_id);
    return 0;
}
