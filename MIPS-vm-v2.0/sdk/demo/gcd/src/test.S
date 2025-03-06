# The main-initialization entry point.
.section .text
.global _start
.type _start, @function
.align 4
_start:
	# Transfer control to the application entry point.
	jal     _it_signal
    jal		main				# jump to main and save position to $ra
    move 	$a0, $v0
    # If control is ever received back, loop infinitely.
.L1:
    nop
    j	    .L1

#include "def.h"

.section .text
.type _it_signal, @function
.align 4
    PUSHRA
    POPRA 
    j   ra
    
