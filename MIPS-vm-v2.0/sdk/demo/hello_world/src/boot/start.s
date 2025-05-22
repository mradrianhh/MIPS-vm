    # See: https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD00901-2B-CPS-APP-01.03.pdf

    .set        noreorder
    .set        noat                     

    .section    .text.Reset_Handler
    .global     Reset_Handler
    .weak       Reset_Handler
    .type       Reset_Handler, @function
Reset_Handler:
    la      $a2,            init_common_resources
    jr      $a2
    nop

init_common_resources:
    la      $a2,            init_gpr                # Fill register file with boot info.
    jalr    $a2
    nop

    la      $a2,            set_gpr_boot_values     # Set register info
    jalr    $a2
    nop

    la      $a2,            init_cp0                # TODO: Initialize CP0 registers.
    jalr    $a2
    nop

    la      $a2,            init_tlb                # TODO: Initialize the TLB.
    jalr    $a2
    nop

    la      $a2,            copy_c2_ram             # Copy code/data to RAM and zero bss.
    jalr    $a2
    nop

    la      $ra,            main                # We should give control to main here, but for now we just loop.
    jr      $ra
    nop

all_done:
    j       all_done
    nop

init_gpr:
    # Initialize the general purpose registers and any shadow register sets.
    # Although not necessary, register initialization may be useful during debugging
    # to see if a register has ever been written


    # Initialize register sets
    li      $1,             0xdeadbeef              # (0xdeadbeef stands out, kseg2 mapped, odd.)

    move    $2,             $1
    move    $3,             $1
    move    $4,             $1
    move    $5,             $1
    move    $6,             $1
    move    $7,             $1
    move    $8,             $1
    move    $9,             $1
    move    $10,            $1
    move    $11,            $1
    move    $12,            $1
    move    $13,            $1
    move    $14,            $1
    move    $15,            $1
    move    $16,            $1
    move    $17,            $1
    move    $18,            $1
    move    $19,            $1
    move    $20,            $1
    move    $21,            $1
    move    $22,            $1
    move    $23,            $1
    move    $24,            $1
    move    $25,            $1
    move    $26,            $1
    move    $27,            $1
    move    $28,            $1
    move    $29,            $1
    move    $30,            $1
    # We can't initialize $ra since we need to return.
    # move	$31, $1

    jr      $ra                                     # Return to caller.
    nop

set_gpr_boot_values:
    li      $sp,            __StackTop              # Load value of stack base address into stack pointer.
    jr      $ra                                     # Return.
    nop

init_cp0:
    jr      $ra                                     # jump to $ra
    nop

init_tlb:
    mfc0    $t0, $10                                # Save ASID.
    mtc0    $zero, $2                               # tlblo = !valid
    li      $a1, 64 << 8                            # Index starts at 63.
    li      $a0, 0xA0000000                         # Used to write impossible VPN to EntryHi.

    .set    noreorder
1:
    subu    $a1, 1 << 8                             # Decrease a1
    mtc0    $a1, $0                                 # Write $a1 index to C0_INDEX.
    mtc0    $a0, $10                                # Write impossible VPN to EntryHi.
    bnez    $a1, 1b                                 # Branch if not zero to loop back.
    tlbwi                                           # BDSLOT.
    .set reorder
    
    mtc0    $t0, $10                                # Restore ASID.
    jr      $ra                                     # jump to $ra
    nop

copy_c2_ram:
    jr      $ra                                     # jump to $ra
    nop

