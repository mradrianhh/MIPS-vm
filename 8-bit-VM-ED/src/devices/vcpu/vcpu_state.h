#ifndef _8BITVM_VCPU_STATE_H_
#define _8BITVM_VCPU_STATE_H_

#include "vregisters.h"
#include "vcpu_decoder.h"

typedef struct vCPU_state vCPU_state_t;

struct vCPU_state
{
    GP_REGISTERS_t gp_registers;
    SPECIAL_REGISTERS_t special_registers;
    vCPU_INSN_t decoded_insn;
};

#endif