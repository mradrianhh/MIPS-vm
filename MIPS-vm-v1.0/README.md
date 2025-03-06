# 8-bit VM

## Specifications

- 1 byte page size.
- 1 byte word size.
- 1 byte instruction size.
- Allows a maximum of 256 bytes of addressable memory.

## Registers

The machine has 4 GPRs(r0-r3), PC, LR, SP, IR, MAR, MDR.

## Instruction Set Architecture

- MOV Rd, Rn ; Move value from Rn to Rd. 0000ddss.

- MOV Rd, #immed ; Move #immed to Rd. 0001ddss.

- LDR Rd, [Rn{, #offset}] ; Read word from address Rn + #offset, and store the value in Rd.

- STR Rd, [Rn{, #offset}] ; Store word from Rd in address Rn + #offset.

- ADD Rd, Rn, Rm ; Rd = Rn + Rm.

- SUB Rd, Rn, Rm ; Rd = Rn - Rm.

## Tasks and planning

- [x] Allocate a virtual address space to hold the guest memory in host RAM.
- [x] Allocate registers per vCPU in host RAM.
- [x] Create a dedicated CPU thread per vCPU.

### vCPU Fetch-Decode-Execute Cycle

- [x] Implement instruction fetch.
- [ ] Implement instruction decoding.
- [ ] Implement instruction execution.

## vCPU Start-up flow

On reset, all registers are zeroed out. In doing so, the PC also points at memory location 0x00 where the reset handler is stored.
While in the reset handler, the vCPU is configuring itself. The reset handler is responsible for copying necessary data from ROM into RAM and initializing the context before handing over control to the application program.

Currently, there is no concept of priviliged or unprivileged tasks, meaning that the application program will always run without restriction. This provides the application program with full access and complete ability to f*** everything up. 

No worries, just reset the device.

## vCPU execution-loop.

The vCPU acts as a real-time translator that translates the guest-machine(GM) instructions(INSN) to host-machine(HM) instructions(INSN) that are executed on the host.

Each vCPU has it's own dedicated thread and host-instruction buffer(HIB). The HIB has it's own host-processing controller(HPC) that actually executes the translated HM INSN.

## Host-processing

Each vCPU has it's own dedicated HIB, and each HIB has it's own dedicated host-instruction buffer observer(HIBO) and HPC.

To stabilize the execution rate of instructions, the process of executing the translated instruction on the host is asynchronous on it's own thread. As the vCPU is translating instructions into the HIB, the HPC is reading that buffer at a fixed frequency and executing the instructions.

To optimize the host-processing, there is a HIBO that notifies the HPC if the HIB is about to run out of space. If the unutilized HIB-space falls below a certain treshold, it will temporarily increase the size of the HIB-space and notify the HPC.

When the HIBO notifies the HPC that the HIB is about to run out of space, the HPC will increase it's read frequency and it will increase it's HIB addressability to accomodate it's increased size.

When the HPC detects that the HIB is empty, it will decrease it's read frequency. This happens at read intervals, meaning that the HPC will see that the HIB is empty and step down it's read-frequency. If the HIB is still empty the next time it reads, it will step down it's read frequency once more. The minimum read-frequency is 1 hz, meaning that if you halt the vCPU, the HPC is still running, waiting for work.

## Interrupts and exception handling

Interrupts are handled by the external nested vector interrupt controller(NVIC), and all interrupt handlers has dedicated addresses in the memory space. If those handlers are moved to a different location, the device will fail as the PC will be set to the expected address.

When an interrupt is triggered, the NVIC will preserve the context of the interrupted task in the stack, and pass control to the interrupt handler. The interrupt handler is responsible for exiting correctly and return control to the interrupted task with it's preserved context, so it's able to continue it's work.

All interrupts are mapped towards a priority number, meaning that an interrupt of higher priority is able to interrupt an interrupt. The way an interrupt is handled allows this since the NVIC makes no distinction between an interrupt handler and the application program when an interrupt is triggered and it passes control.

This also means that when an interrupt does occur, the NVIC will check the interrupts priority against the currently running task. By default, the application program task will have INTERRUPT_PRIORITY=-1, meaning all interrupts have higher priority.

# Abbreviations

- vCPU: Virtual computer processing unit.
- GPR: General purpose register.
- PC: Program counter.
- LR: Link register.
- SP: Stack pointer.
- HIB: Host-instruction buffer.
- HPC: Host-processing controller.
- HIBO: Host-instruction buffer observer.
- IRQ: Interrupt request.
- ISR: Interrupt service routine. Also known as interrupt handler.
- IPRI: Interrupt priority.
- IVT: Interrupt Vector Table.
- HM: Host-Machine.
- GM: Guest-Machine.
- INSN: Instructions.
- NVIC: Nested vector interrupt controller.
