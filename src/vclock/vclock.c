#include <stdio.h>

#include "vclock.h"
#include "../device_table/device_table.h"

int vclock_init(vCLOCK_t *vclock)
{
    vclock->device_info.device_id = device_id_count++;
    vclock->device_info.device_type = DEVICE_TYPE_CLOCK;
    device_table_add(vclock->device_info);
    printf("Device vCLOCK(%d) initializing...\n", vclock->device_info.device_id);
    return 0;
}
