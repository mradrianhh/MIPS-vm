#ifndef _8BITVM_VCPU_STATE_H_
#define _8BITVM_VCPU_STATE_H_

#include "vcpu_registers.h"
#include "vcpu_decoder.h"

typedef struct vCPU_state vCPU_state_t;

struct vCPU_state
{
    REGISTER_FILE_t register_file;
    vCPU_INSN_t decoded_insn;
};

#endif