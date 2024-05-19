# The main-initialization entry point.
.section .text
.global _start
.type _start, @function
_start:
	# Transfer control to the application entry point.
	jal		main				# jump to main and save position to $ra
    move 	$a0, $v0
    # If control is ever received back, loop infinitely.
.L1:
    nop
    j	    .L1
    
