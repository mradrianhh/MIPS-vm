# Nested Vector Interrupt Controller

## Tasks

- [ ] Create a struct called INTERRUPT_EVENT that contains an uint8_t called INTERRUPT_ID and an uint8_t called device_id. 
- [ ] Create an asynchronous queue of INTERRUPT_EVENTs called INTERRUPT_EVENT_QUEUE.
- [ ] Create a function to write INTERRUPT_EVENTs to the INTERRUPT_EVENT_QUEUE.
- [ ] Create a function to read INTERRUPT_EVENTs from the INTERRUPT_EVENT_QUEUE.

## Implementation

The NVIC is implemented as an event system.

Each event corresponds to a specific interrupt, and each event handler will have the same basic functionality:

1. Push current context on the virtual stack. This allows the CPU to pick-up the work from where it was left off by preserving registers and, more specifically, the PC.
2. Point PC to address of interrupt handler in IVT.

This also means that for interrupt handling to function properly, an interrupt handler must pop the context off the stack, so that all the registers and the PC is restored.

Since each interrupt has a priority, this is translated to each event having a priority. The NVIC also needs a way to know the priority of the currently executing work. This is done through the [???] register. There is also a PRIMAST-register and a FAULTMASK-register, that are used to disable/enable different interrupts.

## Description

A NVIC has the following functionality:

- An IVT stored at a fixed position in memory, or at a position in memory marked by a pointer.
- Each interrupt is assigned a priority. The application code is running at the lowest priority, so interrupts arent blocked.
- Interrupts can interrupt other interrupts as long as they have higher priority.

When an interrupt is triggered, the context of the CPU is preserved on the stack and the PC is set to the address of the interrupt handler in memory. When the interrupt handler is finished with it's work, it pops the preserved context of the stack so the CPU can continue with it's previous work.

The side-effect of this is what we refer to as "nested" interrupt handling. By pushing the current context on the stack, we don't care what the CPU was previously doing. In other words, we are agnostic to the CPU's current work, we only care about the interrupt priority. This means that if the CPU is already processing an interrupt and another interrupt with a higher priority is triggered, the current context that we are pushing on the stack is the previous interrupt handler.

In practice, this means that the CPU is processing our application code, then an interrupt is triggered, we preserve the context and we enter the interrupt handler. Inside this interrupt handler, another interrupt is triggered with a higher priority. We know preserve the context again, and enter the new interrupt handler. After it's finished, we pop the preserved context and we continue the work in the previous interrupt handler. Then, we pop again, and we are back in our application code.

This is the functionality of a nested vector interrupt controller, also known as NVIC.

