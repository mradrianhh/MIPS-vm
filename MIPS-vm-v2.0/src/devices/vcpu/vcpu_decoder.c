#include "vcpu_decoder.h"

static void init_decoder_map();
static vCPU_INSN_t vcpu_insn_decode_nop(uint8_t instruction);
static vCPU_INSN_t vcpu_insn_decode_add(uint8_t instruction);
static vCPU_INSN_t vcpu_insn_decode_mov(uint8_t instruction);
static vCPU_INSN_t vcpu_insn_decode_ldr(uint8_t instruction);
static vCPU_INSN_t vcpu_insn_decode_str(uint8_t instruction);

static vCPU_INSN_DECODER_MAP_t _decoder_map;
static vCPU_INSN_EXECUTER_MAP_t _executer_map;

void vcpu_decoder_init(vCPU_INSN_EXECUTER_MAP_t executer_map)
{
    _executer_map = executer_map;
    init_decoder_map();
}

vcpu_insn_decode_t vcpu_decoder_get(REGISTER_t ir)
{
    return _decoder_map.decoders[(ir & OPCODE_MASK) >> 4];
}

vCPU_INSN_t vcpu_decoder_decode(REGISTER_t ir)
{
    vcpu_insn_decode_t decode = vcpu_decoder_get(ir);
    return decode(ir);
}

void init_decoder_map()
{
    _decoder_map.nop = vcpu_insn_decode_nop;
    _decoder_map.add = vcpu_insn_decode_add;
    _decoder_map.mov = vcpu_insn_decode_mov;
    _decoder_map.ldr = vcpu_insn_decode_ldr;
    _decoder_map.str = vcpu_insn_decode_str;
}

vCPU_INSN_t vcpu_insn_decode_nop(uint8_t instruction)
{
    vCPU_INSN_t vcpu_insn;
    vcpu_insn.execute = _executer_map.executers[NOP];
    vcpu_insn.operands.operand1 = (instruction & DEST_MASK) >> 2;
    vcpu_insn.operands.operand2 = (instruction & SOURCE_MASK);
    vcpu_insn.ir = instruction;
    return vcpu_insn;
}

vCPU_INSN_t vcpu_insn_decode_add(uint8_t instruction)
{
    vCPU_INSN_t vcpu_insn;
    vcpu_insn.execute = _executer_map.executers[ADD];
    vcpu_insn.operands.operand1 = (instruction & DEST_MASK) >> 2;
    vcpu_insn.operands.operand2 = (instruction & SOURCE_MASK);
    vcpu_insn.ir = instruction;
    return vcpu_insn;
}

vCPU_INSN_t vcpu_insn_decode_mov(uint8_t instruction)
{
    vCPU_INSN_t vcpu_insn;
    vcpu_insn.execute = _executer_map.executers[MOV];
    vcpu_insn.operands.operand1 = (instruction & DEST_MASK) >> 2;
    vcpu_insn.operands.operand2 = (instruction & SOURCE_MASK);
    vcpu_insn.ir = instruction;
    return vcpu_insn;
}

vCPU_INSN_t vcpu_insn_decode_ldr(uint8_t instruction)
{
    vCPU_INSN_t vcpu_insn;
    vcpu_insn.execute = _executer_map.executers[LDR];
    vcpu_insn.operands.operand1 = (instruction & DEST_MASK) >> 2;
    vcpu_insn.operands.operand2 = (instruction & SOURCE_MASK);
    vcpu_insn.ir = instruction;
    return vcpu_insn;
}

vCPU_INSN_t vcpu_insn_decode_str(uint8_t instruction)
{
    vCPU_INSN_t vcpu_insn;
    vcpu_insn.execute = _executer_map.executers[STR];
    vcpu_insn.operands.operand1 = (instruction & DEST_MASK) >> 2;
    vcpu_insn.operands.operand2 = (instruction & SOURCE_MASK);
    vcpu_insn.ir = instruction;
    return vcpu_insn;
}
