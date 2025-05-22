#ifndef _MIPSVM_GUEST_DEVICES_VMEMORY_VLTB_H_
#define _MIPSVM_GUEST_DEVICES_VMEMORY_VTLB_H_

#include <stdint.h>

#include "mapping.h"

/*
*   Contains TLB structure with functions for reading and writing.
*/

// MMU register masks and offsets.
#define ENTRYHI_VPN_MASK        (0xFFFFF000)
#define ENTRYHI_VPN_OFFSET      (12)
#define ENTRYHI_ASID_MASK       (0x00000FC0)
#define ENTRYHI_ASID_OFFSET     (6)

#define ENTRYLO_PFN_MASK        (0xFFFFF000)
#define ENTRYLO_PFN_OFFSET      (12)
#define ENTRYLO_N_MASK          (0x00000800)
#define ENTRYLO_N_OFFSET        (11)
#define ENTRYLO_D_MASK          (0x00000400)
#define ENTRYLO_D_OFFSET        (10)
#define ENTRYLO_V_MASK          (0x00000200)
#define ENTRYLO_V_OFFSET        (9)
#define ENTRYLO_G_MASK          (0x00000100)
#define ENTRYLO_G_OFFSET        (8)

#define INDEX_P_MASK            (0x80000000)
#define INDEX_P_OFFSET          (31)
#define INDEX_INDEX_MASK        (0x00003F00)
#define INDEX_INDEX_OFFSET      (8)

#define RANDOM_RANDOM_MASK      (0x00003F00)
#define RANDOM_RANDOM_OFFSET    (8)

#define CONTEXT_PTEBASE_MASK    (0xFFE00000)
#define CONTEXT_PTEBASE_OFFSET  (21)
#define CONTEXT_BADVPN_MASK     (0x001FFFFC)
#define CONTEXT_BADVPN_OFFSET   (2)

// Translation constants.
#define VADDR_VPN_MASK          (0xFFFFF000)
#define VADDR_LOWER_BITS_MASK   (0x00000FFF)

#define MMU_REG_COUNT (5)
enum MMU_REGS
{
    MMU_REGISTER_ENTRYHI = 10,
    MMU_REGISTER_ENTRYLO = 2,
    MMU_REGISTER_INDEX = 0,
    MMU_REGISTER_RANDOM = 1,
    MMU_REGISTER_CONTEXT = 4,
};

typedef struct TLB_entry {
    uint32_t EntryHi;
    uint32_t EntryLo;
} TLB_entry;

#define TLB_SIZE (64)
typedef struct TLB {
    TLB_entry *entries;
    uint8_t size;
    uint32_t mmu_regs[MMU_REG_COUNT];
} TLB;

void vtlb_init();
// Main functionality. vtlb_translate returns the physical address and whether the entry is cacheable.
uint32_t vtlb_translate(uint32_t vaddr, ACCESS_MODE access_mode, uint8_t *cacheable);

// Functions for updating structure:

// Used by tlbwi and tlbwr to copy contents from EntryHi/EntryLo registers to entry at index.
void vtlb_write(uint8_t index);

// Used by tlbr to read the contents from entry at index to EntryHi/EntryLo registers.
void vtlb_read(uint8_t index);

// Used by tlbp to find the index of the entry whose VPN and ASID matches current EntryHi register.
// Returns -1 if no match.
uint8_t vtlb_lookup();

#endif
