#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vcpu.h"
#include "../common/common.h"

static int rc;

static void registers_init(vCPU_t *vcpu);

int vcpu_init(vCPU_t *vcpu)
{
    vcpu->device_id = device_id_count++;
    vcpu->device_type = DEVICE_TYPE_CPU;
    registers_init(vcpu);
    printf("Device vCPU(%d) initializing...\n", vcpu->device_id);
    return 0;
}

static void registers_init(vCPU_t *vcpu)
{
    memset(&vcpu->registers, 0, sizeof(vcpu->registers));
}
