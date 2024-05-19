#include <stdint.h>

/* main-initialization routine. */
extern int _start();

/* External declaration for system initialization function                  */
extern void SystemInit(void);

extern uint32_t __data_load__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __StackTop;

typedef void( *pFunc )( void );

/* Forward declaration of the default fault handlers. */
void Default_Handler(void);
extern void Reset_Handler       (void) __attribute__((weak));

/* Interrupt vector table.  Note that the proper constructs must be placed on this to */
/* ensure that it ends up at physical address 0x0000.0000 or at the start of          */
/* the program if located at a start address other than 0.                            */
void (* const interruptVectors[])(void) __attribute__ ((section (".intvecs"))) =
{
    (pFunc)&__StackTop,                    /* The initial stack pointer */
    Reset_Handler,                         /* The reset handler         */
};

/* Forward declaration of the default fault handlers. */
/* This is the code that gets called when the processor first starts execution */
/* following a reset event.  Only the absolutely necessary set is performed,   */
/* after which the application supplied entry() routine is called.  Any fancy  */
/* actions (such as making decisions based on the reset cause register, and    */
/* resetting the bits in that register) are left solely in the hands of the    */
/* application.                                                                */
void Reset_Handler(void)
{
    uint32_t *pui32Src, *pui32Dest;

    //
    // Copy the data segment initializers from flash to SRAM.
    //
    pui32Src = &__data_load__;
    for(pui32Dest = &__data_start__; pui32Dest < &__data_end__; )
    {
        *pui32Dest++ = *pui32Src++;
    }

    /* Call system initialization routine */
    SystemInit();

    /* Jump to the main initialization routine. */
    _start();
}

/* This is the code that gets called when the processor receives an unexpected  */
/* interrupt.  This simply enters an infinite loop, preserving the system state */
/* for examination by a debugger.                                               */
void Default_Handler(void)
{
	/* Enter an infinite loop. */
	while(1)
	{
	}
}