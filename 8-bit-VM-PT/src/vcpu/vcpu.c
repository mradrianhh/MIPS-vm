#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vcpu.h"
#include "device_table/device_table.h"
#include "vsysbus/vsysbus.h"
#include "vmemory_controller.h"
#include "vnvic/interrupt_event.h"
#include "events/events.h"

static void registers_init(vCPU_t *vcpu);
static void *vcpu_loop(void *vargp);

static void check_interrupt(vCPU_t *vcpu);
static void handle_interrupts(LOGGER_t *logger, InterruptArgs_t args);
static void handle_test(LOGGER_t *logger, void *args);
static void handle_test2(LOGGER_t *logger, void *args);

static void fetch_instruction(vCPU_t *vcpu);
static void decode_instruction(vCPU_t *vcpu);
static void execute_instruction(vCPU_t *vcpu);

static void vcpu_insn_decoding_map_init(vCPU_INSN_DECODING_MAP_t *map);

static void vcpu_insn_dec_add(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_add(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands);

static void vcpu_insn_dec_mov(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_mov(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands);

static void vcpu_insn_dec_ldr(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_ldr(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands);

static void vcpu_insn_dec_str(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_str(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands);

static void vcpu_insn_dec_hlt(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction);
static void vcpu_insn_hlt(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands);

int vcpu_init(vCPU_t *vcpu)
{
    vcpu->device_info = device_table_add(DEVICE_TYPE_CPU);

    vcpu->logger.device_info = vcpu->device_info;
    vcpu->logger.file_name = "../logs/vcpu.txt";
    logger_init(&vcpu->logger);

    log_info(&vcpu->logger, "Initializing.\n");

    vcpu->interrupt_event = interrupt_event_get();
    interrupt_event_subscribe(vcpu->interrupt_event, vcpu->device_info->device_id,
                              &vcpu->logger, handle_interrupts);

    registers_init(vcpu);
    vcpu_insn_decoding_map_init(&vcpu->decoding_map);
    vmemory_controller_init(&vcpu->vmemory_controller, &vcpu->special_registers.MAR, &vcpu->special_registers.MDR);

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

void *vcpu_loop(void *vargp)
{
    vCPU_t *vcpu = (vCPU_t *)vargp;

    log_info(&vcpu->logger, "Running on thread [0x%016lx].\n", vcpu->device_info->device_tid);

    while (vcpu->device_info->device_running && !vcpu->special_registers.CTL.halt_exec)
    {
        log_trace(&vcpu->logger, "CYCLE START.\n");
        log_trace(&vcpu->vmemory_controller.logger, "CYCLE START.\n");

        fetch_instruction(vcpu);
        decode_instruction(vcpu);
        execute_instruction(vcpu);

        log_trace(&vcpu->logger, "CYCLE END.\n");
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
    printf("CTL: [0x%02x]\n", vcpu->special_registers.CTL.control_register);

    char input;
    scanf("Press any key to continue. . . %c", &input);
}

void check_interrupt(vCPU_t *vcpu)
{
    // Check register for pending interrupts.
    log_trace(&vcpu->logger, "No pending interrupts.\n");
}

void handle_interrupts(LOGGER_t *logger, InterruptArgs_t args)
{
}

void handle_test(LOGGER_t *logger, void *args)
{
    InterruptArgs_t *temp = (InterruptArgs_t *)args;
    log_event(logger, "Test - Interrupt Code: %d Device ID: %d.\n", temp->interrupt_code, temp->device_id);
}

void handle_test2(LOGGER_t *logger, void *args)
{
    InterruptArgs_t *temp = (InterruptArgs_t *)args;
    log_event(logger, "Test2 - Interrupt Code: %d Device ID: %d.\n", temp->interrupt_code, temp->device_id);
}

void registers_init(vCPU_t *vcpu)
{
    log_trace(&vcpu->logger, "Initializing registers.\n");
    memset(&vcpu->gp_registers, 0, sizeof(vcpu->gp_registers));
    memset(&vcpu->special_registers, 0, sizeof(vcpu->special_registers));
}

/// @brief
/// Fetch instruction from address in PC.
/// The instruction is fetched through the memory-controller.
/// The address value in the PC is loaded into the MAR and sent on the access-bus.
/// The data is then received from memory into the MDR register and loaded into the IR register.
/// Afterwards, the PC value is incremented.
/// @param vcpu is a reference to the CPU responsible for fetching instructions.
void fetch_instruction(vCPU_t *vcpu)
{
    log_trace(&vcpu->logger, "Fetching instruction at PC value [0x%02x].\n", vcpu->special_registers.PC);
    vcpu->special_registers.MAR = vcpu->special_registers.PC;
    log_trace(&vcpu->logger, "Moved PC(0x%02x) to MAR.\n", vcpu->special_registers.PC);
    vmemory_controller_fetch_page(&vcpu->vmemory_controller);
    log_trace(&vcpu->logger, "Fetched page [0x%02x] into MDR from address [0x%02x].\n",
              vcpu->special_registers.MDR, vcpu->special_registers.MAR);
    vcpu->special_registers.IR = vcpu->special_registers.MDR;
    log_trace(&vcpu->logger, "Moved MDR(0x%02x) into IR.\n", vcpu->special_registers.MDR);
    vcpu->special_registers.PC++;
    log_trace(&vcpu->logger, "Incremented PC. New value: [0x%02x].\n", vcpu->special_registers.PC);
}

/// @brief
/// Decode instruction stored in IR.
/// @param vcpu is a reference to the CPU responsible for decoding instructions.
void decode_instruction(vCPU_t *vcpu)
{
    log_trace(&vcpu->logger, "Decoding instruction [0x%02x].\n", vcpu->special_registers.IR);
    _vcpu_insn_dec_func_t decoding_function = vcpu->decoding_map.decoding_functions[(vcpu->special_registers.IR & OPCODE_MASK) >> 4];
    decoding_function(&vcpu->insn_decoding, vcpu->special_registers.IR);
}

void execute_instruction(vCPU_t *vcpu)
{
    log_trace(&vcpu->logger, "Executing instruction [0x%02x].\n", vcpu->special_registers.IR);
    vcpu->insn_decoding.opcode_func(vcpu, &vcpu->insn_decoding.operands);
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

void vcpu_insn_add(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands)
{
    log_debug(&vcpu->logger, "R%d: [0x%02x] R%d: [0x%02x]\n",
              operands->operand1, vcpu->gp_registers.registers[operands->operand1],
              operands->operand2, vcpu->gp_registers.registers[operands->operand2]);
    vcpu->gp_registers.registers[operands->operand1] += vcpu->gp_registers.registers[operands->operand2];
    log_trace(&vcpu->logger, "ADD R%d, R%d ; R%d = R%d + R%d.\n",
              operands->operand1, operands->operand2,
              operands->operand1, operands->operand1, operands->operand2);
    log_debug(&vcpu->logger, "R%d: [0x%02x] R%d: [0x%02x]\n",
              operands->operand1, vcpu->gp_registers.registers[operands->operand1],
              operands->operand2, vcpu->gp_registers.registers[operands->operand2]);
}

void vcpu_insn_dec_mov(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction)
{
    insn_decoding->opcode_func = vcpu_insn_mov;
    insn_decoding->operands.operand1 = (instruction & DEST_MASK) >> 2;
    insn_decoding->operands.operand2 = (instruction & SOURCE_MASK);
}

void vcpu_insn_mov(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands)
{
    log_debug(&vcpu->logger, "R%d: [0x%02x] R%d: [0x%02x]\n",
              operands->operand1, vcpu->gp_registers.registers[operands->operand1],
              operands->operand2, vcpu->gp_registers.registers[operands->operand2]);
    vcpu->gp_registers.registers[operands->operand1] = vcpu->gp_registers.registers[operands->operand2];
    log_trace(&vcpu->logger, "MOV R%d, R%d ; R%d = R%d.\n",
              operands->operand1, operands->operand2,
              operands->operand1, operands->operand2);
    log_debug(&vcpu->logger, "R%d: [0x%02x] R%d: [0x%02x]\n",
              operands->operand1, vcpu->gp_registers.registers[operands->operand1],
              operands->operand2, vcpu->gp_registers.registers[operands->operand2]);
}

void vcpu_insn_dec_ldr(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction)
{
    insn_decoding->opcode_func = vcpu_insn_ldr;
    insn_decoding->operands.operand1 = (instruction & DEST_MASK) >> 2;
    insn_decoding->operands.operand2 = (instruction & SOURCE_MASK);
}

void vcpu_insn_ldr(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands)
{
    vcpu->special_registers.MAR = vcpu->gp_registers.registers[operands->operand2];
    log_trace(&vcpu->logger, "Moved R%d[0x%02x] to MAR.\n",
              operands->operand2, vcpu->gp_registers.registers[operands->operand2]);

    vmemory_controller_fetch_page(&vcpu->vmemory_controller);
    log_trace(&vcpu->logger, "Fetched page [0x%02x] into MDR from address [0x%02x].\n",
              vcpu->special_registers.MDR, vcpu->special_registers.MAR);

    log_debug(&vcpu->logger, "R%d: [0x%02x] R%d: [0x%02x]\n",
              operands->operand1, vcpu->gp_registers.registers[operands->operand1],
              operands->operand2, vcpu->gp_registers.registers[operands->operand2]);
    vcpu->gp_registers.registers[operands->operand1] = vcpu->special_registers.MDR;
    log_trace(&vcpu->logger, "LDR R%d, R%d ; R%d = [R%d].\n",
              operands->operand1, operands->operand2,
              operands->operand1, operands->operand2);
    log_debug(&vcpu->logger, "R%d: [0x%02x] R%d: [0x%02x]\n",
              operands->operand1, vcpu->gp_registers.registers[operands->operand1],
              operands->operand2, vcpu->gp_registers.registers[operands->operand2]);
}

void vcpu_insn_dec_str(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction)
{
    insn_decoding->opcode_func = vcpu_insn_str;
    insn_decoding->operands.operand1 = (instruction & DEST_MASK) >> 2;
    insn_decoding->operands.operand2 = (instruction & SOURCE_MASK);
}

void vcpu_insn_str(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands)
{
    vcpu->special_registers.MAR = vcpu->gp_registers.registers[operands->operand2];
    log_trace(&vcpu->logger, "Moved R%d[0x%02x] to MAR.\n",
              operands->operand2, vcpu->gp_registers.registers[operands->operand2]);

    vcpu->special_registers.MDR = vcpu->gp_registers.registers[operands->operand1];
    log_trace(&vcpu->logger, "Wrote R%d[0x%02x] to MDR.\n",
              operands->operand1, vcpu->gp_registers.registers[operands->operand1]);

    vmemory_controller_write_page(&vcpu->vmemory_controller);

    log_trace(&vcpu->logger, "LDR R%d, R%d ; R%d = [R%d].\n",
              operands->operand1, operands->operand2,
              operands->operand1, operands->operand2);
}

void vcpu_insn_dec_hlt(vCPU_INSN_DECODING_t *insn_decoding, uint8_t instruction)
{
    insn_decoding->opcode_func = vcpu_insn_hlt;
}

void vcpu_insn_hlt(vCPU_t *vcpu, vCPU_INSN_OPERANDS_t *operands)
{
    vcpu->special_registers.CTL.halt_exec = 1;
    log_info(&vcpu->logger, "Halting execution.\n");
}