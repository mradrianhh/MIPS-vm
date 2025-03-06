# The main-initialization entry point.
.section .text.reset
.global _reset
.type _reset, @function
.align 4
_reset:
    # Set SP
    la      $sp, __StackTop       # Load address of Stack Top into SP
    la      $sp, __StackTop       # Load address of Stack Top into SP
	# Transfer control to the application entry point.
	j		_start # jump to _start
    # If control is ever received back, loop infinitely.
.L1:
    nop
    j	    .L1
