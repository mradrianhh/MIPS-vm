#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vcpu.h"
#include "vcpu_state.h"
#include "vcpu_decoder.h"
#include "guest/common/device_table/device_table.h"
#include "guest/common/events/events.h"
#include "guest/devices/vmemory/vmemory.h"
#include "guest/devices/vclock/EdgeChangedEvent.h"

static LOGICAL_REGISTER_FILE_t logical_register_file;
static vCPU_t vcpu;
static vcpu_insn_execute_t *executers;
static vcpu_insn_execute_t *rformat_executers;

static void init_state();
static void init_registers(vCPU_state_t *state);
static void dump_registers(vCPU_state_t *state);
static void handle_rising();
static void handle_falling();

static void vcpu_insn_execute_nop(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_add(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_addu(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_jr(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_lui(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_addiu(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_beq(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_j(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_jal(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_or(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_sw(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_lw(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_jalr(vCPU_INSN_OPERANDS_t operands);
static void vcpu_insn_execute_ori(vCPU_INSN_OPERANDS_t operands);

static void vcpu_flush_pipeline();

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

    vcache_init(&vcpu.icache, ICACHE_SIZE, 4);
    vcache_init(&vcpu.dcache, DCACHE_SIZE, 1);
    vcache_load(&vcpu.icache, 0xDEADBEEF);

    log_info(&vcpu.logger, "Initializing.\n");

    init_state();

    // Store handle to executer and rformat_executers and free on shutdown.
    executers = calloc(OPCODE_MAX_RANGE, sizeof(vcpu_insn_execute_t));
    executers[LUI] = vcpu_insn_execute_lui;
    executers[ADDIU] = vcpu_insn_execute_addiu;
    executers[BEQ] = vcpu_insn_execute_beq;
    executers[J] = vcpu_insn_execute_j;
    executers[JAL] = vcpu_insn_execute_jal;
    executers[SW] = vcpu_insn_execute_sw;
    executers[LW] = vcpu_insn_execute_lw;
    executers[ORI] = vcpu_insn_execute_ori;
    rformat_executers = calloc(FUNCT_MAX_RANGE, sizeof(vcpu_insn_execute_t));
    rformat_executers[ADD] = vcpu_insn_execute_add;
    rformat_executers[ADDU] = vcpu_insn_execute_addu;
    rformat_executers[NOP] = vcpu_insn_execute_nop;
    rformat_executers[JR] = vcpu_insn_execute_jr;
    rformat_executers[OR] = vcpu_insn_execute_or;
    rformat_executers[JALR] = vcpu_insn_execute_jalr;
    vcpu_decoder_init(&vcpu.state.register_file[REGISTER_ADDRESS_PC], executers, rformat_executers);
}

void vcpu_update(const void *args)
{
    const EdgeChangedEventArgs_t *e = (const EdgeChangedEventArgs_t *)args;
    if (e->edge)
    {
        handle_rising();
    }
    else
    {
        handle_falling();
    }
}

void vcpu_shutdown()
{
    log_info(&vcpu.logger, "Shutting down.\n");
    logger_shutdown(&vcpu.logger);
    free(executers);
    free(rformat_executers);
    vcache_destroy(&vcpu.icache);
    vcache_destroy(&vcpu.dcache);
}

void vcpu_dump_state()
{
    log_debug(&vcpu.logger, ">> Current state\n");
    dump_registers(&vcpu.state);
}

uint32_t *vcpu_get_pc_ref()
{
    return &vcpu.state.register_file[REGISTER_ADDRESS_PC];
}

//
// Internal functions
// ------------------
//

void init_state()
{
    // Initialize current state.
    init_registers(&vcpu.state);
    vcpu_flush_pipeline();
}

void init_registers(vCPU_state_t *state)
{
    // Initialize physical register file to 0.
    memset(&state->register_file, 0, sizeof(REGISTER_t) * MAX_REGISTERS);
    state->register_file[REGISTER_ADDRESS_PC] = RESET_VECTOR_VADDR;

    // Initialize logical register file with names.
    strcpy(logical_register_file[REGISTER_ADDRESS_ZERO], "$zero");
    strcpy(logical_register_file[REGISTER_ADDRESS_AT], "$at");
    strcpy(logical_register_file[REGISTER_ADDRESS_V0], "$v0");
    strcpy(logical_register_file[REGISTER_ADDRESS_V1], "$v1");
    strcpy(logical_register_file[REGISTER_ADDRESS_A0], "$a0");
    strcpy(logical_register_file[REGISTER_ADDRESS_A1], "$a1");
    strcpy(logical_register_file[REGISTER_ADDRESS_A2], "$a2");
    strcpy(logical_register_file[REGISTER_ADDRESS_A3], "$a3");
    strcpy(logical_register_file[REGISTER_ADDRESS_T0], "$t0");
    strcpy(logical_register_file[REGISTER_ADDRESS_T1], "$t1");
    strcpy(logical_register_file[REGISTER_ADDRESS_T2], "$t2");
    strcpy(logical_register_file[REGISTER_ADDRESS_T3], "$t3");
    strcpy(logical_register_file[REGISTER_ADDRESS_T4], "$t4");
    strcpy(logical_register_file[REGISTER_ADDRESS_T5], "$t5");
    strcpy(logical_register_file[REGISTER_ADDRESS_T6], "$t6");
    strcpy(logical_register_file[REGISTER_ADDRESS_T7], "$t7");
    strcpy(logical_register_file[REGISTER_ADDRESS_S0], "$s0");
    strcpy(logical_register_file[REGISTER_ADDRESS_S1], "$s1");
    strcpy(logical_register_file[REGISTER_ADDRESS_S2], "$s2");
    strcpy(logical_register_file[REGISTER_ADDRESS_S3], "$s3");
    strcpy(logical_register_file[REGISTER_ADDRESS_S4], "$s4");
    strcpy(logical_register_file[REGISTER_ADDRESS_S5], "$s5");
    strcpy(logical_register_file[REGISTER_ADDRESS_S6], "$s6");
    strcpy(logical_register_file[REGISTER_ADDRESS_S7], "$s7");
    strcpy(logical_register_file[REGISTER_ADDRESS_T8], "$t8");
    strcpy(logical_register_file[REGISTER_ADDRESS_T9], "$t9");
    strcpy(logical_register_file[REGISTER_ADDRESS_K0], "$k0");
    strcpy(logical_register_file[REGISTER_ADDRESS_K1], "$k1");
    strcpy(logical_register_file[REGISTER_ADDRESS_GP], "$gp");
    strcpy(logical_register_file[REGISTER_ADDRESS_SP], "$sp");
    strcpy(logical_register_file[REGISTER_ADDRESS_FP], "$fp");
    strcpy(logical_register_file[REGISTER_ADDRESS_RA], "$ra");
    strcpy(logical_register_file[REGISTER_ADDRESS_PC], "PC");
    strcpy(logical_register_file[REGISTER_ADDRESS_HI], "HI");
    strcpy(logical_register_file[REGISTER_ADDRESS_LO], "LO");
    strcpy(logical_register_file[REGISTER_ADDRESS_IR], "IR");
}

void dump_registers(vCPU_state_t *state)
{
    log_debug(&vcpu.logger, "\t>> Register Dump\n");
    for (int i = 0; i < MAX_REGISTERS; i++)
    {
        log_debug(&vcpu.logger, "\t%-4s:\t[0x%02x]\n",
                  logical_register_file[i], state->register_file[i]);
    }
}

void handle_rising()
{
    log_info(&vcpu.logger, ">> CYCLE START.\n");

    // There are three stages of the processor: fetch, decode and execute.
    // Each stage has it's own state and the
    // state is moved to the next stage on completion.
    // Because each stage depends on the previous, we have to move backwards so we don't
    // overwrite the data from the previous cycle.

    // Third stage - Execute:
    // - Execute the instruction
    {
        log_info(&vcpu.logger, ">>> EXECUTE STAGE(0x%08x).\n", vcpu.state.decode_insn); // BEQ
        vcpu.state.decoded_insn.execute(vcpu.state.decoded_insn.operands);
    }

    // Second stage - Decode:
    // - Decode the instruction.
    { // NOP
        vcpu.state.decode_insn = vcpu.state.register_file[REGISTER_ADDRESS_IR];
        log_info(&vcpu.logger, ">>> DECODE STAGE(0x%08x).\n", vcpu.state.decode_insn);
        vcpu.state.decoded_insn = vcpu_decoder_decode(vcpu.state.register_file[REGISTER_ADDRESS_IR]);
    }

    // First stage - Fetch:
    // - Fetch the next instruction.
    // - Increase PC.
    { // NOP
        vcpu.state.register_file[REGISTER_ADDRESS_IR] = vmemory_read_word(vcpu.state.register_file[REGISTER_ADDRESS_PC]);
        log_info(&vcpu.logger, ">>> FETCH STAGE(0x%08x).\n", vcpu.state.register_file[REGISTER_ADDRESS_IR]);
        log_debug(&vcpu.logger, "Fetching instruction at PC(0x%08x): 0x%08x)\n", vcpu.state.register_file[REGISTER_ADDRESS_PC], vcpu.state.register_file[REGISTER_ADDRESS_IR]);
        vcpu.state.register_file[REGISTER_ADDRESS_PC] += 4;
    }
}

void handle_falling()
{
    log_info(&vcpu.logger, ">> CYCLE END.\n");
    log_info(&vcpu.logger, "\n");
}

void vcpu_insn_execute_nop(vCPU_INSN_OPERANDS_t operands)
{
    printf("NOP\n");
}

void vcpu_insn_execute_add(vCPU_INSN_OPERANDS_t operands)
{
    log_debug(&vcpu.logger, "Executing ADD instruction: R%d(0x%08x) + R%d(0x%08x) = R%d\n", operands.rt, vcpu.state.register_file[operands.rt],
              operands.rs, vcpu.state.register_file[operands.rs], operands.rd);
    vcpu.state.register_file[operands.rd] = vcpu.state.register_file[operands.rs] + vcpu.state.register_file[operands.rt];
    log_debug(&vcpu.logger, "Executed ADD instruction: R%d(0x%08x) = R%d + R%d", operands.rd, vcpu.state.register_file[operands.rd], operands.rs, operands.rt);
    printf("ADD: %d + %d = %d\n", vcpu.state.register_file[operands.rs], vcpu.state.register_file[operands.rt], vcpu.state.register_file[operands.rd]);
}

void vcpu_insn_execute_addu(vCPU_INSN_OPERANDS_t operands)
{
    log_debug(&vcpu.logger, "Executing ADDU instruction: R%d(0x%08x) + R%d(0x%08x) = R%d\n", operands.rt, vcpu.state.register_file[operands.rt],
              operands.rs, vcpu.state.register_file[operands.rs], operands.rd);
    vcpu.state.register_file[operands.rd] = vcpu.state.register_file[operands.rs] + vcpu.state.register_file[operands.rt];
    log_debug(&vcpu.logger, "Executed ADDU instruction: R%d(0x%08x) = R%d + R%d", operands.rd, vcpu.state.register_file[operands.rd], operands.rs, operands.rt);
    printf("ADDU: R%d(0x%08x) = R%d + R%d\n", operands.rd, vcpu.state.register_file[operands.rd], operands.rs, operands.rt);
}

void vcpu_insn_execute_jr(vCPU_INSN_OPERANDS_t operands)
{
    printf("JR: PC(0x%08x) = R%d(0x%08x)\n", vcpu.state.register_file[REGISTER_ADDRESS_PC], operands.rs, vcpu.state.register_file[operands.rs]);
    vcpu.state.register_file[REGISTER_ADDRESS_PC] = vcpu.state.register_file[operands.rs];
    if (vcpu.state.register_file[REGISTER_ADDRESS_PC] % 4 != 0)
    {
        log_error(&vcpu.logger, "Error: Misaligned jump address: 0x%08x\n", vcpu.state.register_file[REGISTER_ADDRESS_PC]);
        return;
    }
    log_debug(&vcpu.logger, "Jumping to address: 0x%08x\n", vcpu.state.register_file[REGISTER_ADDRESS_PC]);

    // We need to flush pipeline so we don't execute previously fetched and decoded instructions.
    // We do this manually because we don't trust the compiler.
    // vcpu_flush_pipeline();
}

void vcpu_insn_execute_lui(vCPU_INSN_OPERANDS_t operands)
{
    vcpu.state.register_file[operands.rt] = operands.immed << 16;
    printf("LUI: R%d = 0x%08x\n", operands.rt, vcpu.state.register_file[operands.rt]);
    log_debug(&vcpu.logger, "Executed LUI instruction: Set R%d to 0x%08x\n", operands.rt, vcpu.state.register_file[operands.rt]);
}

void vcpu_insn_execute_addiu(vCPU_INSN_OPERANDS_t operands)
{
    uint32_t extended_immed;
    // We need to sign extend. First, append leading zeroes.
    extended_immed = IMMED_MASK & operands.immed;
    // Check if we should extend with 1's.
    if (operands.immed & INSN15_MASK)
    {
        // We flip the IMMED_MASK with an XOR so we can flip the upper 16 bits to 1's with an OR.
        extended_immed |= (IMMED_MASK ^ WORD_HIGH_VALUES);
    }

    log_debug(&vcpu.logger, "Executing ADDIU instruction: Add 0x%08x to R%d(0x%08x)\n", extended_immed, operands.rt, vcpu.state.register_file[operands.rt]);
    // Move via uint64_t to prevent wrap-around and allow truncation.
    uint64_t temp = vcpu.state.register_file[operands.rs] + extended_immed;
    vcpu.state.register_file[operands.rt] = temp;
    log_debug(&vcpu.logger, "Executed ADDIU instruction: R%d = 0x%08x\n", operands.rt, vcpu.state.register_file[operands.rt]);
    printf("ADDIU: R%d(0x%08x) = R%d + 0x%08x\n", operands.rt, vcpu.state.register_file[operands.rs], operands.rt, extended_immed);
}

void vcpu_insn_execute_beq(vCPU_INSN_OPERANDS_t operands)
{
    if (vcpu.state.register_file[operands.rs] == vcpu.state.register_file[operands.rt])
    {
        // We calculated the correct address in the decode-stage based on offset and PC, so we only reassign PC here.
        vcpu.state.register_file[REGISTER_ADDRESS_PC] = operands.address;
        log_debug(&vcpu.logger, "BEQ executed: Branching to address 0x%08x\n", vcpu.state.register_file[REGISTER_ADDRESS_PC]);

        // We need to flush pipeline so we don't execute the next instructions already loaded by inserting NOP/stall into fetch and decode stage.
        // We do this manually because we don't trust the compiler.
        // vcpu_flush_pipeline();
    }
    else
    {
        log_debug(&vcpu.logger, "BEQ condition not met: No branch taken.\n");
    }
}

void vcpu_insn_execute_j(vCPU_INSN_OPERANDS_t operands)
{
    printf("J: PC = 0x%08x\n", operands.address);
    vcpu.state.register_file[REGISTER_ADDRESS_PC] = operands.address;
    log_debug(&vcpu.logger, "J executed: Jumping to address 0x%08x\n", vcpu.state.register_file[REGISTER_ADDRESS_PC]);

    // vcpu_flush_pipeline();
}

void vcpu_insn_execute_jal(vCPU_INSN_OPERANDS_t operands)
{
    printf("JAL: PC(0x%08x) = 0x%08x\n", vcpu.state.register_file[REGISTER_ADDRESS_PC], operands.address);
    vcpu.state.register_file[REGISTER_ADDRESS_RA] = vcpu.state.register_file[REGISTER_ADDRESS_PC] - 4;
    vcpu.state.register_file[REGISTER_ADDRESS_PC] = operands.address;
    log_debug(&vcpu.logger, "JAL executed: Jumping to address 0x%08x, Link (RA) set to 0x%08x\n", vcpu.state.register_file[REGISTER_ADDRESS_PC], vcpu.state.register_file[REGISTER_ADDRESS_RA]);

    // vcpu_flush_pipeline();
}

void vcpu_insn_execute_or(vCPU_INSN_OPERANDS_t operands)
{
    vcpu.state.register_file[operands.rd] = vcpu.state.register_file[operands.rs] | vcpu.state.register_file[operands.rt];
    printf("OR: 0x%08x | 0x%08x = 0x%08x\n", vcpu.state.register_file[operands.rs], vcpu.state.register_file[operands.rt], vcpu.state.register_file[operands.rd]);
    log_debug(&vcpu.logger, "Executed OR instruction: Set R%d to 0x%08x\n", operands.rd, vcpu.state.register_file[operands.rd]);
}

void vcpu_insn_execute_sw(vCPU_INSN_OPERANDS_t operands)
{
    // We need to calculate memory address by sign-extending offset and adding it with
    // the content of Rs.
    // First, append leading zeroes.
    uint32_t extended_immed = IMMED_MASK & operands.immed;
    // Check if we should extend with 1's.
    if (operands.immed & INSN15_MASK)
    {
        // We flip the IMMED_MASK with an XOR so we can flip the upper 16 bits to 1's with an OR.
        extended_immed |= (IMMED_MASK ^ WORD_HIGH_VALUES);
    }
    // We have to add the extended offset with the content of Rs, but to prevent
    // C's wraparound, we do it via uint64_t to allow truncation.
    uint64_t temp = extended_immed + vcpu.state.register_file[operands.rs];
    // Now we truncate it into uint32_t.
    uint32_t address = temp;
    // Finally, we write the content of Rt to that memory location.
    vmemory_write_word(vcpu.state.register_file[operands.rt], address);
    log_debug(&vcpu.logger, "SW executed: Stored R%d(0x%08x) to memory address 0x%08x\n", operands.rt, vcpu.state.register_file[operands.rt], address);
}

void vcpu_insn_execute_lw(vCPU_INSN_OPERANDS_t operands)
{
    uint32_t extended_immed = IMMED_MASK & operands.immed;

    if (operands.immed & INSN15_MASK)
    {

        extended_immed |= (IMMED_MASK ^ WORD_HIGH_VALUES);
    }

    uint64_t temp = extended_immed + vcpu.state.register_file[operands.rs];
    uint32_t address = temp;

    vcpu.state.register_file[operands.rt] = vmemory_read_word(address);
    log_debug(&vcpu.logger, "LW executed: Loaded memory address 0x%08x into R%d(0x%08x)\n", address, operands.rt, vcpu.state.register_file[operands.rt]);
}

void vcpu_insn_execute_jalr(vCPU_INSN_OPERANDS_t operands)
{
    printf("JALR: R%d <-- [PC](0x%08x); PC <-- [R%d](0x%08x)\n", operands.rd, vcpu.state.register_file[REGISTER_ADDRESS_PC]-4, operands.rs, vcpu.state.register_file[operands.rs]);
    vcpu.state.register_file[operands.rd] = vcpu.state.register_file[REGISTER_ADDRESS_PC] - 4;
    vcpu.state.register_file[REGISTER_ADDRESS_PC] = vcpu.state.register_file[operands.rs];
    log_debug(&vcpu.logger, "Jumping to address 0x%08x from 0x%08x\n", vcpu.state.register_file[REGISTER_ADDRESS_PC], vcpu.state.register_file[operands.rd]);
}

void vcpu_insn_execute_ori(vCPU_INSN_OPERANDS_t operands)
{
    vcpu.state.register_file[operands.rt] = vcpu.state.register_file[operands.rs] | (operands.immed | WORD_LOW_VALUES);
    printf("ORI: 0x%08x | 0x%08x = 0x%08x\n", vcpu.state.register_file[operands.rs], operands.immed, vcpu.state.register_file[operands.rt]);
    log_debug(&vcpu.logger, "Executed ORI instruction: Set R%d to 0x%08x\n", operands.rt, vcpu.state.register_file[operands.rt]);
}

// Used to flush pipeline. Should rarely be necessary because the compiler will insert NOPs/stalls into the compiled code to ensure the pipeline is
// properly flushed.
void vcpu_flush_pipeline()
{
    vcpu.state.register_file[REGISTER_ADDRESS_IR] = 0;
    vcpu.state.decoded_insn.execute = vcpu_insn_execute_nop;
    vcpu.state.decoded_insn.operands = (vCPU_INSN_OPERANDS_t){0};

    log_debug(&vcpu.logger, "Pipeline flushed: NOP inserted into fetch and decode stages.\n");
}
