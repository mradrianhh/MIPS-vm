#include <string.h>
#include <stdarg.h>

#include "vcontrol_unit.h"
#include "vcu_decoding.h"

static void vcpu_insn_decoding_map_init(vCPU_INSN_DECODING_MAP_t *map);

static void vcpu_insn_dec_add(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_add(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                          GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands);

static void vcpu_insn_dec_mov(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_mov(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                          GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands);

static void vcpu_insn_dec_ldr(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_ldr(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                          GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands);

static void vcpu_insn_dec_str(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_str(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                          GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands);

static void vcpu_insn_dec_hlt(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_hlt(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                          GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands);

int vcontrol_unit_init(vCONTROL_UNIT_t *vcu, GP_REGISTERS_t *gp_registers,
                       SPECIAL_REGISTERS_t *special_registers, vMEMORY_CONTROLLER_t *controller)
{
    vcu->device_info = device_table_add(DEVICE_TYPE_CONTROL_UNIT);

    vcu->logger.device_info = vcu->device_info;
    vcu->logger.file_name = "../logs/vcontrol_unit.txt";
    logger_init(&vcu->logger);

    log_info(&vcu->logger, "Initializing.\n");

    vcu->gp_registers = gp_registers;
    vcu->special_registers = special_registers;
    vcu->vmemory_controller = controller;
    vcpu_insn_decoding_map_init(&vcu->decoding_map);

    return 0;
}

int vcontrol_unit_shutdown(vCONTROL_UNIT_t *vcu)
{
    log_info(&vcu->logger, "Shutting down.\n");
    logger_shutdown(&vcu->logger);

    return 0;
}

/// @brief
/// Fetch instruction from address in PC.
/// The instruction is fetched through the memory-controller.
/// The address value in the PC is loaded into the MAR and sent on the access-bus.
/// The data is then received from memory into the MDR register and loaded into the IR register.
/// Afterwards, the PC value is incremented.
/// @param vcu is a reference to the control unit responsible for fetching instructions.
/// @param controller is a reference to the memory-controller responsible for accessing memory.
void fetch_instruction(vCONTROL_UNIT_t *vcu)
{
    vcu->special_registers->MAR = vcu->special_registers->PC;
    log_trace(&vcu->logger, "Moved PC(0x%02x) to MAR.\n", vcu->special_registers->PC);
    vmemory_controller_fetch_page(vcu->vmemory_controller);
    log_trace(&vcu->logger, "Fetched page [0x%02x] into MDR from address [0x%02x].\n",
              vcu->special_registers->MDR, vcu->special_registers->MAR);
    vcu->special_registers->IR = vcu->special_registers->MDR;
    log_trace(&vcu->logger, "Moved MDR(0x%02x) into IR.\n", vcu->special_registers->MDR);
    vcu->special_registers->PC++;
    log_trace(&vcu->logger, "Incremented PC. New value: [0x%02x].\n", vcu->special_registers->PC);
}

/// @brief
/// Decode instruction stored in IR.
/// @param vcu is a reference to the control unit responsible for decoding instructions.
void decode_instruction(vCONTROL_UNIT_t *vcu)
{
    log_trace(&vcu->logger, "Decoding instruction [0x%02x].\n", vcu->special_registers->IR);
    _vcpu_insn_dec_func_t decoding_function = vcu->decoding_map.decoding_functions[(vcu->special_registers->IR & OPCODE_MASK) >> 4];
    decoding_function(&vcu->insn_decoding, vcu->special_registers->IR);
}

void execute_instruction(vCONTROL_UNIT_t *vcu)
{
    log_trace(&vcu->logger, "Executing instruction [0x%02x].\n", vcu->special_registers->IR);
    vcu->insn_decoding.opcode_func(&vcu->logger, vcu->special_registers,
                                   vcu->gp_registers, &vcu->insn_decoding.operands);
}

void vcpu_insn_decoding_map_init(vCPU_INSN_DECODING_MAP_t *map)
{
    vCPU_INSN_DECODING_MAP_t temp_map = {
        .add = vcpu_insn_dec_add,
        .mov = vcpu_insn_dec_mov,
        .ldr = vcpu_insn_dec_ldr,
        .str = vcpu_insn_dec_str,
        .hlt = vcpu_insn_dec_hlt,
    };

    *map = temp_map;
}

void vcpu_insn_dec_add(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction)
{
    insn_decoding->opcode_func = vcpu_insn_add;
    insn_decoding->operands.operand1 = (instruction & DEST_MASK) >> 2;
    insn_decoding->operands.operand2 = (instruction & SOURCE_MASK);
}

void vcpu_insn_add(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                   GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands)
{
    log_debug(logger, "SIMULATING: ADD [0x%02x], [0x%02x] ; ADD R%d, R%d\n",
              operands->operand1, operands->operand2, operands->operand1, operands->operand2);
}

void vcpu_insn_dec_mov(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction)
{
    insn_decoding->opcode_func = vcpu_insn_mov;
    insn_decoding->operands.operand1 = (instruction & DEST_MASK) >> 2;
    insn_decoding->operands.operand2 = (instruction & SOURCE_MASK);
}

void vcpu_insn_mov(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                   GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands)
{
    log_debug(logger, "SIMULATING: MOV [0x%02x], [0x%02x] ; MOV R%d, R%d\n",
              operands->operand1, operands->operand2, operands->operand1, operands->operand2);
}

void vcpu_insn_dec_ldr(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction)
{
    insn_decoding->opcode_func = vcpu_insn_ldr;
    insn_decoding->operands.operand1 = (instruction & DEST_MASK) >> 2;
    insn_decoding->operands.operand2 = (instruction & SOURCE_MASK);
}

void vcpu_insn_ldr(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                   GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands)
{
    log_debug(logger, "SIMULATING: LDR [0x%02x], [0x%02x] ; LDR R%d, R%d\n",
              operands->operand1, operands->operand2, operands->operand1, operands->operand2);
}

void vcpu_insn_dec_str(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction)
{
    insn_decoding->opcode_func = vcpu_insn_str;
    insn_decoding->operands.operand1 = (instruction & DEST_MASK) >> 2;
    insn_decoding->operands.operand2 = (instruction & SOURCE_MASK);
}

void vcpu_insn_str(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                   GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands)
{
    log_debug(logger, "SIMULATING: STR [0x%02x], [0x%02x] ; STR R%d, R%d\n",
              operands->operand1, operands->operand2, operands->operand1, operands->operand2);
}

void vcpu_insn_dec_hlt(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction)
{
    insn_decoding->opcode_func = vcpu_insn_hlt;
}

void vcpu_insn_hlt(LOGGER_t *logger, SPECIAL_REGISTERS_t *special_registers,
                   GP_REGISTERS_t *gp_registers, vCPU_INSN_OPERANDS_t *operands)
{
    special_registers->CTL.halt_exec = 1;
    log_info(logger, "Halting execution.\n");
}
