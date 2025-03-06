# User program entry point.
.section .text
.global main
.type main, @function
.align 4
main:
    beq		$a0, $a1, .L4	    # If a == b, goto .L2(exit).
    sgt     $v0, $a1, $a0       # Is b > a?
    bne		$v0, $zero, .L3	    # Yes, subtract a from b(go to .L1).
    
    subu    $a0, $a0, $a1       # No, subtract b from a.
    b       main             # Branch to start and repeat.
    
.L3:
    subu	$a1, $a1, $a0		# Subtract a from b(a < b).
    b		main			    # Branch to start and repeat.

.L4:
    move 	$v0, $a0		    # Return a.
    jr		$ra			        # Return to caller.
