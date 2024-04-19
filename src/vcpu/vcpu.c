#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vcpu.h"
#include "device_table/device_table.h"
#include "vsysbus/vsysbus.h"

static void registers_init(vCPU_t *vcpu);
static void *vcpu_loop(void *vargp);

int vcpu_init(vCPU_t *vcpu)
{
    vcpu->device_info = device_table_add(DEVICE_TYPE_CPU);

    vcpu->logger.device_info = vcpu->device_info;
    vcpu->logger.file_name = "../logs/vcpu.txt";
    logger_init(&vcpu->logger);

    log_info(&vcpu->logger, "Initializing.\n");
    
    registers_init(vcpu);
    vmemory_controller_init(&vcpu->vmemory_controller);

    return 0;
}

int vcpu_start(vCPU_t *vcpu)
{
    log_info(&vcpu->logger, "Starting.\n");
    vcpu->device_info->device_running = DEVICE_RUNNING;

    pthread_create(&vcpu->device_info->device_tid, NULL, vcpu_loop, (void *)vcpu);

    return 0;
}

int vcpu_shutdown(vCPU_t *vcpu)
{
    vmemory_controller_shutdown(&vcpu->vmemory_controller);
    log_info(&vcpu->logger, "Shutting down.\n");
    vcpu->device_info->device_running = DEVICE_STOPPED;
    logger_shutdown(&vcpu->logger);

    return 0;
}

static void *vcpu_loop(void *vargp)
{
    vCPU_t *vcpu = (vCPU_t *)vargp;

    log_info(&vcpu->logger, "Running on thread [0x%08lx].\n", vcpu->device_info->device_tid);

    uint8_t i = 0;
    PAGE_t page;
    while (vcpu->device_info->device_running)
    {
        vmemory_controller_fetch_page(&vcpu->vmemory_controller, i, &page);
        log_trace(&vcpu->logger, "Fetched page [0x%02x] from address [0x%02x].\n", page, i);
        i++;
        if (i > 255)
        {
            i = 0;
        }
        sleep(3);
    }

    pthread_exit(NULL);
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

static void registers_init(vCPU_t *vcpu)
{
    memset(&vcpu->registers, 0, sizeof(vcpu->registers));
}