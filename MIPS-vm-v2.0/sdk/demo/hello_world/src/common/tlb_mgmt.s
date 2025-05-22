    # Return EntryLo contents for the TLB at the index passed through $a0. 
    # 'c' callable as ret_tlblo(index).
    .section    .text
    .global     ret_tlblo
    .weak       ret_tlblo
    .type       ret_tlblo, @function
ret_tlblo:
    .set    noreorder
    mfc0    $t0, $12        # Save SR.
    nop                  
    and     $t0, ~(0x8000)  # Don't clear PE bit in SR(mask 0x80000).
    mtc0    $zero, $12      # Clear interrupts
    mfc0    $t1, $10        # Save PID/ASID.
    sll		$a0, 0x7		# Shift index-parameter(passed in a0) 7-bits to the left to align it with bits 13..8 in C0_INDEX register.
    mtc0    $a0, $0         # Write to index register.
    nop
    tlbr                    # Read TLB entry at index into EntryHi/EntryLo.
    nop
    mfc0    $v0, $2         # Load EntryLo0 into $v0
    mtc0    $t1, $10        # Restore PID/ASID.
    mtc0    $t0, $12        # Restore SR register.
    j		$ra				# Return to caller.
    nop
    .set    reorder

    # Return EntryLo contents for the TLB at the index passed through $a0. 
    .section    .text
    .global     ret_tlbhi
    .weak       ret_tlbhi
    .type       ret_tlbhi, @function
ret_tlbhi:
    .set    noreorder
    mfc0    $t0, $12        # Save SR.
    nop                  
    and     $t0, ~(0x8000)  # Don't clear PE bit in SR(mask 0x80000).
    mtc0    $zero, $12      # Clear interrupts
    mfc0    $t1, $10        # Save PID/ASID.
    sll		$a0, 0x7		# Shift index-parameter(passed in a0) 7-bits to the left to align it with bits 13..8 in C0_INDEX register.
    mtc0    $a0, $0         # Write to index register.
    nop
    tlbr                    # Read TLB entry at index into EntryHi/EntryLo.
    nop
    mfc0    $v0, $10         # Load EntryLo0 into $v0
    mtc0    $t1, $10        # Restore PID/ASID.
    mtc0    $t0, $12        # Restore SR register.
    j		$ra				# Return to caller.
    nop
    .set    reorder

    # Return TLB PID/ASID contained in current EntryHi.
    .section    .text
    .global     ret_tlbpid
    .weak       ret_tlbpid
    .type       ret_tlbpid, @function
ret_tlbpid:
    mfc0    $v0, $10        # Load the current PID/ASID from EntryHi.
    nop
    and     $v0, 0x1FE0     # Mask to extract the PID/ASID
    jr      $ra             # Return to caller
    nop
