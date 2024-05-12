# 8-bit-VM - version 2.0

## Introduction

The goal of this project is to evaluate a PoC software-archicture design for implementing an emulated computer. When reached, we hope to have a design where you can run the virtual machine and provide it a binary file. The machine will then upload the binary data into it's virtual memory and begin execution. 

At this moment, the project attempts to emulate a computer consisting only of a CPU, a clock and some memory. The computer has a very simplified instruction set whose only aim is to accomodate testing of the architecture.

## Version notes

### Version 1.0

Version 1.0 introduced an architecture where the different components were running on separate threads and communicating through asynchronous queues. This aimed to emulate the flow of data between the components in a realistic manner in which an asynchronous queue acts like a data bus.

The challenges faced with this architecture is synchronization. Although the design implemented the flow of data in a realistic manner, the sequence of processing is not. 

In a real computer, each component runs independently and communicates over buses, but they are not asynchronous, they perform a unit of work at the same time. This means that all the components in the computer do work independently, but synchronously with each unit of work being performed at either a clock cycle, or perhaps at either a falling or rising edge of the clock signal.

In an effort to help the components work independently and synchronously, we revised the design in version 2.0.

### Version 2.0

Version 2.0 introduced an event-driven architecture.

As mentioned in the notes of version 1.0, although the different components of a computer works independently, the work synchronously, and the synchronizer is the clock.

In the revised design, we implemented an event-driven architecture with the clock as the driver. This means that the clock is triggering an event at a fixed frequency. We wanted to not only accomodate component designs in which the component responds to a complete cycle, but also designs in which it responds to a specific edge. 

This means that the clock triggers an event not for each complete cycle, but at each change in the edge, with this change occuring at double the frequency - or twice as fast - as the frequency provided to the clock. As a result, the clock will trigger two events per cycle: a rising edge event, and a falling edge event.

Each component will then subscribe to the clock's event, and perform their work when triggered. For the CPU, this eased the implementation of a pipeline design for the fetch-decode-execute cycle. Now, at each cycle, the CPU is able to read the new instruction in the fetch stage, decode the previous instruction in the decode stage and execute the instruction before that in the execute stage.

# Virtual memory

Memory is emulated by allocating a contigous segment of memory as an array in the virtual address space provided by the host.

Writing and reading data to and from memory is as simple as modifiying/reading the value in the array indexed by the address. But an interface is provided in the means of buses.

Inspired by version 1.0, we still use asynchronous queues as buses. When plugging components together, you register the buses with the different components so they can read/write on them. Synchronization is maintained because components only read/write on the buses in response to signal change event from the clock.

Each component can register itself to the memory by providing a data bus and an address bus, and the clock will read/write to them.

Care must be taken to distinguish between the virtual address space provided by the host, referred to as host virtual address space, and the virtual address space allocated to a process/application running on the virtual machine. No distinction will be made when referring to the latter, but unless host is specified, it is to assumed that we are discussing memory in the virtual machine used by processes/applications running the virtual machine.

It is also worth noting that for a single-application virtual machine, such as an emulated single-application embedded system, the virtual address space(not the host virtual address space) is the complete virtual memory.

# Virtual CPU

The CPU is inspired by the design of a classic RISC pipeline. This means that it's designed to implement a five step pipeline comprising of:

1. Fetch instruction.
2. Decode instruction and fetch registers.
3. Execute instruction..
4. Access memory.
5. Register write back.

Between each stage, there are buffers to forward information from one stage to the next.

An instruction can have a maximum of two source register and one destination register.

## Instruction fetch

During instruction fetch, the CPU spends one cycle to fetch the instruction from memory at the address pointed to by the PC. Afterwards, it needs to recalculate the PC. Since all instructions has the same length and this is an 8-bit computer, PC is incremented by one byte. In the case of a branch, jump or exception, the PC needs to be set to the appropriate address. PC will always be incremented here, but it might be re-calculated in the decode stage.

## Instruction decode

This CPU architecture does not use microcode. Decoding is done directly at this stage.

During the decoding, the register indexes are pulled from the instruction and stored in register memory as an address into the register file.

We also check if the instruction is ready to execute. If not, we stall the fetch and decode stage.

If the decoded instruction is a branch or jump instruction, we need to compute the target address of the instruction in parallel while reading the register file. Then, during the execute, the actual condition for the branch will be computed, and if taken or if it's a jump, we need to assign the target address to the PC instead of the incremented value. 

Also, if the branch is taken, we will have instructions in the pipeline that we no longer wish to execute, so we need to flush it by overwriting the fetch stage with a stall.

## Instruction execute

During this stage, the actual computation occurs. This stage comprises of an ALU and a bit-shifter.
For our sake, we'll simply execute the computation in the C-language on the host.

For reference, we'll list the latency classes of RISC instructions below:

- Register-to-register operations(single-cycle latency):  Add, subtract, compare, and logical operations. During the execute stage, the two arguments were fed to a simple ALU, which generated the result by the end of the execute stage.

- Memory reference(two-cycle latency): All loads from memory. During the execute stage, the ALU added the two arguments (a register and a constant offset) to produce a virtual address by the end of the cycle.

- Multi-cycle instrucitons(multiple cycles latency): Integer multiply and divide and all floating-point operations. During the execute stage, the operands to these operations were fed to the multi-cycle multiply/divide unit. The rest of the pipeline was free to continue execution while the multiply/divide unit did its work. To avoid complicating the writeback stage and issue logic, multicycle instruction wrote their results to a separate set of registers.

## Memory access

This stage is used by memory reference instructions. The reason that they have a two-cycle latency is that they have two stages of execution. First, the instruction execute stage where the memory access addresses are computed, and then this stage, where the memory is actually accessed.

During this stage, register-to-register instructions are simply forwarded so that both two-cycle latency instructions and single-cycle latency instructions reach the write-back stage at the same time. This allows a single port into the register file for these two latency-classes to write their results, and it will always be available.

Note: multi-cycle instructions usually has their separate set of registers, so the port-issue does not apply for them.

## Write-back

During this stage, both the single-cycle and two-cycle latency instructions write their results into the register file.

A hazard may occur during this stage as both the write-back stage and decode stage is accessing the register file. This is a data hazard as the decode stage is reading two source registers whereas the write-back stage is updating a destination register. If they both were two access the same register at the same time, we would have a data hazard. Read below for information on how this is handled.

## Stage buffers

Between each stage, there exists a buffer to pass information from one stage to the next: Those buffer are:

- IF/ID: Holds the instruction fetched.
- ID/EX: Holds the decoded instruction and both the source operands.
- EX/MEM: Holds the executed result from the instruction and information for the write-back stage. The information for the write-back stage must be forwarded through the memory-access stage.
- MEM/WB: Holds the data fetched from memory for a load operation, for arithmetic and logical operations the results are just passed.

Note the naming convention: [source stage]/[destination stage].

## Register file

The register file is an array of registers inside the CPU. This is a simple CPU, so the logical registers presented to the programmer is mapped one-to-one to the physical register file in the CPU. In practice, this means that there are no renaming and dynamic change of the mapping between the logical and physical registers.

The CPU might later on contain multiple register banks where necessary, but for now, we use a simple register file.

Note: register banking is the method of having a single logical register name access multiple, different physical registers depending on the operating mode.

## Bubbles and hazards

A bubble is inserted during a stall. A bubble is simply a NOP-instruction that will flow through the pipeline preventing each stage from doing any work.

## Exceptions

An exception is in simple terms a redirect to a target subroutine that is responsible for handling errors. 

If you were to add up two large integers that don't fit in a 32-bit register, you would have an overflow exception and the program would redirect to a subroutine responsible for handling this error and fixing it. During this re-direct, special-registers are written to with the exception cause and location.

Exceptions are resolved in the write-back stage, and when an exception occurs, it's important for it to be precise. This means that all instructions before the exception occured have been executed, and all instructions after it occured won't be executed. Execution is prevented by invalidating the latter. 

For most instruction's, this is handled implicitly by the pipeline's nature since they write their results during the write-back stage, but the store instruction's write their results during the memory access stage to the Store Data Queue. If a store instruction is invalidated, this Store Data Queue must be invalidated so that it's not written to memory later.