#include <stdlib.h>
#include <stdio.h>

#include "vtlb.h"

static TLB tlb;

void vtlb_init()
{
    tlb.size = TLB_SIZE;
    tlb.entries = calloc(TLB_SIZE, sizeof(TLB_entry));
}

uint32_t vtlb_translate(uint32_t vaddr, ACCESS_MODE access_mode, uint8_t *cacheable)
{
    // Look for matching entry in TLB.
    uint32_t vpn = (vaddr & ENTRYHI_VPN_MASK);
    uint32_t asid = (tlb.mmu_regs[MMU_REGISTER_ENTRYHI] & ENTRYHI_ASID_MASK);

    uint8_t index = -1;
    for (int i = 0; i < tlb.size; i++)
    {
        // First check match on VPN.
        if ((tlb.entries[i].EntryHi & ENTRYHI_VPN_MASK) == vpn)
        {
            // If global flag is set, we're done.
            if (tlb.entries[i].EntryLo & ENTRYLO_G_MASK)
            {
                index = i;
            }
            // If not, we must check ASID also.
            else if ((tlb.entries[i].EntryHi & ENTRYHI_ASID_MASK) == asid)
            {
                index = i;
            }
            // No match, continue search.
        }
    }

    // Check if entry is found.
    TLB_entry entry;
    if (index == -1)
    {
        // If entry not found, we must signal a TLB refill exception,
        // but first, we must find out if it's user triggered or not.
        // This is done by checking what segment we're trying to access.
        if (vaddr >= KUSEG_START_VADDR && vaddr <= KUSEG_END_VADDR)
        {
            // We're trying to access kuseg, so it's a user-access TLB refill exception.
            printf("User-access TLB refill exception for address: 0x%08x. Entry not found.\n", vaddr);
            return 0xFFFFFFFF;
        }
        else
        {
            // It's a kernel-access TLB refill exception.
            printf("Kernel-access TLB refill exception for address: 0x%08x. Entry not found.\n", vaddr);
            return 0xFFFFFFFF;
        }
    }
    else
    {
        entry = tlb.entries[index];
    }

    // If entry is not valid, signal TLB refill exception.
    if (!(entry.EntryLo & ENTRYLO_V_MASK))
    {
        if (vaddr >= KUSEG_START_VADDR && vaddr <= KUSEG_END_VADDR)
        {
            // We're trying to access kuseg, so it's a user-access TLB refill exception.
            printf("User-access TLB refill exception for address: 0x%08x. Entry not valid.\n", vaddr);
            return 0xFFFFFFFF;
        }
        else
        {
            // It's a kernel-access TLB refill exception.
            printf("Kernel-access TLB refill exception for address: 0x%08x. Entry not valid.\n", vaddr);
            return 0xFFFFFFFF;
        }
    }

    // If ACCESS_MODE_STORE and entry not dirty, signal TLB refill exception.
    if (access_mode == ACCESS_MODE_STORE && !(entry.EntryLo & ENTRYLO_D_MASK))
    {
        if (vaddr >= KUSEG_START_VADDR && vaddr <= KUSEG_END_VADDR)
        {
            // We're trying to access kuseg, so it's a user-access TLB refill exception.
            printf("User-access TLB refill exception for address: 0x%08x. Attempting to store, but entry not dirty.\n", vaddr);
            return 0xFFFFFFFF;
        }
        else
        {
            // It's a kernel-access TLB refill exception.
            printf("Kernel-access TLB refill exception for address: 0x%08x. Attempting to store, but entry not dirty.\n", vaddr);
            return 0xFFFFFFFF;
        }
    }

    // If everything is okay, set cacheable...
    if ((entry.EntryLo & ENTRYLO_N_MASK) != 0)
    {
        // N-bit = 1 => Non-cacheable
        *cacheable = 0;
    }
    else
    {
        // N-bit = 0 => Cacheable
        *cacheable = 1;
    }

    // calculate physical address by concatenating PFN with lower bits...
    uint32_t paddr = (entry.EntryLo & ENTRYLO_PFN_MASK) | (vaddr & VADDR_LOWER_BITS_MASK);
    // and return it.
    return paddr;
}

void vtlb_write(uint8_t index)
{
    tlb.entries[index].EntryHi = tlb.mmu_regs[MMU_REGISTER_ENTRYHI];
    tlb.entries[index].EntryLo = tlb.mmu_regs[MMU_REGISTER_ENTRYLO];
}

void vtlb_read(uint8_t index)
{
    tlb.mmu_regs[MMU_REGISTER_ENTRYHI] = tlb.entries[index].EntryHi;
    tlb.mmu_regs[MMU_REGISTER_ENTRYLO] = tlb.entries[index].EntryLo;
}

uint8_t vtlb_lookup()
{
    uint32_t vpn = tlb.mmu_regs[MMU_REGISTER_ENTRYHI] & ENTRYHI_VPN_MASK;
    uint32_t asid = tlb.mmu_regs[MMU_REGISTER_ENTRYHI] & ENTRYHI_ASID_MASK;

    uint8_t index = -1;

    for (uint8_t i = 0; i < TLB_SIZE; i++) {
        if ((tlb.entries[i].EntryHi & ENTRYHI_VPN_MASK) == vpn &&
            (tlb.entries[i].EntryHi & ENTRYHI_ASID_MASK) == asid) {
            index = i;
            break;
        }
    }

    return index;
}
