#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vcpu.h"
#include "device_table/device_table.h"
#include "vsysbus/vsysbus.h"
#include "vcontrol_unit.h"
#include "vmemory_controller.h"

static void registers_init(vCPU_t *vcpu);
static void *vcpu_loop(void *vargp);
static void check_interrupt(vCPU_t *vcpu);

int vcpu_init(vCPU_t *vcpu)
{
    vcpu->device_info = device_table_add(DEVICE_TYPE_CPU);

    vcpu->logger.device_info = vcpu->device_info;
    vcpu->logger.file_name = "../logs/vcpu.txt";
    logger_init(&vcpu->logger);

    log_info(&vcpu->logger, "Initializing.\n");

    registers_init(vcpu);
    vmemory_controller_init(&vcpu->vmemory_controller, &vcpu->special_registers.MAR, &vcpu->special_registers.MDR);
    vcontrol_unit_init(&vcpu->vcontrol_unit, &vcpu->gp_registers, &vcpu->special_registers, &vcpu->vmemory_controller);

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
    vcontrol_unit_shutdown(&vcpu->vcontrol_unit);
    log_info(&vcpu->logger, "Shutting down.\n");
    vcpu->device_info->device_running = DEVICE_STOPPED;
    logger_shutdown(&vcpu->logger);

    return 0;
}

static void *vcpu_loop(void *vargp)
{
    vCPU_t *vcpu = (vCPU_t *)vargp;

    log_info(&vcpu->logger, "Running on thread [0x%016lx].\n", vcpu->device_info->device_tid);

    while (vcpu->device_info->device_running && !vcpu->special_registers.CTL.halt_exec)
    {
        log_trace(&vcpu->logger, "CYCLE START.\n");
        log_trace(&vcpu->vcontrol_unit.logger, "CYCLE START.\n");
        log_trace(&vcpu->vmemory_controller.logger, "CYCLE START.\n");

        log_trace(&vcpu->logger, "Calling %s(%d) to fetch next instruction.\n",
                  convert_device_type_str(vcpu->vcontrol_unit.device_info->device_type),
                  vcpu->vcontrol_unit.device_info->device_id);
        fetch_instruction(&vcpu->vcontrol_unit);

        log_trace(&vcpu->logger, "Calling %s(%d) to decode instruction.\n",
                  convert_device_type_str(vcpu->vcontrol_unit.device_info->device_type),
                  vcpu->vcontrol_unit.device_info->device_id);
        decode_instruction(&vcpu->vcontrol_unit);

        log_trace(&vcpu->logger, "Calling %s(%d) to execute instruction.\n",
                  convert_device_type_str(vcpu->vcontrol_unit.device_info->device_type),
                  vcpu->vcontrol_unit.device_info->device_id);
        execute_instruction(&vcpu->vcontrol_unit);

        log_trace(&vcpu->logger, "CYCLE END.\n");
        log_trace(&vcpu->vcontrol_unit.logger, "CYCLE END.\n");
        log_trace(&vcpu->vmemory_controller.logger, "CYCLE END.\n");

        check_interrupt(vcpu);
        sleep(1);
    }

    pthread_exit(NULL);
}

void registers_dump(vCPU_t *vcpu)
{
    printf("*\n");
    printf("* Register Dump\n");
    printf("*\n");
    printf("R0:  [0x%02x]\n", vcpu->gp_registers.R0);
    printf("R1:  [0x%02x]\n", vcpu->gp_registers.R1);
    printf("R2:  [0x%02x]\n", vcpu->gp_registers.R2);
    printf("R3:  [0x%02x]\n", vcpu->gp_registers.R3);
    printf("MAR: [0x%02x]\n", vcpu->special_registers.MAR);
    printf("MDR: [0x%02x]\n", vcpu->special_registers.MDR);
    printf("PC:  [0x%02x]\n", vcpu->special_registers.PC);
    printf("LR:  [0x%02x]\n", vcpu->special_registers.LR);
    printf("SP:  [0x%02x]\n", vcpu->special_registers.SP);
    printf("INSN: [0x%02x]\n", vcpu->special_registers.IR);
    printf("CTL: [0x%02x]\n", vcpu->special_registers.CTL);

    char input;
    scanf("Press any key to continue. . . %c", &input);
}

void check_interrupt(vCPU_t *vcpu)
{
    log_trace(&vcpu->logger, "No pending interrupts.\n");
}

static void registers_init(vCPU_t *vcpu)
{
    log_trace(&vcpu->logger, "Initializing registers.\n");
    memset(&vcpu->gp_registers, 0, sizeof(vcpu->gp_registers));
    memset(&vcpu->special_registers, 0, sizeof(vcpu->special_registers));
}
