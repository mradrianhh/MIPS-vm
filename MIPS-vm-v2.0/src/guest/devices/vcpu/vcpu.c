#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "guest/common/device_table/device_table.h"
#include "guest/common/events/events.h"
#include "guest/devices/vmemory/vmemory.h"
#include "guest/devices/vmemory/vtlb.h"
#include "guest/devices/vclock/EdgeChangedEvent.h"
#include "guest/devices/vmemory/mapping.h"

#include "vcpu.h"

static vCPU_t vcpu;

uint8_t format_table[OPCODE_RANGE] = {
    [0] = INSN_FORMAT_RFORMAT,
    [16] = INSN_FORMAT_RFORMAT,
    [17] = INSN_FORMAT_RFORMAT,
    [8] = INSN_FORMAT_IFORMAT,
    [9] = INSN_FORMAT_IFORMAT,
    [10] = INSN_FORMAT_IFORMAT,
    [11] = INSN_FORMAT_IFORMAT,
    [12] = INSN_FORMAT_IFORMAT,
    [13] = INSN_FORMAT_IFORMAT,
    [14] = INSN_FORMAT_IFORMAT,
    [35] = INSN_FORMAT_IFORMAT,
    [43] = INSN_FORMAT_IFORMAT,
    [36] = INSN_FORMAT_IFORMAT,
    [32] = INSN_FORMAT_IFORMAT,
    [40] = INSN_FORMAT_IFORMAT,
    [15] = INSN_FORMAT_IFORMAT,
    [49] = INSN_FORMAT_IFORMAT,
    [57] = INSN_FORMAT_IFORMAT,
    [4] = INSN_FORMAT_IFORMAT,
    [5] = INSN_FORMAT_IFORMAT,
    [6] = INSN_FORMAT_IFORMAT,
    [7] = INSN_FORMAT_IFORMAT,
    [1] = INSN_FORMAT_IFORMAT,
    [2] = INSN_FORMAT_JFORMAT,
    [3] = INSN_FORMAT_JFORMAT,
};

INSN_entry insn_table[INSN_RANGE] = {
    [NOP] = {.opcode = 0, .function = 0, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "NOP\0"},
    [ADD] = {.opcode = 0, .function = 32, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "ADD\0"},
    [ADDU] = {.opcode = 0, .function = 33, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "ADDU\0"},
    [SUB] = {.opcode = 0, .function = 34, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "SUB\0"},
    [SUBU] = {.opcode = 0, .function = 35, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "SUBU\0"},
    [MUL] = {.opcode = 0, .function = 24, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "MUL\0"},
    [MULU] = {.opcode = 0, .function = 25, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "MULU\0"},
    [DIV] = {.opcode = 0, .function = 26, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "DIV\0"},
    [DIVU] = {.opcode = 0, .function = 27, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "DIVU\0"},
    [SLT] = {.opcode = 0, .function = 42, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "SLT\0"},
    [SLTU] = {.opcode = 0, .function = 43, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "SLTU\0"},
    [AND] = {.opcode = 0, .function = 36, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "AND\0"},
    [OR] = {.opcode = 0, .function = 37, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "OR\0"},
    [NOR] = {.opcode = 0, .function = 39, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "NOR\0"},
    [XOR] = {.opcode = 0, .function = 40, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "XOR\0"},
    [ADDI] = {.opcode = 8, .function = 0, .format = INSN_FORMAT_IFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "ADDI\0"},
    [ADDIU] = {.opcode = 9, .function = 0, .format = INSN_FORMAT_IFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "ADDIU\0"},
    [JR] = {.opcode = 0, .function = 8, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_BRANCH, .mnemonic = "JR\0"},
    [LUI] = {.opcode = 15, .function = 0, .format = INSN_FORMAT_IFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "LUI\0"},
    [BEQ] = {.opcode = 4, .function = 0, .format = INSN_FORMAT_IFORMAT, .insn_type = INSN_TYPE_BRANCH, .mnemonic = "BEQ\0"},
    [J] = {.opcode = 2, .function = 0, .format = INSN_FORMAT_JFORMAT, .insn_type = INSN_TYPE_BRANCH, .mnemonic = "J\0"},
    [JAL] = {.opcode = 3, .function = 0, .format = INSN_FORMAT_JFORMAT, .insn_type = INSN_TYPE_BRANCH, .mnemonic = "JAL\0"},
    [SW] = {.opcode = 43, .function = 0, .format = INSN_FORMAT_IFORMAT, .insn_type = INSN_TYPE_MEM_STORE, .mnemonic = "SW\0"},
    [LW] = {.opcode = 35, .function = 0, .format = INSN_FORMAT_IFORMAT, .insn_type = INSN_TYPE_MEM_LOAD, .mnemonic = "LW\0"},
    [JALR] = {.opcode = 0, .function = 9, .format = INSN_FORMAT_RFORMAT, .insn_type = INSN_TYPE_BRANCH, .mnemonic = "JALR\0"},
    [ORI] = {.opcode = 13, .function = 0, .format = INSN_FORMAT_IFORMAT, .insn_type = INSN_TYPE_ALU, .mnemonic = "ORI\0"},
};

/*
 * Pipeline
 */
static void handle_rising();
static void if_pipestage();
static void rd_pipestage();
static void alu_pipestage();
static void mem_pipestage();
static void wb_pipestage();

/*
 * Executer table
 */
static vcpu_insn_execute_t executers[INSN_RANGE];
static void vcpu_insn_execute_nop();
static void vcpu_insn_execute_add();
static void vcpu_insn_execute_addu();
static void vcpu_insn_execute_jr();
static void vcpu_insn_execute_lui();
static void vcpu_insn_execute_addiu();
static void vcpu_insn_execute_beq();
static void vcpu_insn_execute_j();
static void vcpu_insn_execute_jal();
static void vcpu_insn_execute_or();
static void vcpu_insn_execute_sw();
static void vcpu_insn_execute_lw();
static void vcpu_insn_execute_jalr();
static void vcpu_insn_execute_ori();

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

    vcpu.turned_on = 0;

    // Store handle to executer and rformat_executers and free on shutdown.
    executers[LUI] = vcpu_insn_execute_lui;
    executers[ADDIU] = vcpu_insn_execute_addiu;
    executers[BEQ] = vcpu_insn_execute_beq;
    executers[J] = vcpu_insn_execute_j;
    executers[JAL] = vcpu_insn_execute_jal;
    executers[SW] = vcpu_insn_execute_sw;
    executers[LW] = vcpu_insn_execute_lw;
    executers[ORI] = vcpu_insn_execute_ori;
    executers[ADD] = vcpu_insn_execute_add;
    executers[ADDU] = vcpu_insn_execute_addu;
    executers[NOP] = vcpu_insn_execute_nop;
    executers[JR] = vcpu_insn_execute_jr;
    executers[OR] = vcpu_insn_execute_or;
    executers[JALR] = vcpu_insn_execute_jalr;
}

void vcpu_reset()
{
    vcpu.turned_on = 1;
    // Reset state.
    vcpu.pc = RESET_VECTOR_VADDR;
}

void vcpu_update(const void *args)
{
    const EdgeChangedEventArgs_t *e = (const EdgeChangedEventArgs_t *)args;
    if (e->edge)
    {
        handle_rising();
    }
}

void vcpu_shutdown()
{
    log_info(&vcpu.logger, "Shutting down.\n");
    logger_shutdown(&vcpu.logger);
    vcache_destroy(&vcpu.icache);
    vcache_destroy(&vcpu.dcache);
}

uint32_t *vcpu_get_pc_ref()
{
    return &vcpu.pc;
}

//
// Internal functions
// ------------------
//

void handle_rising()
{
    wb_pipestage();
    mem_pipestage();
    alu_pipestage();
    rd_pipestage();
    if_pipestage();
}

void if_pipestage()
{
    // We never have to stall an instruction fetch.
    // Reset buffer.
    vcpu.IF_ID.insn = 0;
    vcpu.IF_ID.pc = 0;
    // Default forwarding
    vcpu.IF_ID.pc = vcpu.pc;
    // We should read from cache here, but how does the CPU determine when to read from cache, when to use TLB and when to read from memory?
    // We need to translate the address first.
    uint8_t cacheable;
    uint32_t paddr = address_translation(vcpu.pc, ACCESS_MODE_LOAD, &cacheable);
    if(cacheable)
    {
        vcpu.IF_ID.insn = vcache_load(&vcpu.icache, paddr);
    }
    else
    {
        vcpu.IF_ID.insn = vmemory_read_word(paddr);
    }
    printf("Fetched instruction 0x%08x at vaddr: 0x%08x, paddr: 0x%08x\n", vcpu.IF_ID.insn, vcpu.pc, paddr);
    vcpu.pc += 4;
}

void rd_pipestage()
{
    // Here we need to check if we are cleared to proceed. If another instruction is attempting to update a register we are reading from, we have a RAW conflict,
    // and we must stall until it's passed the WB stage.
    
    printf("RD(0x%08x)\n", vcpu.IF_ID.insn);
    // Reset ID_EX buffer.
    vcpu.ID_EX.function = 0;
    vcpu.ID_EX.immed = 0;
    vcpu.ID_EX.insn = 0;
    vcpu.ID_EX.insn_id = 0;
    vcpu.ID_EX.opcode = 0;
    vcpu.ID_EX.pc = 0;
    vcpu.ID_EX.rd = 0;
    vcpu.ID_EX.rs = 0;
    vcpu.ID_EX.rt = 0;
    // Default forwarding.
    vcpu.ID_EX.insn = vcpu.IF_ID.insn;
    vcpu.ID_EX.pc = vcpu.IF_ID.pc;

    // Decode the instruction and forward operands.
    vcpu.ID_EX.opcode = (vcpu.IF_ID.insn & OPCODE_MASK) >> OPCODE_OFFSET;

    uint8_t format = format_table[vcpu.ID_EX.opcode];

    switch (format)
    {
    case INSN_FORMAT_RFORMAT:
        vcpu.ID_EX.rt = (vcpu.IF_ID.insn & RT_MASK) >> RT_OFFSET;
        vcpu.ID_EX.rs = (vcpu.IF_ID.insn & RS_MASK) >> RS_OFFSET;
        vcpu.ID_EX.rd = (vcpu.IF_ID.insn & RD_MASK) >> RD_OFFSET;
        vcpu.ID_EX.function = (vcpu.IF_ID.insn & FUNCT_MASK) >> FUNCT_OFFSET;
        break;
    case INSN_FORMAT_IFORMAT:
        vcpu.ID_EX.rs = (vcpu.IF_ID.insn & RS_MASK) >> RS_OFFSET;
        vcpu.ID_EX.rt = (vcpu.IF_ID.insn & RT_MASK) >> RT_OFFSET;
        vcpu.ID_EX.immed = (vcpu.IF_ID.insn & IMMED_MASK) >> IMMED_OFFSET;
        break;
    case INSN_FORMAT_JFORMAT:
        vcpu.ID_EX.immed = (vcpu.IF_ID.insn & ADDRESS_MASK) >> ADDRESS_OFFSET;
        break;
    default:
        fprintf(stderr, "Opcode(0x%08x) not known in format table.\n", vcpu.ID_EX.opcode);
        exit(EXIT_FAILURE);
    }
}

void alu_pipestage()
{
    printf("ALU(0x%08x)\n", vcpu.ID_EX.insn);
    // Reset EX_MEM buffer.
    vcpu.EX_MEM.condition_flag = 0;
    vcpu.EX_MEM.dest_reg = 0;
    vcpu.EX_MEM.insn = 0;
    vcpu.EX_MEM.insn_info = (INSN_entry){0};
    vcpu.EX_MEM.result = 0;
    vcpu.EX_MEM.store_value = 0;
    vcpu.EX_MEM.target_address = 0;
    // Default forwards
    vcpu.EX_MEM.insn = vcpu.ID_EX.insn;

    for (int i = 0; i < INSN_RANGE; i++)
    {
        if (insn_table[i].insn_id == vcpu.ID_EX.insn_id)
        {
            vcpu.EX_MEM.insn_info = insn_table[i];
            executers[i](&vcpu);
            return;
        }
    }
}

void mem_pipestage()
{
    printf("MEM(0x%08x)\n", vcpu.EX_MEM.insn);
    // Default forwards
    vcpu.MEM_WB.dest_reg = vcpu.EX_MEM.dest_reg;
    vcpu.MEM_WB.insn = vcpu.EX_MEM.insn;
    // We might update result afterwards if memory load.
    vcpu.MEM_WB.result = vcpu.EX_MEM.result;
    // Check if branch, if so, deliver new target address and condition to IF.
    uint8_t cacheable;
    uint32_t paddr;
    switch (vcpu.EX_MEM.insn_info.insn_type)
    {
    case INSN_TYPE_BRANCH:
        if(vcpu.EX_MEM.condition_flag)
        {
            printf("Setting PC to 0x%08x\n", vcpu.EX_MEM.target_address);
            vcpu.pc = vcpu.EX_MEM.target_address;
        }
        break;
    case INSN_TYPE_MEM_LOAD:
        paddr = address_translation(vcpu.EX_MEM.target_address, ACCESS_MODE_LOAD, &cacheable);
        if(cacheable)
        {            
            vcpu.MEM_WB.result = vcache_load(&vcpu.dcache, paddr);
        }
        else
        {
            vcpu.MEM_WB.result = vmemory_read_word(paddr);
        }
        break;
    case INSN_TYPE_MEM_STORE:
        paddr = address_translation(vcpu.EX_MEM.target_address, ACCESS_MODE_STORE, &cacheable);
        if(cacheable)
        {
            vcache_store(&vcpu.dcache, vcpu.EX_MEM.store_value, paddr);
        }
        else
        {
            vmemory_write_word(vcpu.EX_MEM.store_value, paddr);
        }
        break;
    default:
        break;
    }
}

void wb_pipestage()
{
    printf("WB(0x%08x)\n", vcpu.MEM_WB.insn);
    // We write results from single-cycle and two-cycle latency instructions here to the register file entry rd.
    vcpu.gprs[vcpu.MEM_WB.dest_reg] = vcpu.MEM_WB.result;
    printf("Setting R%d(0x%08x) to 0x%08x\n", vcpu.MEM_WB.dest_reg, vcpu.gprs[vcpu.MEM_WB.dest_reg], vcpu.MEM_WB.result);
}

void vcpu_insn_execute_nop()
{
    printf("NOP\n");
}

void vcpu_insn_execute_add()
{
    printf("ADD\n");
    vcpu.EX_MEM.dest_reg = vcpu.ID_EX.rd;
    vcpu.EX_MEM.result = vcpu.gprs[vcpu.ID_EX.rs] + vcpu.gprs[vcpu.ID_EX.rt];
}

void vcpu_insn_execute_addu()
{
    printf("ADDU\n");
    vcpu.EX_MEM.dest_reg = vcpu.ID_EX.rd;
    vcpu.EX_MEM.result = vcpu.gprs[vcpu.ID_EX.rs] + vcpu.gprs[vcpu.ID_EX.rt];
}

void vcpu_insn_execute_jr()
{
    printf("JR\n");
    vcpu.EX_MEM.target_address = vcpu.gprs[vcpu.ID_EX.rs];
    vcpu.EX_MEM.condition_flag = 1;
}

void vcpu_insn_execute_lui()
{
    printf("LUI\n");
    vcpu.EX_MEM.result = vcpu.ID_EX.immed << 16;
    vcpu.EX_MEM.dest_reg = vcpu.ID_EX.rt;
}

void vcpu_insn_execute_addiu()
{
    uint32_t extended_immed;
    // We need to sign extend. First, append leading zeroes.
    extended_immed = IMMED_MASK & vcpu.ID_EX.immed;
    // Check if we should extend with 1's.
    if (vcpu.ID_EX.immed & INSN15_MASK)
    {
        // We flip the IMMED_MASK with an XOR so we can flip the upper 16 bits to 1's with an OR.
        extended_immed |= (IMMED_MASK ^ WORD_HIGH_VALUES);
    }

    // Move via uint64_t to prevent wrap-around and allow truncation.
    uint64_t temp = vcpu.gprs[vcpu.ID_EX.rs] + extended_immed;
    vcpu.EX_MEM.result = temp;
    vcpu.EX_MEM.dest_reg = vcpu.ID_EX.rt;
    printf("ADDIU: set R%d to 0x%08x + 0x%08x = 0x%08x\n", vcpu.EX_MEM.dest_reg, vcpu.gprs[vcpu.ID_EX.rs], extended_immed, vcpu.EX_MEM.result);
}

void vcpu_insn_execute_beq()
{
    printf("BEQ\n");
    // Set condition flag = rs == rt
    vcpu.EX_MEM.condition_flag = vcpu.gprs[vcpu.ID_EX.rs] == vcpu.gprs[vcpu.ID_EX.rt];
    // Set target address = [PC] + 4 + 4 * offset(immed).
    vcpu.EX_MEM.result = vcpu.ID_EX.pc + 4 + 4 * vcpu.ID_EX.immed;
}

void vcpu_insn_execute_j()
{
    printf("J\n");
    // Set condition = true(1).
    vcpu.EX_MEM.condition_flag = 1;
    // Set target address = ([PC_31..28] || (IMMED << 2)).
    // TODO: How should we set PC? We should not put it in result.
    // A branch has one delay slot, meaning that from the point it's executed, there is one stage until
    // PC is updated and we can fetch the correct instruction.
    vcpu.EX_MEM.target_address = 0 || (vcpu.ID_EX.pc & PC_31_28_MASK) || (vcpu.ID_EX.immed << 2);
}

void vcpu_insn_execute_jal()
{
    printf("JAL\n");
    // $ra = [PC] + 4;
    vcpu.EX_MEM.dest_reg = vcpu.gprs[REGISTER_RA];
    vcpu.EX_MEM.result = vcpu.ID_EX.pc + 4;
    // Set target address = ([PC_31..28] || (IMMED << 2)).
    vcpu.EX_MEM.target_address = 0 || (vcpu.ID_EX.pc & PC_31_28_MASK) || (vcpu.ID_EX.immed << 2);
    vcpu.EX_MEM.condition_flag = 1;
}

void vcpu_insn_execute_or()
{
    printf("OR\n");
    vcpu.EX_MEM.dest_reg = vcpu.ID_EX.rd;
    vcpu.EX_MEM.result = vcpu.gprs[vcpu.ID_EX.rs] | vcpu.gprs[vcpu.ID_EX.rt];
}

void vcpu_insn_execute_sw()
{
    printf("SW\n");
    // We need to calculate memory address by sign-extending offset and adding it with
    // the content of Rs.
    // First, append leading zeroes.
    uint32_t extended_immed = IMMED_MASK & vcpu.ID_EX.immed;
    // Check if we should extend with 1's.
    if (vcpu.ID_EX.immed & INSN15_MASK)
    {
        // We flip the IMMED_MASK with an XOR so we can flip the upper 16 bits to 1's with an OR.
        extended_immed |= (IMMED_MASK ^ WORD_HIGH_VALUES);
    }
    // We have to add the extended offset with the content of Rs, but to prevent
    // C's wraparound, we do it via uint64_t to allow truncation.
    uint64_t temp = extended_immed + vcpu.gprs[vcpu.ID_EX.rs];
    // Now we truncate it into uint32_t and store the address in target_address.
    vcpu.EX_MEM.target_address = temp;
    // Set store value.
    vcpu.EX_MEM.store_value = vcpu.gprs[vcpu.ID_EX.rt];
}

void vcpu_insn_execute_lw()
{
    printf("LW\n");
    // Calculate load address.
    uint32_t extended_immed = IMMED_MASK & vcpu.ID_EX.immed;

    if (vcpu.ID_EX.immed & INSN15_MASK)
    {

        extended_immed |= (IMMED_MASK ^ WORD_HIGH_VALUES);
    }

    uint64_t temp = extended_immed + vcpu.gprs[vcpu.ID_EX.rs];
    vcpu.EX_MEM.target_address = temp;
    // Set destination register.
    vcpu.EX_MEM.dest_reg = vcpu.ID_EX.rt;
}

void vcpu_insn_execute_jalr()
{
    printf("JALR\n");
    // Store [PC] + 4 in Rd.
    vcpu.EX_MEM.dest_reg = vcpu.ID_EX.rd;
    vcpu.EX_MEM.result = vcpu.ID_EX.pc + 4;
    // Pass [Rs] to PC.
    vcpu.EX_MEM.target_address = vcpu.gprs[vcpu.ID_EX.rs];
    vcpu.EX_MEM.condition_flag = 1;
}

void vcpu_insn_execute_ori()
{
    printf("ORI\n");
    // Set destination register.
    vcpu.EX_MEM.dest_reg = vcpu.ID_EX.rt;
    // Set result.
    vcpu.EX_MEM.result = vcpu.gprs[vcpu.ID_EX.rs] | (vcpu.ID_EX.immed | WORD_LOW_VALUES);
}
