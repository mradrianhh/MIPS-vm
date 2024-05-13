# MIPS-vm - version 2.0

## Introduction

The goal of this project is to evaluate a PoC software-archicture design for implementing an emulated MIPS R3000 computer. When reached, we hope to have a design where you can run the virtual machine and provide it a binary file. The machine will then upload the binary data into it's virtual memory and begin execution. 

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

The addressability of the user space is 2 GB, since this is a 32-bit computer with a 32-bit PC.
The total addressability is 4 GB.

The memory map consists of four segments: **kuseg**, **kseg0**, **kseg1** and **kseg2**.

The logical user address space is the **kuseg**-segment which ranges from address 0x00000000 to 0x7FFFFFFF

![Alt text](./Memory%20Map.drawio.svg)

# Virtual CPU

The CPU is inspired by the design of a classic RISC pipeline. This means that it's designed to implement a five step pipeline comprising of:

1. Fetch instruction.
2. Decode instruction and fetch registers.
3. Execute instruction..
4. Access memory.
5. Register write back.

Between each stage, there are buffers to forward information from one stage to the next.

![Alt text](./CPU.drawio.svg)

An instruction can have a maximum of two source register and one destination register.

## Instruction fetch

During instruction fetch, the CPU spends one cycle to fetch the instruction from memory at the address pointed to by the PC. Afterwards, it needs to recalculate the PC. Since all instructions has the same length and this is a 32-bit computer, PC is incremented by 4 bytes. In the case of a branch, jump or exception, the PC needs to be set to the appropriate address. PC will always be incremented here, but it might be re-calculated in the decode stage.

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

## Instruction Set Architecture

The MIPS R3000 computer implemented the MIPS I ISA.

MIPS I is a load/store architecture. It only supports register-to-register operations with the exception of memory access.

### Registers

MIPS I has thirty-two 32-bit registers. 

Register $0 is hardwired to zero such that all writes to it are discarded.

Register $31 is the link-register.

For multiplication and division operations there is two registers called HI and LO to hold the upper 32-bit result and lower 32-bit result, respectively. There is also a small set of instructions for copying data between general-purpose registers and the HI/LO register.

The program-counter is 32-bits with the two low-order bits always containing zero as the instructions are fixed-size and aligned to their natural word boundary. This also implies that when data is inserted, alignment must be kept for the instructions to keep their data integrity.

The complete register list is provided in the table below.

| Name    | Number | Usage                | Preserved |
| :----   | :----- | :------------        | :-------- |
| $zero   | 0      | Constant zero        | No        |
| $at     | 1      | Reserved (assembler) | No        |
| $v0-$v1 | 2-3    | Function result      | No        |
| $a0-$a3 | 4-7    | Function arguments   | No        |
| $t0-$t7 | 8-15   | Temporaries          | No        |
| $s0-$s7 | 16-23  | Saved                | Yes       |
| $t8-$t9 | 24-25  | Temporaries          | No        |
| $k0-$k1 | 26-27  | Reserved(OS)         | No        |
| $gp     | 28     | Global pointer       | Yes       |
| $sp     | 29     | Stack pointer        | Yes       |
| $fp     | 30     | Frame pointer        | Yes       |
| $ra     | 31     | Return address       | Yes       |
| PC      | --     | Program Counter      | Yes       |
| HI      | --     | Upper 32-bits        | Yes       |
| LO      | --     | Lower 32-bits        | Yes       |
### Instruction formats

**R:**
- **Bits[26-31]**: opcode(6 bits).
- **Bits[21-25]**: rs(5 bits).
- **Bits[16-20]**: rt(5 bits).
- **Bits[11-15]**: rd(5 bits).
- **Bits[6-10]**: shamt(5 bits).
- **Bits[0-5]**: funct(6 bits).

**I:**
- **Bits[26-31]**: opcode(6 bits).
- **Bits[21-25]**: rs(5 bits).
- **Bits[16-20]**: rt(5 bits).
- **Bits[0-15]**: immediate(16 bits).

**J:**
- **Bits[26-31]**: opcode(6 bits).
- **Bits[0-25]**: address(26 bits).

### Instructions

MIPS I has instructions that can load and store 8-bit bytes, 16-bit halfwords and 32-bit words. Only one addressing mode is supported: (base + displacement).

The instruction set can be divided into three groups:

1. Arithmetic/bitwise operations. Addition, left-shift, etc.
2. Data transfer operations.
3. Control flow operations.
    - Unconditionally jump to an address in memory.
    - Jump to an address if a register has a value of zero.
    - Invoke a function.
    - Return from a function.

Since MIPS I is a 32-bit architecture, loading values less then 32-bits requires the datum to be either sign- or zero-extended. In the case of instructions suffixed with "unsigned", they perform zero-extension. Otherwise, sign-extension is performed.

#### Load and store

Load instructions source the base from the GPR denoted by **rs** in the instruction format, and they write the result to the GPR denoted by **rt**.

Store instructions also source the base from **rs**, but store the content *from* the register **rt**.

All load and store instructions compute the address by summing the base from **rs** with the sign-extended 16-bit immediate value.

All memory accesses are required to be aligned on their natural word boundary. If not, an exception is signaled. However, to support efficient unaligned memory access, there are load/store instructions suffixed by "left"/"right".

All load instructions are followed by a *load delay slot* to prevent a data hazard as mentioned above. The instruction in the load delay slot can not use the data loaded by the load instruction. If the following instruction needs to use that data, a NOP instruction needs to be inserted into the load delay slot, also known as a bubble. This design is not efficient, as it wastes CPU cycles and reduces the overall throughput, but it's a very simple way of dealing with the issue.

#### Addition and subtraction

MIPS I has instructions to perform addition and subtraction. They source their operands from the GPR's denoted by **rs** and **rt** in the format, and write the result to the GRP denoted by **rd**. Alternatively, addition can source one of it's operands from a 16-bit immediate value that is sign-extended to 32-bits.

Addition and subtraction instructions has two variants. By default, an exception is signaled if the result overflows. Instructions with the "unsigned" suffix however, do not signal an exception. In this case, the overflow check interprets it as a 32-bit two's complement integer.

#### Bitwise logic

MIPS I has the following instructions to perform bitwise operations: AND, OR, XOR and NOR. These instructions also source their operands from the GRP's denoted by **rs** and **rt**, and similarly writes the result to the GRP denoted by **rd**. Also, similarly, they can alternatively retrieve one of their operands from a 16-bit immediate which is *zero-extended* to 32-bits.

#### Set on *relation*

The Set on *relation* instructions writes one or zero to the destination register if the specified relation is true or false, respectively. These instructions source their operands from two GRP's, or alternatively one GRP and one 16-bit immediate value that is sign-extended to 32-bits, and write the result to the third GPR. By default, the operands are interpreted as signed integers. The variants with the "unsigned" suffix interpret the operands as unsigned, even the 16-bit immediate value that is sign-extended.

#### Load 32-bit immediate

When loading a 32-bit immediate, the MIPS I ISA provides the instruction "load upper immediate" to load the high-order 16-bits of a GPR with a 16-bit immediate value. This instruction is used in conjunction with the "OR immediate" instruction to load the remaining low-order 16-bits into the register.

#### Logical and arithmetic shifts

MIPS I provides instructions for performing left and right arithmetic and logical shifts. The operand is obtained from the GRP denoted by **rt**, and the result is written to the GRP denoted by **rd**. The shift distance is obtained either from the GPR denoted by **rs** or from the 5-bit shift amount denoted by **sa**.

#### Multiplication and division

MIPS I provides instructions for signed and unsigned integer multiplication and division. These instructions retrieve their operands from two GPR's and because these instructions may execute separately, or concurrently, with  the other CPU instructions, they write their result to a pair of 32-bit registers called **HI** and **LO**. 

For multiplication, the high- and low-order 32-bits of the 64-bit result is written to **HI** and **LO**, respectively. For division, the quotient is written to **LO** and the remainder to **HI**.

To access the results, a pair of instructions is provided to copy the content of **HI**/**LO** to a GPR: Move from **LO** and Move from **HI**.

These instructions are interlocked, meaning that reads of **HI** and **LO** do not proceed past an unfinished arithmetic operation that will write to **HI** or **LO**.

A pair of instructions is also provided to write to **HI** or **LO**: Move to **HI** and Move to **LO**. These are used to restore **HI** and **LO** to their original state after exception handling.

Instructions that read **HI** or **LO** must be separated by two instructions that don't write to **HI** or **LO**.

#### Control flow

All MIPS I control flow instructions are followed by a *branch delay slot*.

A *delay slot*, generally, is an inserted instruction that is not affected by the preceding instruction. In this case, a branch delay slot is an instruction that is not affected by the branch. The most simple case, is a NOP instruction. Branch delay slots are used to maintain the timing for an instruction that needs more cycles to complete.

For the MIPS I, unless the branch delay slot is filled with an instruction performing useful work, a NOP instruction is substituted.

MIPS I branch instructions compare the content of the GPR denoted by **rs** against either zero or another GRP denoted by **rt** as signed integers and branch if true.

Control is transferred to the address computed by shifting the 16-bit offset left by two bits, sign-extending it to a 32-bit result and adding it with the sum of the program counter and the value 8(base-10).

Jumps have two versions: absolute and register-indirect.

Absolute jumps include the instructions "Jump" and "Jump and link". They compute the address to which control is transferred by shifting the 26-bit inst_index left by two bits, and concatenating the 28-bit result with the four higher-order bits of the address of the instruction in the branch delay slot.

Register-indirect jumps include the instructions "Jump register" and "Jump and link register". They transfer control to the address stored in the GRP denoted by **rs**. This address must be word-aligned. If not, an exception is signaled after the instruction in the branch delay slot is executed.

Branch and jump instructions that link store the return address in the link register(GRP 31). The "Jump and link register" instruction allows the return address to be stored in any writable GPR.

#### Exception signaling

MIPS I has two instructions for signaling exceptions: "System Call" and "Breakpoint". "System Call" is used by user-mode software to make kernel calls, and "Breakpoint" is used to transfer control to a debugger via the kernel's exception handler.

Both instructions has a 20-bit code field that can contain operating-environment specific information for the exception handler.

#### Floating-point operations

MIPS I has 32 floating-point registers. These are paired to double precision numbers. Odd numbered registers can not be used for arithmetic and branching resulting in 16 usable registers. Moves/copies and load/stores are not affected.

Single precision is denoted by the ".s" suffix, double precision is denoted by the ".d" suffix.

## Change log

- 12.05.2024 vCPU: Remove vCPU_state. Add buffers between pipeline stages.

- 10.05.2024: Version 2.0
