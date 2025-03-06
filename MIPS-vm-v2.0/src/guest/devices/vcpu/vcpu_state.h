#ifndef _MIPSVM_GUEST_DEVICES_VCPU_STATE_H_
#define _MIPSVM_GUEST_DEVICES_VCPU_STATE_H_

#include "vcpu_registers.h"
#include "vcpu_decoder.h"
#include "guest/devices/vmemory/vmemory.h"

typedef struct vCPU_state vCPU_state_t;

struct vCPU_state
{
    uint32_t decode_insn;
    REGISTER_FILE_t register_file;
    vCPU_INSN_t decoded_insn;
};

#endif
