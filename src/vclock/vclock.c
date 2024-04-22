#include <stdio.h>
#include <unistd.h>

#include "vclock.h"
#include "device_table/device_table.h"
#include "vsysbus/vsysbus.h"

static void *vclock_loop(void *vargp);

int vclock_init(vCLOCK_t *vclock)
{
    vclock->device_info = device_table_add(DEVICE_TYPE_CLOCK);

    vclock->logger.device_info = vclock->device_info;
    vclock->logger.file_name = "../logs/vclock.txt";
    logger_init(&vclock->logger);

    log_info(&vclock->logger, "Initializing.\n");
    return 0;
}

int vclock_start(vCLOCK_t *vclock)
{
    log_info(&vclock->logger, "Starting.\n");

    pthread_create(&(vclock->device_info->device_tid), NULL, vclock_loop, (void *)vclock);

    return 0;
}

int vclock_shutdown(vCLOCK_t *vclock)
{
    log_info(&vclock->logger, "Shutting down.\n");
    logger_shutdown(&vclock->logger);

    return 0;
}

static void *vclock_loop(void *vargp)
{
    vCLOCK_t *vclock = (vCLOCK_t *)vargp;

    log_info(&vclock->logger, "Running on thread [0x%016lx].\n", vclock->device_info->device_tid);

    while (vclock->device_info->device_running)
    {
        sleep(3);
    }

    pthread_exit(NULL);
}