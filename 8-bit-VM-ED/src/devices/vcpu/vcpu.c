#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vcpu.h"
#include "internal/device_table/device_table.h"
#include "internal/events/events.h"

static vCPU_t vcpu;

//
// External functions
// ------------------
//

void vcpu_init(const void *args)
{
    vcpu.device_info = device_table_add(DEVICE_TYPE_CPU);

    vcpu.logger.device_info = vcpu.device_info;
    vcpu.logger.file_name = "../logs/vcpu.txt";
    logger_init(&vcpu.logger);

    log_info(&vcpu.logger, "Initializing.\n");
}

void vcpu_update(const void *args)
{
    log_info(&vcpu.logger, "Update\n");
}

void vcpu_shutdown(const void *args)
{
    log_info(&vcpu.logger, "Shutting down.\n");
    logger_shutdown(&vcpu.logger);
}