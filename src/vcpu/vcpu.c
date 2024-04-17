#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vcpu.h"
#include "../device_table/device_table.h"
#include "vsysbus/vsysbus.h"

static void registers_init(vCPU_t *vcpu);
static void write_vsysbus(vCPU_t *vcpu, uint8_t data);
static void *vcpu_loop(void *vargp);

int vcpu_init(vCPU_t *vcpu)
{
    vcpu->device_info.device_id = device_id_count++;
    vcpu->device_info.device_type = DEVICE_TYPE_CPU;
    device_table_add(vcpu->device_info);
    printf("Device vCPU(%d) initializing...\n", vcpu->device_info.device_id);
    registers_init(vcpu);

    return 0;
}

static void registers_init(vCPU_t *vcpu)
{
    memset(&vcpu->registers, 0, sizeof(vcpu->registers));
}

int vcpu_start(vCPU_t *vcpu)
{
    printf("Device vCPU(%d) starting...\n", vcpu->device_info.device_id);

    pthread_create(&(vcpu->device_info.device_tid), NULL, vcpu_loop, (void *)vcpu);

    return 0;
}

static void *vcpu_loop(void *vargp)
{
    vCPU_t *vcpu = (vCPU_t *)vargp;
    uint8_t i = 0;
    while (1)
    {
        write_vsysbus(vcpu, i++);
        sleep(1);
    }

    pthread_exit(NULL);
}

static void write_vsysbus(vCPU_t *vcpu, uint8_t data)
{
    vSYSBUS_PACKET_t packet = {
        .device_id = vcpu->device_info.device_id,
        .data = data,
    };
    vsysbus_write(&packet);
}

void registers_dump(vCPU_t *vcpu)
{
    printf("*\n");
    printf("* Register Dump\n");
    printf("*\n");
    printf("R0:  [0x%02x]\n", vcpu->registers.R0);
    printf("R1:  [0x%02x]\n", vcpu->registers.R1);
    printf("R2:  [0x%02x]\n", vcpu->registers.R2);
    printf("R3:  [0x%02x]\n", vcpu->registers.R3);
    printf("R4:  [0x%02x]\n", vcpu->registers.R4);
    printf("R5:  [0x%02x]\n", vcpu->registers.R5);
    printf("R6:  [0x%02x]\n", vcpu->registers.R6);
    printf("R7:  [0x%02x]\n", vcpu->registers.R7);
    printf("R8:  [0x%02x]\n", vcpu->registers.R8);
    printf("R9:  [0x%02x]\n", vcpu->registers.R9);
    printf("R10: [0x%02x]\n", vcpu->registers.R10);
    printf("R11: [0x%02x]\n", vcpu->registers.R11);
    printf("R12: [0x%02x]\n", vcpu->registers.R12);
    printf("PC:  [0x%02x]\n", vcpu->registers.PC);
    printf("LR:  [0x%02x]\n", vcpu->registers.LR);
    printf("SP:  [0x%02x]\n", vcpu->registers.SP);
}
