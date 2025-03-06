#include <stdlib.h>
#include <stdio.h>
#include "vcpu_decoder.h"

static void init_decoder_maps();
static void decode_operands_rformat(uint32_t insn_raw, vCPU_INSN_t *insn);
static void decode_operands_iformat(uint32_t insn_raw, vCPU_INSN_t *insn);
static void decode_operands_jformat(uint32_t insn_raw, vCPU_INSN_t *insn);
static vCPU_INSN_t vcpu_insn_decode_rformat(uint32_t insn_raw);
static vCPU_INSN_t vcpu_insn_decode_lui(uint32_t insn_raw);
static vCPU_INSN_t vcpu_insn_decode_addiu(uint32_t insn_raw);
static vCPU_INSN_t vcpu_insn_decode_beq(uint32_t insn_raw);
static vCPU_INSN_t vcpu_insn_decode_j(uint32_t insn_raw);
static vCPU_INSN_t vcpu_insn_decode_jal(uint32_t insn_raw);
static vCPU_INSN_t vcpu_insn_decode_sw(uint32_t insn_raw);
static vCPU_INSN_t vcpu_insn_decode_lw(uint32_t insn_raw);
static vCPU_INSN_t vcpu_insn_decode_ori(uint32_t insn_raw);


static vcpu_insn_decode_t *_decoders;
static vcpu_insn_execute_t *_executers;
static vcpu_insn_execute_t *_rformat_executers;
// We need a reference to the PC when decoding branch/jump.
static REGISTER_t *_pc_ref; 

void vcpu_decoder_init(REGISTER_t *pc_ref, vcpu_insn_execute_t *executers, vcpu_insn_execute_t *rformat_executers)
{
    _pc_ref = pc_ref;
    _executers = executers;
    _rformat_executers = rformat_executers;
    init_decoder_maps();
}

vCPU_INSN_t vcpu_decoder_decode(REGISTER_t ir)
{
    vcpu_insn_decode_t decoder = _decoders[(ir & OPCODE_MASK) >> OPCODE_OFFSET];

    if(decoder == NULL)
    {
        printf("> Decoder not found for instruction 0x%08x. Generating NOP.\n", ir);
        return vcpu_insn_decode_rformat(0);
    }

    return decoder(ir);
}

void init_decoder_maps()
{
    _decoders = calloc(OPCODE_MAX_RANGE, sizeof(vcpu_insn_decode_t));
    _decoders[RFORMAT] = vcpu_insn_decode_rformat;
    _decoders[LUI] = vcpu_insn_decode_lui;
    _decoders[ADDIU] = vcpu_insn_decode_addiu;
    _decoders[BEQ] = vcpu_insn_decode_beq;
    _decoders[J] = vcpu_insn_decode_j;
    _decoders[JAL] = vcpu_insn_decode_jal;
    _decoders[SW] = vcpu_insn_decode_sw;
    _decoders[LW] = vcpu_insn_decode_lw;
    _decoders[ORI] = vcpu_insn_decode_ori;
}

void decode_operands_rformat(uint32_t insn_raw, vCPU_INSN_t *insn)
{
    insn->operands.rs = (insn_raw & RS_MASK) >> RS_OFFSET;
    insn->operands.rt = (insn_raw & RT_MASK) >> RT_OFFSET;
    insn->operands.rd = (insn_raw & RD_MASK) >> RD_OFFSET;
    insn->operands.shamt = (insn_raw & SHAMT_MASK) >> SHAMT_OFFSET;
}

void decode_operands_iformat(uint32_t insn_raw, vCPU_INSN_t *insn)
{
    insn->operands.rs = (insn_raw & RS_MASK) >> RS_OFFSET;
    insn->operands.rt = (insn_raw & RT_MASK) >> RT_OFFSET;
    insn->operands.immed = (insn_raw & IMMED_MASK) >> IMMED_OFFSET;
}

void decode_operands_jformat(uint32_t insn_raw, vCPU_INSN_t *insn)
{
    insn->operands.address = (insn_raw & ADDRESS_MASK) >> ADDRESS_OFFSET;
}

vCPU_INSN_t vcpu_insn_decode_rformat(uint32_t insn_raw)
{
    vCPU_INSN_t insn;

    vcpu_insn_execute_t executer = _rformat_executers[(insn_raw & FUNCT_MASK) >> FUNCT_OFFSET];

    if(executer == NULL)
    {
        printf("> Executer not found for instruction 0x%08x. Generating NOP.\n", insn_raw);
        insn.execute = _rformat_executers[NOP];
        insn.operands = (vCPU_INSN_OPERANDS_t){0};
    }
    else
    {
        insn.execute = executer;
        decode_operands_rformat(insn_raw, &insn);
    }

    return insn;
}

// Load upper immediate(I).
vCPU_INSN_t vcpu_insn_decode_lui(uint32_t insn_raw)
{
    vCPU_INSN_t insn;

    vcpu_insn_execute_t executer = _executers[LUI];

    if(executer == NULL)
    {
        printf("> Executer not found for instruction 0x%08x. Generating NOP.\n", insn_raw);
        insn.execute = _rformat_executers[NOP];
        insn.operands = (vCPU_INSN_OPERANDS_t){0};
    }
    else
    {
        insn.execute = executer;
        decode_operands_iformat(insn_raw, &insn);
    }

    return insn;
}

// Addition immediate without overflow(I).
vCPU_INSN_t vcpu_insn_decode_addiu(uint32_t insn_raw)
{
    vCPU_INSN_t insn;

    vcpu_insn_execute_t executer = _executers[ADDIU];

    if(executer == NULL)
    {
        printf("> Executer not found for instruction 0x%08x. Generating NOP.\n", insn_raw);
        insn.execute = _rformat_executers[NOP];
        insn.operands = (vCPU_INSN_OPERANDS_t){0};
    }
    else
    {
        insn.execute = executer;
        decode_operands_iformat(insn_raw, &insn);
    }

    return insn;
}

// Branch on equal(I).
vCPU_INSN_t vcpu_insn_decode_beq(uint32_t insn_raw)
{
    vCPU_INSN_t insn;

    vcpu_insn_execute_t executer = _executers[BEQ];

    if(executer == NULL)
    {
        printf("> Executer not found for instruction 0x%08x. Generating NOP.\n", insn_raw);
        insn.execute = _rformat_executers[NOP];
        insn.operands = (vCPU_INSN_OPERANDS_t){0};
    }
    else
    {
        insn.execute = executer;
        decode_operands_iformat(insn_raw, &insn);
        // We need to calculate the address here so it contains the correct address based on the offset and the current PC.
        insn.operands.address = *_pc_ref + (insn.operands.immed << 2);
    }
    
    return insn;
}

vCPU_INSN_t vcpu_insn_decode_j(uint32_t insn_raw)
{
    vCPU_INSN_t insn;

    vcpu_insn_execute_t executer = _executers[J];

    if(executer == NULL)
    {
        printf("> Executer not found for instruction 0x%08x. Generating NOP.\n", insn_raw);
        insn.execute = _rformat_executers[NOP];
        insn.operands = (vCPU_INSN_OPERANDS_t){0};
    }
    else
    {
        insn.execute = executer;
        decode_operands_jformat(insn_raw, &insn);
        // We need to recalculate the address = [PC_31..28] || [I_25..0] || 0^2
        insn.operands.address = (insn.operands.address << 2) | ((*_pc_ref-4) & PC_31_28_MASK);
    }

    return insn;
}

vCPU_INSN_t vcpu_insn_decode_jal(uint32_t insn_raw)
{
    vCPU_INSN_t insn;

    vcpu_insn_execute_t executer = _executers[JAL];

    if(executer == NULL)
    {
        printf("> Executer not found for instruction 0x%08x. Generating NOP.\n", insn_raw);
        insn.execute = _rformat_executers[NOP];
        insn.operands = (vCPU_INSN_OPERANDS_t){0};
    }
    else
    {
        insn.execute = executer;
        decode_operands_jformat(insn_raw, &insn);
        // We need to recalculate the address = [PC_31..28] || [I_25..0] || 0^2
        insn.operands.address = (insn.operands.address << 2) | ((*_pc_ref-4) & PC_31_28_MASK);
    }

    return insn;
}

// Store word(I).
vCPU_INSN_t vcpu_insn_decode_sw(uint32_t insn_raw)
{
    vCPU_INSN_t insn;

    vcpu_insn_execute_t executer = _executers[SW];

    if(executer == NULL)
    {
        printf("> Executer not found for instruction 0x%08x. Generating NOP.\n", insn_raw);
        insn.execute = _rformat_executers[NOP];
        insn.operands = (vCPU_INSN_OPERANDS_t){0};
    }
    else
    {
        insn.execute = executer;
        decode_operands_iformat(insn_raw, &insn);
    }

    return insn;
}

vCPU_INSN_t vcpu_insn_decode_lw(uint32_t insn_raw)
{
    vCPU_INSN_t insn;

    vcpu_insn_execute_t executer = _executers[LW];

    if(executer == NULL)
    {
        printf("> Executer not found for instruction 0x%08x. Generating NOP.\n", insn_raw);
        insn.execute = _rformat_executers[NOP];
        insn.operands = (vCPU_INSN_OPERANDS_t){0};
    }
    else
    {
        insn.execute = executer;
        decode_operands_iformat(insn_raw, &insn);
    }

    return insn;
}

vCPU_INSN_t vcpu_insn_decode_ori(uint32_t insn_raw)
{
    vCPU_INSN_t insn;

    vcpu_insn_execute_t executer = _executers[ORI];

    if(executer == NULL)
    {
        printf("> Executer not found for instruction 0x%08x. Generating NOP.\n", insn_raw);
        insn.execute = _rformat_executers[NOP];
        insn.operands = (vCPU_INSN_OPERANDS_t){0};
    }
    else
    {
        insn.execute = executer;
        decode_operands_iformat(insn_raw, &insn);
    }

    return insn;
}
