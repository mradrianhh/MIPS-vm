#include <stdlib.h>
#include <stdio.h>

#include "mapping.h"
#include "vtlb.h"

uint32_t address_translation(uint32_t vaddr, ACCESS_MODE access_mode, uint8_t *cacheable)
{
    uint32_t paddr;
    // Kuseg is mapped via TLB if TLB is present.
    // If not, they are transformed by adding a 1GB offset.
    if(vaddr >= KUSEG_START_VADDR && vaddr <= KUSEG_END_VADDR)
    {
        if(TLB_PRESENT)
        {
            paddr = vtlb_translate(vaddr, access_mode, cacheable);
        }
        else
        {
            // Add 1GB offset.
            paddr = vaddr + 0x00100000;
            *cacheable = 1;
        }
    }
    // kseg0 is unmapped and transformed by stripping top bit. 
    // Addresses in this region are always accessed through the cache.
    else if(vaddr >= KSEG0_START_VADDR && vaddr <= KSEG0_END_VADDR)
    {
        paddr = vaddr & KSEG0_TRANSFORM_MASK;
        *cacheable = 1;
    }
    // kseg1 is unmapped and transformed by stripping top three bits. 
    // This is a duplicate physical mapping of kseg0, but will not use the cache.
    else if(vaddr >= KSEG1_START_VADDR && vaddr <= KSEG1_END_VADDR)
    {
        paddr = vaddr & KSEG1_TRANSFORM_MASK;
        *cacheable = 0;
    }
    // kseg2 is mapped via TLB if present.
    // If not, physical addresses are the same as program addresses.
    else if(vaddr >= KSEG2_START_VADDR && vaddr <= KSEG2_END_VADDR)
    {
        if(TLB_PRESENT)
        {
            paddr = vtlb_translate(vaddr, access_mode, cacheable);
        }
        else
        {
            paddr = vaddr;
            *cacheable = 1;
        }
    }
    else
    {
        fprintf(stderr, "vaddr 0x%08x out of range. Exceeds size of virtual memory map.\n", vaddr);
        exit(EXIT_FAILURE);
    }

    return paddr;
}
