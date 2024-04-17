#include <stdio.h>
#include <unistd.h>

#include "vclock.h"
#include "../device_table/device_table.h"
#include "vsysbus/vsysbus.h"

static void *vclock_loop(void *vargp);

int vclock_init(vCLOCK_t *vclock)
{
    vclock->device_info.device_id = device_id_count++;
    vclock->device_info.device_type = DEVICE_TYPE_CLOCK;
    device_table_add(vclock->device_info);
    printf("Device vCLOCK(%d) initializing...\n", vclock->device_info.device_id);
    return 0;
}

int vclock_start(vCLOCK_t *vclock)
{
    printf("Device vCLOCK(%d) starting...\n", vclock->device_info.device_id);

    pthread_create(&(vclock->device_info.device_tid), NULL, vclock_loop, (void *)vclock);

    return 0;
}

static void *vclock_loop(void *vargp)
{
    vCLOCK_t *vclock = (vCLOCK_t *)vargp;
    vSYSBUS_PACKET_t packet = {
        .device_id = vclock->device_info.device_id,
        .packet = vclock->device_info.device_type,
    };

    while (1)
    {
        vsysbus_write(&packet);
        sleep(3);
    }

    pthread_exit(NULL);
}