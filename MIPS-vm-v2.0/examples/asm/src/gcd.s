.text
.global __start

__start:
    beq		$a0, $a1, .L2	    # If a == b, goto .L2(exit).
    sgt     $v0, $a1, $a0       # Is b > a?
    bne		$t0, $t1, .L1	    # Yes, subtract a from b(go to .L1).
    
    subu    $a0, $a0, $a1       # No, subtract b from a.
    b       __start             # Branch to start and repeaet.
    
.L1:
    subu	$a1, $a1, $a0		# Subtract a from b(a < b).
    b		__start			    # Branch to start and repeat.

.L2:
    move 	$v0, $a0		    # Return a.
    jr		$ra			        # Return to caller.
    