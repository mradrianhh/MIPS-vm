#include <string.h>

#include "vcontrol_unit.h"

static void registers_init(vCONTROL_UNIT_t *vcu);

int vcontrol_unit_init(vCONTROL_UNIT_t *vcu)
{
    vcu->device_info = device_table_add(DEVICE_TYPE_CONTROL_UNIT);

    vcu->logger.device_info = vcu->device_info;
    vcu->logger.file_name = "../logs/vcontrol_unit.txt";
    logger_init(&vcu->logger);

    log_info(&vcu->logger, "Initializing.\n");

    registers_init(vcu);

    return 0;
}

int vcontrol_unit_shutdown(vCONTROL_UNIT_t *vcu)
{
    log_info(&vcu->logger, "Shutting down.\n");
    logger_shutdown(&vcu->logger);

    return 0;
}

void registers_init(vCONTROL_UNIT_t *vcu)
{
    log_trace(&vcu->logger, "Initializing registers.\n");
    memset(&vcu->cu_registers, 0, sizeof(vcu->cu_registers));
}

void fetch_instruction(vCONTROL_UNIT_t *vcu, vMEMORY_CONTROLLER_t *controller)
{
    controller->mc_registers.MAR = vcu->cu_registers.PC;
    log_trace(&vcu->logger, "Moved PC(0x%02x) to MAR.\n", vcu->cu_registers.PC);
    vmemory_controller_fetch_page(controller);
    log_trace(&vcu->logger, "Fetched page [0x%02x] into MDR from address [0x%02x].\n",
              controller->mc_registers.MDR, controller->mc_registers.MAR);
    vcu->cu_registers.PC++;
    log_trace(&vcu->logger, "Incremented PC. New value: [0x%02x].\n", vcu->cu_registers.PC);
}

void decode_instruction(vCONTROL_UNIT_t *vcu)
{
    log_trace(&vcu->logger, "Decoding instruction [0x%02x].\n", vcu->cu_registers.IR);
}

void execute_instruction(vCONTROL_UNIT_t *vcu)
{
    log_trace(&vcu->logger, "Executing instruction [0x%02x].\n", vcu->cu_registers.IR);
}