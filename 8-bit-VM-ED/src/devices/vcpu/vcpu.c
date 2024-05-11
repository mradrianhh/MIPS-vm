#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vcpu.h"
#include "vcpu_state.h"
#include "vcpu_decoder.h"
#include "internal/device_table/device_table.h"
#include "internal/events/events.h"
#include "devices/vmemory/vmemory.h"
#include "devices/vclock/EdgeChangedEvent.h"

static vCPU_state_t prev_state;
static vCPU_t vcpu;

static void init_state();
static void init_registers(vCPU_state_t *state);
static void dump_registers(vCPU_state_t *state);
static void handle_rising();
static void handle_falling();

static vCPU_INSN_EXECUTER_MAP_t init_executer_map();
static void vcpu_insn_execute_nop(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_add(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_mov(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_ldr(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_str(vCPU_INSN_OPERANDS_t operands);

//
// External functions
// ------------------
//

void vcpu_init()
{
    vcpu.device_info = device_table_add(DEVICE_TYPE_CPU);

    vcpu.logger.device_info = vcpu.device_info;
    vcpu.logger.file_name = "../logs/vcpu.txt";
    logger_init(&vcpu.logger);

    log_info(&vcpu.logger, "Initializing.\n");

    init_state();
    vCPU_INSN_EXECUTER_MAP_t executer_map = init_executer_map();
    vcpu_decoder_init(executer_map);
}

void vcpu_update(const void *args)
{
    const EdgeChangedEventArgs_t *e = (const EdgeChangedEventArgs_t *)args;
    if(e->edge)
    {
        handle_rising();
    }
    else {
        handle_falling();
    }
}

void vcpu_shutdown()
{
    log_info(&vcpu.logger, "Shutting down.\n");
    logger_shutdown(&vcpu.logger);
}

void vcpu_dump_state()
{
    log_debug(&vcpu.logger, ">> Previous state\n");
    dump_registers(&prev_state);

    log_debug(&vcpu.logger, ">> Current state\n");
    dump_registers(&vcpu.state);
}

//
// Internal functions
// ------------------
//

void init_state()
{
    // Initialize previous state.
    init_registers(&prev_state);
    prev_state.decoded_insn.execute = vcpu_insn_execute_nop;
    prev_state.decoded_insn.operands.operand1 = 0;
    prev_state.decoded_insn.operands.operand2 = 0;
    prev_state.decoded_insn.operands.operand3 = 0;

    // Initialize current state.
    init_registers(&vcpu.state);
    vcpu.state.decoded_insn.execute = vcpu_insn_execute_nop;
    vcpu.state.decoded_insn.operands.operand1 = 0;
    vcpu.state.decoded_insn.operands.operand2 = 0;
    vcpu.state.decoded_insn.operands.operand3 = 0;
}

void init_registers(vCPU_state_t *state)
{
    memset(&state->gp_registers, 0, sizeof(state->gp_registers));
    memset(&state->special_registers, 0, sizeof(state->special_registers));
}

void dump_registers(vCPU_state_t *state)
{
    log_debug(&vcpu.logger, "\t>> Register Dump\n");
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "R0", state->gp_registers.R0);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "R1", state->gp_registers.R1);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "R2", state->gp_registers.R2);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "R3", state->gp_registers.R3);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "MAR", state->special_registers.MAR);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "MDR", state->special_registers.MDR);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "PC", state->special_registers.PC);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "LR", state->special_registers.LR);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "SP", state->special_registers.SP);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "IR", state->special_registers.IR);
    log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n", "CTL", state->special_registers.CTL.control_register);
}

void handle_rising()
{
    log_info(&vcpu.logger, ">> CYCLE START.\n");
    // Fetch instruction
    // 1. Load prev-PC into prev-MAR.
    vcpu.state.special_registers.MAR = prev_state.special_registers.PC;
    // 2. Memory controller communicates prev-MAR over address bus.
    // 3. Memory listens on address bus and returns address.
    // 4. Memory controller places data into MDR.
    vcpu.state.special_registers.MDR = vmemory_fetch(vcpu.state.special_registers.MAR);
    // 5. Load MDR into IR.
    vcpu.state.special_registers.IR = vcpu.state.special_registers.MDR;
    log_info(&vcpu.logger, "Fetch: Instruction [0x%02x] at address [0x%02x](PC).\n",
             vcpu.state.special_registers.IR, prev_state.special_registers.PC);
    // 6. Increment prev-PC.
    vcpu.state.special_registers.PC++;

    // Decode instruction
    // 1. Decode prev-IR.
    vcpu.state.decoded_insn = vcpu_decoder_decode(prev_state.special_registers.IR);
    log_info(&vcpu.logger, "Decode: Instruction [0x%02x].\n", prev_state.special_registers.IR);

    // Execute instruction
    // 1. Execute prev-INSN.
    prev_state.decoded_insn.execute(prev_state.decoded_insn.operands);
    log_info(&vcpu.logger, "Execute: Instruction [0x%02x].\n", prev_state.decoded_insn.ir);
}

void handle_falling()
{
    log_info(&vcpu.logger, ">> CYCLE END.\n");
    log_info(&vcpu.logger, ">>\n");
    // Move vcpu.state to prev_state.
    prev_state = vcpu.state;
}

vCPU_INSN_EXECUTER_MAP_t init_executer_map()
{
    vCPU_INSN_EXECUTER_MAP_t map;
    map.executers[NOP] = vcpu_insn_execute_nop;
    map.executers[ADD] = vcpu_insn_execute_add;
    map.executers[MOV] = vcpu_insn_execute_mov;
    map.executers[LDR] = vcpu_insn_execute_ldr;
    map.executers[STR] = vcpu_insn_execute_str;

    return map;
}

void vcpu_insn_execute_nop(vCPU_INSN_OPERANDS_t operands)
{
}

void vcpu_insn_execute_add(vCPU_INSN_OPERANDS_t operands)
{
    vcpu.state.gp_registers.registers[operands.operand1] += vcpu.state.gp_registers.registers[operands.operand2];
}

void vcpu_insn_execute_mov(vCPU_INSN_OPERANDS_t operands)
{
    vcpu.state.gp_registers.registers[operands.operand1] = vcpu.state.gp_registers.registers[operands.operand2];
}

void vcpu_insn_execute_ldr(vCPU_INSN_OPERANDS_t operands)
{
}

void vcpu_insn_execute_str(vCPU_INSN_OPERANDS_t operands)
{
}