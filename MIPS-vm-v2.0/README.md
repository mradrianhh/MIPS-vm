# MIPS VM

# Introduction

The goal of this project is to evaluate a PoC software-archicture design for implementing an emulated MIPS R3000 computer. When reached, we hope to have a design where you can run the virtual machine and provide it a binary file. The machine will then upload the binary data into it's virtual memory and begin execution. 

At this moment, the project attempts to emulate a computer consisting only of a CPU, a clock and some memory. The computer has a very simplified instruction set whose only aim is to accomodate testing of the architecture.

# Version notes

## Version 1.0

Version 1.0 introduced an architecture where the different components were running on separate threads and communicating through asynchronous queues. This aimed to emulate the flow of data between the components in a realistic manner in which an asynchronous queue acts like a data bus.

The challenges faced with this architecture is synchronization. Although the design implemented the flow of data in a realistic manner, the sequence of processing is not. 

In a real computer, each component runs independently and communicates over buses, but they are not asynchronous, they perform a unit of work at the same time. This means that all the components in the computer do work independently, but synchronously with each unit of work being performed at either a clock cycle, or perhaps at either a falling or rising edge of the clock signal.

In an effort to help the components work independently and synchronously, we revised the design in version 2.0.

## Version 2.0

Version 2.0 introduced an event-driven architecture.

As mentioned in the notes of version 1.0, although the different components of a computer works independently, the work synchronously, and the synchronizer is the clock.

In the revised design, we implemented an event-driven architecture with the clock as the driver. This means that the clock is triggering an event at a fixed frequency. We wanted to not only accomodate component designs in which the component responds to a complete cycle, but also designs in which it responds to a specific edge. 

This means that the clock triggers an event not for each complete cycle, but at each change in the edge, with this change occuring at double the frequency - or twice as fast - as the frequency provided to the clock. As a result, the clock will trigger two events per cycle: a rising edge event, and a falling edge event.

Each component will then subscribe to the clock's event, and perform their work when triggered. For the CPU, this eased the implementation of a pipeline design for the fetch-decode-execute cycle. Now, at each cycle, the CPU is able to read the new instruction in the fetch stage, decode the previous instruction in the decode stage and execute the instruction before that in the execute stage.

# CPU

The CPU is inspired by the design of a classic RISC pipeline. This means that it's designed to implement a five step pipeline comprising of 5 pipestages:

1. IF: Fetch instruction from I-cache.
2. RD: Decode instruction and fetch registers.
3. ALU: Perform arithmethic or logical operation in one clock. Floating-point math and divide/multiply can't be done in one clock.
4. MEM: Instructions can read/write memory variables in D-cache.
5. WB: Write back value to register file.

One thing to note is that memory access occurs after the ALU, so an instruction can't perform arithmethic/logical operations on memory variables. They must be fetched into registers first.

Between each stage, there are buffers to forward information from one stage to the next.

![Alt text](./CPU.drawio.svg)

An instruction can have a maximum of two source register and one destination register.

## Pipestages

The architecture consists of the five pipestages mentioned above, with each pipestage having specific tasks.

### IF: Instruction fetch from I-cache

During instruction fetch, the CPU spends one cycle to fetch the instruction from memory at the address pointed to by the PC. Afterwards, it needs to recalculate the PC. Since all instructions has the same length and this is a 32-bit computer, PC is incremented by 4 bytes. In the case of a branch, jump or exception, the PC needs to be set to the appropriate address. PC will always be incremented here, but it might be re-calculated in the decode stage.

### RD: Read register

This CPU architecture does not use microcode. Decoding is done directly at this stage.

During the decoding, the register indexes are pulled from the instruction and stored in register memory as an address into the register file.

We also check if the instruction is ready to execute. If not, we stall the fetch and decode stage.

If the decoded instruction is a branch or jump instruction, we need to compute the target address of the instruction in parallel while reading the register file. Then, during the execute, the actual condition for the branch will be computed, and if taken or if it's a jump, we need to assign the target address to the PC instead of the incremented value. 

Also, if the branch is taken, we will have instructions in the pipeline that we no longer wish to execute, so we need to flush it by overwriting the fetch stage with a stall.

### ALU: Single-cycle arithmetic/logical operations

During this stage, the actual computation occurs. This stage comprises of an ALU and a bit-shifter.
For our sake, we'll simply execute the computation in the C-language on the host.

For reference, we'll list the latency classes of RISC instructions below:

- Register-to-register operations(single-cycle latency):  Add, subtract, compare, and logical operations. During the execute stage, the two arguments were fed to a simple ALU, which generated the result by the end of the execute stage.

- Memory reference(two-cycle latency): All loads from memory. During the execute stage, the ALU added the two arguments (a register and a constant offset) to produce a virtual address by the end of the cycle.

- Multi-cycle instrucitons(multiple cycles latency): Integer multiply and divide and all floating-point operations. During the execute stage, the operands to these operations were fed to the multi-cycle multiply/divide unit. The rest of the pipeline was free to continue execution while the multiply/divide unit did its work. To avoid complicating the writeback stage and issue logic, multicycle instruction wrote their results to a separate set of registers.

### MEM: D-cache read/write

This stage is used by memory reference instructions. The reason that they have a two-cycle latency is that they have two stages of execution. First, the instruction execute stage where the memory access addresses are computed, and then this stage, where the memory is actually accessed.

During this stage, register-to-register instructions are simply forwarded so that both two-cycle latency instructions and single-cycle latency instructions reach the write-back stage at the same time. This allows a single port into the register file for these two latency-classes to write their results, and it will always be available.

Note: multi-cycle instructions usually has their separate set of registers, so the port-issue does not apply for them.

### WB: Register-file write back

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

### RAW - Read after write conflict

There are multiple scenarios in which a RAW conflict might occur. A simple of example of this is the following:

    la  $a2,    init_common_resources
    jr  $a2
    nop

Which is expanded to:
    lui     $a2, "higher two bytes"
    addiu   $a2, $a2, "lower two bytes"
    jr      $a2
    nop

In this case, we see that when decoding lui, we are fetching the $a2 register which we will both read and write to. Then, in the following instruction,
we are again fetching the $a2 register which we will again both read and write to. The conflict which occurs is that the result of the lui-instruction
is written back to the register file two cycles later, in the WB stage. What happens then, is that when addiu is decoding, it's not able to fetch the true
value of the $a2 register.

There are two methods of fixing this hazard. We either have to implement a bypass, such that the result of the lui-instruction in the EX stage is delivered
directly to the other pipestages before it's written back to the register file in the WB stage. The other alternative is to insert stalls such that the following
addiu-instruction is not decoded until after the lui-instruction has passed the WB stage. 

To implement the latter, we have to calculate the number of stalls. The addiu-instruction is one cycle behind the lui-instruction initially, but
if we want to align its ID stage with the lui-instruction's WB stage, we have to stall the addiu-instruction so that it's three cycles behind. In other words,
we have to insert two stalls in this case.

From a more general point of view, we see that when a subsequent instruction is referencing a register which is updated by a previous instruction, and
the subsequent instruction is less than three cycles behind the previous instruction, we have to insert as many stalls as necessary.

## Caches

There are implemented two distinct caches: an instruction cache(I-cache) and a data cache(D-cache).
Access to the caches requires just a single-cycle, and both the I- and D-cache can be accessed
simultaneously.

### Mapping and indexing

Both caches are direct mapped, physically indexed and physically tagged. 

The CPUs program address is translated to a physical address just as in the case of real memory addressing, 
before being used for the cache lookup. 
Tag comparison(looking for a hit) is also based on physical addresses.

The caches are direct mapped meaning that each physical address as only one location in each cache
where it may reside. Each cache index consists of a tag and a cache line. For the D-cache, the cache line 
consists of a one word data item, but for the I-cache it's usually a 4-word line. The tag contains the memory address
for which this data is a copy.

If the tag matches the higher-order address bits, then the cache line contains the data the CPU is
looking for. The data is returned and execution continues. However, for the I-cache, the two lower-address
bits are used to index into the cache line and select one of the four words. 

When a cache miss occurs, the whole line must be filled from memory. It is however possible to fetch
more than a line's worth of data. In the case of the D-cache, it's possible to fetch 4 words of data
which is then stored in 4 1-word lines. It's important to note that because the D-cache is write-through
the main-memory and cache is always up-to-date. This means that we can discard the previous data when 
refilling the line at an index without concern.

### I-cache and D-cache

The data cache is of type write-through meaning that writes to the D-cache are 
passed through directly to memory via a writeable buffer.

During a data load, we attempt to read data from a cacheable location. In this case, the data will be 
returned from the D-cache if the cache contains the corresponding physical address and the cache line is 
valid. This is called a "cache hit". 

If not, we have what is called a "cache miss" in which case the data is not found. We then have to
read the data from external memory(i.e ram). How this is implemented can differ between implementations, 
but generally the CPU will read one or more words from memory which is then loaded into the D-cache. 
Normal operation then proceeds.

In normal operation, a "cache miss" will cause the targeted cache line to "invalidate" the valid cache data.
This data is then discarded without any issue because the D-cache is write-through, meaning that cache-data
is never more up-to-date than memory.

If we attempt to load data from an uncacheable location, the data is always fetched from external memory
or a memory-mapped IO location. On rare occasions, a system might attempt to access a data location
as both cached and uncached. There are no issues with this because when you perform an uncacheable load,
the cache data is neither used nor updated. The same applies for data stores.

In all cases, when performing a cacheable store, it's also written to main memory to ensure that the cache
and main memory are up-to-date. The write-through occurs asynchronously though through a writable buffer.

If performing a cacheable 32-bit store, the cache is always updated, but possibly discarding data from
a previously cached location.

For byte or half-word stores, the cache will only be updated if the reference hits in the cache.
If the reference hits, data will be extracted from the cache, merged with the store data and, finally, 
written back.

If the partial-word store misses in the cache, the cache is left alone.

### Write buffer

To ensure that D-cache is write-through, a write buffer is implemented because memory is not able to write as fast as the CPU. 
The write buffer is a FIFO store which stores both the address and the data, and holds multiple write cycles. 
It's implemented as a 4-entry queue, meaning it can hold up to 4 write cycles at a time.

### Code example: Flush write buffer

wbflush:
    .set    noreorder
    lw      t0,wbflush      # Read an uncached memory location.
    j       ra              # Return to caller.
    nop
    .set    reorder

### Code example: Initializing and sizing the caches.

Below is a sample program for initializing and sizing the caches. Sizes are stored in globals dcache_size and icache_size.

- We swap the caches when sizing the I-cache. 
- We write a marker into the initial cache entry.
- We start with the smallest permissible cache size(MINCACHE).
- Read memory at the location for the current cache size. If it contains the marker, that is the correct size. Otherwise, double the size to try and repeat this step until the marker is found.

config_cache:
    .set    noreorder
    subu    sp,24           # Create a 24 byte/6 word stack frame.
    sw      ra,20(sp)       # Save return address in the first word of the new stack frame.
    sw      s0,16(sp)       # Save s0 in first regsave slot(second word).
    mfc0    s0,C0_SR        # Save SR into s0.
    mtc0    zero,C0_SR      # Disable interrupts
    .set    reorder
    jal     _size_cache     # Start by sizing D-cache.
    sw      v0,dcache_size  # Store the result in the global dcache_size.
    li      v0,SR_SWC       # Swap caches. I-cache is now D-cache.
    .set    noreorder
    mtc0    v0,C0_SR        # Write swap to SR.
    jal     _size_cache     # We swapped the caches, so we now size the I-cache.
    nop                     # We must insert branch delay slot here ourselves since we set noreorder.
    sw      v0,icache_size  # Store the result in global icache_size.
    mtc0    zero,C0_SR      # Swap back caches.
    and     s0,~SR_PE       # Do not inadvertently clear PE(cache parity error if set).
    mtc0    s0,C0_SR        # Restore SR from s0.
    .set    reorder
    lw      s0,16(sp)       # Restore s0
    lw      ra,20(sp)       # Restore ra
    addu    sp,24           # Pop stack frame
    j       ra              # Return control to caller.

_size_cache:
    .set    noreorder
    mfc0    t0,C0_SR        # Save current SR
    and     t0,~SR_PE       # Do not inadvertently clear PE.
    or      v0,t0,SR_ISC    # Isolate cache
    mtc0    v0,C0_SR        
    # First check if there is a cache there at all.
    move    v0,zero
    li      v1,0xa5a5a5a5   # Distinctive pattern(marker).
    sw      v1,K0BASE       # Try to write marker into initial cache entry.
    lw      t1,K0BASE       # Try to read marker from initial cache entry.
    nop                     # Load slow.
    mcf0    t2,C0_SR        # Read SR register into t2.
    nop                     # mcf0 slow.
    .set    reorder
    and     t2,SR_CM        # Check CM. CM is set if the load operation would have resulted in cache hit even though cache is isolated.
    bne     t2,zero,3f      # If CM is not zero(?) it's a cache miss, there is no cache and we skip to end(3f).
    bne     v1,t1,3f        # If data we read is not equal to the data we wrote there is no cache and we skip to end(3f).
    # Clear cache size boundaries to known state.
    li      v0,MINCACHE
1: 
    sw      zero,K0BASE(v0)
    sll     v0,1            # Cache size * 2
    ble     v0,MAXCACHE,1b  # If cache size <= maximum cache size, go back to 1.

    li      v0,-1
    sw      v0,K0BASE(zero) # Store marker in cache.
    li      v0,MINCACHE     # Set v0 to minimum cache size.
2:
    lw      v1,K0BASE(v0)   # Look for marker
    bne     v1,zero,3f      # If v1 is not zero, we found the marker. Go forward to 3.
    sll     v0,1            # Cache size * 2
    ble     v0,MAXCACHE,2b  # Keep looking if v0 <= maximum cache size, go back to 2.
    move    v0,zero         # If we get here, it means we exceeded maximum cache size and found no marker, so there must be no cache. Return zero.
    .set    noreorder
3:
    mtc0    t0,C0_SR        # Restore SR
    j       ra              # Return to caller.
    nop
    .set    
    
Below is a program for validating the cache. Every cache entry is either invalid or correctly corresponds to a memory location, and
also contains correct parity.

- Check that SR bit PZ is cleared to zero(parity enabled).
- Isolate the D-cache. Swap to access the I-cache.
- For each word of the cache, first write a word value with correct tag, data and parity, then write a byte to invalidate the line.

flush_cache:
    lw      t1,icache_size
    lw      t2,dcache_size          # Store cache sizes in t1 and t2.
    .set    noreorder
    mfc0    t3,C0_SR                # Save SR
    nop                             # mcf0 slow.
    and     t3,~SR_PE               # Dont inadvertently clear PE.
    beq     t1,zero,_check_dcache   # If no I-cache(icache_size is zero), check D-cache.
    nop                             # Branch delay slot
    li      v0,SR_ISC|SR_SWC        # Disable interrupt, isolate and swap.
    mtc0    v0, C0_SR
    li      t0, K0BASE
    .set    reorder
    or      t1,t0,t1                # t1 = t0|t1. t1 = K0BASE + icache_size(?)
1:
    sb      zero,0(t0)              # Write bytes for each word in cache. 8 words at a time.
    sb      zero,4(t0)
    sb      zero,8(t0)
    sb      zero,12(t0)
    sb      zero,16(t0)
    sb      zero,20(t0)
    sb      zero,24(t0)
    addu    t0,32                   
    sb      zero,-4(t0)
    bne     t0,t1,1b                # If we have not yet reached end of cache, keep going.
    # Flush data cache.
_check_dcache:
    li      v0,SR_ISC               # Isolate and swap back caches.
    .set    noreorder
    mtc0    v0,C0_SR
    nop                             # mtc0 slow.
    beq     t2,zero,_flush_done     # If dcache_size is zero, there is no D-cache and we jump to end.
    .set    reorder
    li      t0,K0BASE               
    or      t1,t0,t2                # t1 = K0BASE + dcache_size(?)
1:
    sb      zero,0(t0)              # Write bytes for each word in cache. 8 words at a time.
    sb      zero,4(t0)
    sb      zero,8(t0)
    sb      zero,12(t0)
    sb      zero,16(t0)
    sb      zero,20(t0)
    sb      zero,24(t0)
    addu    t0,32
    sb      zero,-4(t0)
    bne     t0,t1,1b                # If we have not yet reached end of cache, keep going.
    .set    noreorder
_flush_done:
    mtc0    t3, C0_SR               # Restore SR. Un-isolate and enable interrupts.
    .set    reorder
    j       ra                      # Return to caller.
    
### Invalidation

Invalidation means that specified cache lines contain no valid references to main memory, but are consistent(e.g. valid parity).

Software needs to invalidate the D-cache when memory contents have been changed by something other than store operations from the CPU, typically when 
some DMA device is reading into memory, and the I-cache when instructions have either been written by the CPU or obtained by DMA.

Invalidation is done by finding the address range to invalidate. Invalidating a region larger than the cache size is a waste of time.

Isolate the D-cache. Once it is isolated, the system must insure at all costs against an exception since the memory interface will be
temporarily disabled. Disable interrupts and ensure that software which follows cannot cause a memory access exception.

To work on the I-cache, swap the caches.

Write a byte value to each cache line in the range.

Unswap and unisolate.

An invalidate routine is normally executed with cacheable instructions. An invalidation routine in uncached space will run 4-10 times slower.

### Code example: Invalidation routine

Below is a code example for invalidating a cache starting a base address a0 with byte count a1.

clear_cache:
    # Flush I-cache.
    lw      t1,icache_size      
    lw      t2,dcache_size      # Store cache sizes.
    .set    noreorder
    mfc0    t3,C0_SR            # Save SR
    and     t3,~SR_PE           # Dont clear PE. And is performed in "ALU", in which case mfc0 is in "MEM". We therefore dont need a branch delay slot here.
    nop
    nop
    li      v0,SR_ISC|SR_SWC    # Disable interrupt, isolate and swap.
    mtc0    v0,C0_SR
    .set    reorder
    bltu    t1,a1,1f            # If cache size less than address range size(byte count), use cache size.
    move    t1,a1               # Else, use range size(byte count).
1:
    addu    t1,a0               # Set size to base address + size.
    move    t0,a0               # Store base address in t0.
    sb      zero,0(t0)          # Write bytes to invalidate each word(flush) starting at the base address.
    sb      zero,4(t0)
    sb      zero,8(t0)
    sb      zero,12(t0)
    sb      zero,16(t0)
    sb      zero,20(t0)
    sb      zero,24(t0)
    addu    t0,32
    sb      zero,-4(t0)
    bltu    t0,t1,1b            # If we havent reached the end of the address range, keep going.
    
    # Flush D-cache.
    .set    noreorder
    nop
    li      v0,SR_ISC           # Isolate and swap back caches.
    mtc0    v0,C0_SR        
    nop
    .set    reorder
    bltu    t2,a1,1f            # If D-cache is smaller than address range size(byte count), use cache size.
    move    t2,a1               # Else, use range size.
1:
    addu    t2,a0               # Set size to base address + size
    move    t0,a0
1:
    sb      zero,0(t0)          # Flush the D-cache.
    sb      zero,4(t0)
    sb      zero,8(t0)
    sb      zero,12(t0)
    sb      zero,16(t0)
    sb      zero,20(t0)
    sb      zero,24(t0)
    addu    t0,32
    sb      zero,-4(t0)
    bltu    t0,t2,1b            # If we havent reached end of address range, keep going.

    .set    noreorder
    mtc0    t3, C0_SR           # Restore SR.
    .set    reorder
    j       ra                  # Return to caller.

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

A complete list of instructions can be found in the [here](./MIPS%20Instruction%20Set.pdf).

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

## CP0 - System Control Co-Processor

The system control co-processor is used to manage the CPU environment, including state, exceptions, cache control and memory management. 

## CPU Control Registers

The following table describes the general CPU control registers, not including the MMU registers. Note the convention to reserve k0 and k1 for exception handling.

| Mnemonic  | Number | Description                          |
| :-------  | :----- | :-----------------------             |
| PRId      | 15     | CP0 type and rev level               |   
| SR        | 12     | (Status register) CPU mode flags     |
| Cause     | 13     | Most recent exception                | 
| EPC       | 14     | Return address from trap             | 
| BadVaddr  | 8      | Last invalid pgm address             | 
| Config    | 3      | CPU config(R3081/R3041)              |
| BusCtrl   | 2      | -                                    | 
| PortSize  | 10     | -                                    | 
| Count     | 9      | 24-bit counter(R3041)                | 
| Compare   | 11     | 24-bit value to wraparound counter   | 

### PRId

 Read-only register which describes CPU type.

 Bits[31..16]:  reserved
 Bits[15..8]:   Imp
 Bits[7..0]:    Rev

### SR

Status register containing CPU mode flags/bits.

Bit[31]:        CU3 
Bit[30]:        CU2 - CU3 and CU2 control usability of co-processors 3 and 2, respectively.
Bit[29]:        CU1 - "co-processor 1 usable": 1 to use FPA if present. When 0, all FPA instructions cause an exception.
Bit[28]:        CU0 - "co-processor 0 usable": 1 to enable nominally-priviliged instructions in user mode.
Bits[27..26]:   0
Bit[25]:        RE  - Reverse endianness in user mode.
Bits[24..23]:   0
Bit[22]:        BEV - "boot exception vectors": when 1 the CPU uses the ROM(kseg1) space exception entry point. When 0(typical for running systems), the exception vectors are relocated to RAM addresses.
Bit[21]:        TS  - "TLB shutdown": Gets set if a program address matches two TLB entries at the same time. It's set by reset meaning that software can use it to determine if TLB support is available.
Bit[20]:        PE  - Set if a cache parity error has occured.
Bit[19]:        CM  - Result of last load operation performed with D-cache isolated. CM is set if the load would have hit in the cache even if the cache had not been isolated.
Bit[18]:        PZ  - When set, cache parity bits are written as zero and not checked.
Bit[17]:        SwC 
Bit[16]:        IsC - "Swap caches" and "Isolate (data) cache". When IsC is set, all loads and store only access D-cache, never memory. When SwC is set, we swap I- and D-cache so I-cache is now writable and can be invalidated.
Bits[15..8]:    IM  - "Interrupt mask": 8-bit field which defines what interrupt sources, when active, will be allowed to cause an exception.
Bits[7..6]:     0
Bit[5]:         KUo
Bit[4]:         IEo - "KU old, IE old": on exception, KUp and IEp are stored here. Basically a 3-deep, 2-bit wide KU/IE stack which is pushed on exception and popped by rfe.
Bit[3]:         KUp
Bit[2]:         IEp - "KU previous, IE previous": on exception, KUc and IEc are stored here before setting KUc to 1(kernel mode) and IEc to 0(interrupts disabled). Instruction rfe copies KUp and IEp back to KUc and IEc.
Bit[1]:         KUc
Bit[0]:         IEc - Two basic CPU protection bits. KUc is set when running with kernel priviliges, but 0 in user mode. IEc is set to 0 to prevent the CPU taking any interrupt, 1 to enable.

### Cause Register

Register containing fields which are used to determine the kind of exception which happened, and is used to determine which
exception routine to call.

Bit[31]:        BD - "branch delay": When set, it indicates that EPC does not point to exception instruction, but the most recent branch instruction. Used to prevent returning to branch delay slot which could brake the interrupted program.
Bit[30]:        0
Bits[29..28]:   CE - "co-processor error": If the exception occured because a co-processor instruction was for a co-processor which is not enabled(CUx), then this field contains the co-processor number from that instruction.
Bits[27..16]:   0
Bits[15..8]:    IP - "Interrupt Pending": Shows the currently asserted interrupts(may be masked from signalling an exception).
Bit[7]:         0
Bits[6..2]:     ExcCode - 5-bit code which indicated what kind of exception happened. See table below.
Bits[1..0]:     0

| ExcCode Value | Mnemonic  | Description                   |
| :------------ | :-------  | :----------                   |
| 0             | Int       | Interrupt.                    |
| 1             | Mod       | "TLB modification".           |   
| 2             | TLBL      | "TLB load".                   |
| 3             | TLBS      | "TLB store".                  |
| 4             | AdEL      | Address error on load/I-fetch.|
| 5             | AdES      | Address error on store.       |
| 6             | IBE       | Bus error I-fetch.            |
| 7             | DBE       | Bus error data load.          |
| 8             | Syscall   | Syscall instruction.          |
| 9             | Bp        | Breakpoint                    |
| 10            | RI        | "reserved instruction".       |
| 11            | CpU       | "Co-Processor unusable".      |
| 12            | Ov        | "Arithmetic overflow".        |
| 13-31         | -         | reserved.                     |

### EPC Register

A 32-bit register containing the 32-bit address of the return point for this exception. The instruction causing the exception is at EPC,
but if BD is set in the Cause register, EPC points to the previous branch instruction.

### BadVaddr Register

A 32-bit register containing the address whose reference led to an exception. It's set on any MMU-related exception, on an attempt
by a user program to access addresses outside kuseg, or if an address is wrongly aligned for the datum size referenced.

### Count and Compare Registers(R3041 only)

These provide a simple 24-bit counter/timer running at CPU cycle rate. 
Count counts up and then wraps around to zero when it has reached the value in the Compare register. When it wraps around, the Tc* CPU output is asserted.
If Tc* should generate an interrupt, it must be connected to one of the interrupt inputs.
From reset, compare is set to its maximum value(0xFFFFFF).

### Config Register(R3041)

Bit[31]:        Lock - Set 1 to finally configure register. Additional writes to this register will not have any effect until the CPU is reset.
Bit[30]:        1
Bit[29]:        DBR - "DBlockRefill". set 1 to read 4 words into the cache on a miss, 0 to refill just the word missed on.
Bits[28..20]:   0
Bit[19]:        FDM - "Force D-Cache Miss". Set 1 forces all loads to fetch data from memory(missing in the data cache), but incoming data is still used to refill the cache. Stores continue to write the cache.
Bits[18..0]:    0

### BusCtrl Register(R3041 only)

Used to configure hardware interface options.

Not currently planned to be implemented.

### PortSize Register(R3041 only)

Used to flag different parts of the program address space for accesses to 8-, 16- or 32-bit wide memory.

Not currently planned to be implemented.

### What registers are relevant when?

The various CP0 registers and their fields provide support at specific times during system operation.

After hardware reset: software must initialize SR to get the CPU into the right state to bootstrap itself.

Hard configuration at start-up: R3041 require initialization of Config, BusCtrl and/or PortSize before very much will work.

After any exception: Any MIPS exception invokes a single common general exception handler routine at a fixed address. 
On entry, no program registers are saved, only the return address in EPC. The exception routine cannot use the user-mode stack because there might be a TLB miss on stack memory.
Exception software will have to use atleast one of k0 and k1 to point to some safe(exception-proof) memory space. 
Consult Cause register to find out what kind of exception it was and dispatch accordingly.

Returning from exception: Control must be returned to the value stored in EPC on entry. Software will have to adjust SR upon return from exception with instruction rfe.
To return control, you must load EPC into a general purpose register and use a jr operation.

Interrupts: SR is used to adjust the interrupt masks to determine which interrupts will be allowed higher priority than the current one.

Instructions which always cause exception: are often used for system calls, breakpoints and to emulate some kinds of instructions.
These sometimes require partial decoding of the offending instruction which can usually be found at the location EPC. 
If the offending instruction is in a branch delay slot, EPC will point to the previous branch. To find the offending instruction you have to look at location EPC + 4 when the BD bit is set.

Cache management routines: SR contains bits defining special modes for cache management. They allow software to isolate the data cache and to swap the roles of the I-cache and D-cache.

## Address space

Physical address is the actual address in memory.
Program address is commonly known as virtual address.
These are not necessarily the same as the program address might undergo transformation before being presented to physical memory.

A MIPS-1 CPU has two operating modes: user and kernel. While in user mode, any address above 2GB is illegal and causes a trap. In other words, if the MSB of the address is set, a trap is caused.

The 32-bit program address space is divided into four areas with traditional names:

kuseg 0x0000 0000 - 7FFF FFFF(low 2GB): These are the addresses permitted in user mode. If an MMU is present, they will always be translated. 
If not, they are transformed to a physical address by adding a 1GB offset.

kseg0 0x8000 0000 - 9FFF FFFF(512MB): These addresses are transformed to physical addresses by removing the MSB. 
This maps them contigously into the low 512MB of physical memory.
Addresses in this region are always accessed through the cache, and may therefore not be used until the cache is initialized.

kseg1 0xA000 0000 - BFFF FFFF(512MB): These addresses are transformed to physical addresses by removing the 3 top-most bits. 
They are therefore a duplicate mapping of the low 512MB of physical memory, but they will not use the cache when accessed.
This is the only region which is guaranteed to behave properly after system reset.
The after-reset starting pointer(0xBFC0 0000), commonly called the "reset exception vector", lies within it.
The equivalent physical address, with the 3 top-most bits stripped off, will be 0x1FC0 0000. The boot rom should there be placed at this physical address.

IO devices and system ROM should always be mapped to addresses that are accessible from kseg1. Code in the rom can then be accessed 
uncacheably during boot up using kseg1 program addresses and cacheably for normal operation using kseg0 program addresses.

kseg2 0xC000 0000 - FFFF FFFF(1GB): This area is only accessible in kernel mode



## Exceptions

Interrupts, traps, system calls and everything else which disrupts the normal flow of execution are called exceptions
and handled by a single mechanism. 

External events: interrupts or a bus error on a read. Interrupts are the only exception conditions which can be
disabled under software control.

Program errors and unusual conditions: non-existent instructions, including co-processor instructions executed with 
the appriopriate SR disabled, integer overflow, address alignment errors, accesses outside kuseg in user mode.

Memory translation exceptions: using an invalid translation or a write to a write-protected page, and access to a page for which
there is no translation in the TLB.

System calls and traps: exceptions deliberately generated by software to access kernel facilities in a secure way(i.e. syscalls,
conditional traps and breakpoints).

### Precise exceptions

The MIPS architecture implements precise exceptions which provides the following.

Unambigous proof of cause: after an exception caused by an internal error the EPC points to the offending instruction or the previous branch.

Exceptions are seen in instruction sequence: exceptions can arise at several different stages of execution, creating a potential hazard.
For example, if a load instruction suffers a TLB miss the exception won't be signalled until the "MEM" pipestage. If the next
instruction suffers an instruction TLB miss at the "IF" pipestage, the logically second exception will be signaled first because IF occurs earlier
in the pipe than "MEM".
To avoid this problem, early-detected exceptions are not activated until it is known that all previous instructions will complete successfully.
In this case, the TLB miss in "IF" will be suppressed and the earlier exception handled.

Subsequent instrutions nullified: because of the pipelining, instructions lying in sequence after the EPC may have been started, but the
architecture guarantees that no effects produced by them will be visible in the registers or CPU state, and no effect will occur
which prevent execution being restarted at the EPC. For example, the FPA cannot update the register file until it knows that the operation
will not generate an exception.

### Exception vectors

The "reset exception vector" which is called on reset is always mapped to kseg1 which means it is uncacheable. The reasoning behind this is that 
caches are not initialized at during reset, so the boot ROM must be accessible uncached.

TLB miss on an address in kuseg is handled by the exception vector stored at program address in kseg0 0x8000 0000(0x0000 0000 in physical memory). 
This entry point is cached.
All other exceptions have a general exception handler which have a cached entry point at program address in kseg0 0x8000 0080(0x0000 0080 in physical memory).
TLB miss also have an uncached entry point which is used if SR bit BEV is set. This is stored at program address in kseg1 0xBFC0 0100(0x1FC0 0100 in physical memory).
All other exceptions have a general exception handler which have a an uncached entry point at program address in kseg1 0xBFC0 0180(0x1FC0 0180 in physical memory).

The general exception handler reads the cause register to determine the kind of exception and dispatches to an appropriate exception handler.

The exception vectors have alternative entry points which are both cached and uncached to provide access both during start-up and during normal operations.

On an exception, the CPU sets up EPC to point to the restart location. The pre-existing user-mode and interrupt-enable flags in SR(KUc/KUp, IEc/IEp) are
saved by pushing the 3-entry stack inside SR and changing to kernel mode with interrupts disabled(KUc=1, IEc=0). Cause is then setup so that
software can see the reason for the exception. On address exceptions BadVaddr is also set. Memory management system exceptions set up some of the
MMU registers too. Finally, the CPU transfers control to the exception entry point.

### Exception handling

Any MIPS exception handler has to go through the same stages.

Bootstrapping: on entry to the exception handler very little of the state of the interrupted program has been saved, so the first job is to provide room to
preserve relevant state information. Almost inevitably, this is done by using the k0 and k1 registers which are reserved for kernel mode use to reference
a piece of memory which can be used for other register saves.

Dispatching different exceptions: consult the Cause register. The initial decision is likely to be made on the "ExcCode" field which is thoughfully
aligned so that its code value(between 0 and 31) can be used to index an array of words without a shift. See the following example code.

    mfc0    t1, C0_CAUSE        # Fetch Cause register from CP0.
    and     t2, t1, 0x3F        # Use ExcCode mask to fetch the ExcCode value from Cause register and store it in t2.
    lw      t2, tablebase(t2)   # Use the ExcCode to index into an array of words(exception handler addresses) and load
                                # the address of the appriopriate exception handler in t2.
    jr      t2                  # Jump to the appriopriate exception handler. Do not link. We will not return here. The called handler will return to EPC.

Constructing the exception processing environment: complex exception handling routines may be written in a high level language and/or a software
may wish to use standard library routines. To do this, software must switch to a suitable stack and save the values of all registers which called 
subroutines may use.

Processing the exception: this is system and cause dependent.

Return from an exception: The return address is contained in the EPC register on exception entry. The value must be placed into a general purpose
register for return from exception. Returning control is done with a jr instruction and restoring state is done by an rfe instruction in the 
delay slot after the jr instruction.

### Code example: Simple exception routine.

Below is as simple as an exception routine can be. It does nothing but increment a counter on each exception.

    .set noreorder          # Prevent compiler from reordering the code. Specifically, we want to prevent the compiler from adjusting branch delay slots.
    .set noat               # Prevent compiler from using at register. 
xcptgen:
    la      k0, xcptcount   # Get address of the counter.
    lw      k1, 0(k0)       # Load the counter.
    nop                     # Load delay(load slow).
    addu    k1, 1           # Increment the counter.
    sw      k1, 0(k0)       # Store the counter.
    mfc0    k0, C0_EPC      # Get EPC.
    nop                     # Load delay(mfc0 slow).
    j       k0              # Return to program
    rfe                     # Branch delay slot. Restore state.
    .set at
    .set reorder



## HDU - Hazard Detection Unit

The HDU is responsible for inserting bubbles, or stalls, when necessary. This is primarily done when the current instruction depends on the result of a memory load
instruction, in which case we can't simply forward the result from the MEM/WB buffer because the load instruction needs two cycles to complete. 

The HDU should stall the pipeline if the previous instruction in ID/EX is a load, and if the destination register of the load, register Rt, matches one of the 
source operands of the current instruction in IF/ID, register Rs or Rt. In pseudo-code:

If (ID/EX.MemRead) and
   (ID/EX.RegisterRt = IF/ID.RegisterRs) or
   (ID/EX.RegisterRt = IF/ID.RegisterRt) then
   stall the pipeline

## Forwarding unit

It's not always necessary to insert stalls. Typical data hazards, such as a RAW conflict, only requires forwarding the result from EX/MEM or MEM/WB into
the EX stage, so that the next instruction is able to access the updated value of the dependent register which is not yet written to the register file.

The forwarding unit differentiates between a hazard where an instruction is dependent on another instruction in the EX stage, an EX hazard, and a hazard
where an instruction is dependent on another instruction in the MEM stage, a MEM hazard.

### EX hazard

To check for an EX hazard, we first check if the in the EX stage actually writes to the register file. If that's the case, we check if the destination register
matches one of the source operands of the current instruction. If that's the case, we tell the current instruction to fetch the register value from
the EX/MEM buffer instead of the ID/EX buffer.

### MEM hazard

In the case of a MEM hazard, we also have to check if the instruction in the MEM stage actually writes to the register file. Additionally, we might
have two instructions following eachother that updates the same register, in which case we need to take in mind that the register value used by the
instruction should match the register value expected by the sequence of the program. This means that we also need to check if the destination register
in the EX stage also matches one of the source operands of the current instruction. In other words, if an EX hazard and a MEM hazard occurs at the same time,
we should forward the value of the EX hazard. The rest is the same. We check if we match on one of the source operands.

# Memory management

R30xx "E" versions will have on-chip memory management hardware for dynamically translating program addresses in the kuseg and kseg2 regions.
The key piece of hardware is the "TLB" - "translation lookaside buffer". The TLB is a look-up table of virtual to physical
address translations.

The memory management is paged with a fixed page size of 4KB. The low order 12-bit of the program address are used directly as the low order
bits of the physical address, so address translation operates in 4K chunks.

## TLB

The TLB is a 64-entry associative memory. Each entry consists of a key field and a data field. When presented with a key, the memory returns the data
of any entry where the key matches.

The TLB's key field contains two sections contained in the EntryHi MMU register.

Virtual Page Number(VPN): A program address with the low 12 bits cut off since the low-order bits don't participate in the translation process.

Address Space Identifier(ASID):  Magic number used to stamp translations. Why?

In multi-tasking systems, all user-level tasks may use the same program addresses, which are of course mapped to different physical addresses.
They are using different address spaces. But the VPN is fetched from the program address, so different user tasks might have the same VPN. Without ASID,
it would be impossible to differentiate them when the OS switches from one task to another. ASID is a 6-bit unique code where new tasks are assigned new 
ASIDs until all 64 are assigned. When all ASIDs are assigned, all the tasks are flushed of their ASIDs and the TLB flushed. As each task is 
re-entered, a new ASID is given. Thus, ASID flushing is relatively infrequent.

The TLB data field stored in EntryLo MMU register includes:

Physical Frame Number(PFN): The corresponding physical address of the entry with the low 12 bits cut off. In an address translation, the VPN bits
are replaced by the corresponding PFN bits to form the true physical address.

Cache control bit(N): Set 1 to make the page uncacheable.

Write control bit(D): Set 1 to allow stores to this page to happen.

Valid bit(V): Set 0 to make this entry usable. Why have unusable entries? Access to an invalid page causes a different trap from a TLB refill exception.

Global bit(G): Set 1 to disable the ASID-matching scheme, allowing an OS to map some program addresses to the same physical address for all tasks.

Translating an address is now simple.

CPU generates a program address: either for an instruction fetch, a load or a store, in one of the translated address regions. The low 12 bits
are separated off, and the resulting VPN together with the current value of the ASID field in EntryHi is used as the key to the TLB.

TLB matches key: selecting the matching entry. The PFN is glued to the low-order bits of the program address to form a complete physical address.

Valid?: The V and D bits are consulted. If it isn't valid, or a store is being attempted with D cleared, the CPU takes a trap. As with all
translation traps, the BadVaddr register will be filled with the offending program address and TLB registers Context and EntryHi pre-filled
with relevant information. The system software can use these registers to obtain data for exception service.

Cached?: If the N bit is set the CPU looks in the cache for a copy of the physical location's data. If it isn't there it will be fetched from memory and a 
copy left in the cache. If the N bit is clear, the CPU neither looks in nor refills the cache.

## TLB refill

The TLB only contains 64 entries which can hold translations for maximum 256KB of program addresses. This is rarely enough because the TLB is almost
always going to be used as a software-maintained "cache" for a much larger set of translations. When a program address lookup in the TLB failes,
a TLB refill trap is taken. System software then has the job of:

Figuring out whether there is a correct translation, if not the trap will be dispatched to the software which handles address errors.
If there is a correct translation, constructing a TLB entry which will implement it.
If the TLB is already full, selecting an entry which can be discarded.
Writing the new entry into the TLB.

### How it happens

When a program makes an access in kuseg or kseg2 to a page for which no translation record is present,
the CPU takes a TLB refill exception. The assumption is that the system software is maintaining a large
number of page translations and is using the TLB as a "cache" for recently-used translations. This means 
that the refill exception will normally be handled by finding a correct translation, installing it
and returning to user code.

To save time on user-program TLB refills, which are frequent in a big OS, refill exceptions are vectored
through a low-memory address used for no other exceptions. Also, special exception rules permit the
kuseg refill handler to risk a nested TLB refill exception on a kseg2 address. Because the CPU's SR
register has both KUc/IEc, KUp/IEp and KUo/IEo, we have a 3-deep stack which allows 1 level of nested exceptions.
In which case, the user program state are maintained in KUo/IEo, the kuseg refill handler state in KUp/IEp,
and the kseg2 refill handler state in KUc/IEc. However, the kuseg refill handler stores the EPC in the
k1 register. The kseg2 handler is dispatched from the general exception handler, so both the general
exception handler and the kseg2 refill handler must be careful to preserve the EPC stored in k1 to
allow a clean return.

## MMU Registers

| Mnemonic  | Number | Description                                                                                  |
| :-------  | :----- | :-----------------------                                                                     |
| EntryHi   | 10     |                                                                                              |
| EntryLo   | 2      | EntryHi and EntryLo hold a TLB entry.                                                        |
| Index     | 0      | Which TLB entry will be read/written by appropriate instruction.                             |
| Random    | 1      | Pseudo-random value(counter) used by tlbwr to write a new TLB entry into a random location.  |
| Context   | 4      | Used to TLB refill traps.                                                                    |

### EntryHi

Bits[31..12]:   VPN - The high-order bits of a program address. On a refill exception this field is set up to match the program address which could not be translated.
Bits[11..6]:    ASID - Address space identifier. Not changed by exceptions.
Bits[5..0]:     0

### EntryLo

Bits[31..12]:   PFN - Physical frame number.
Bit[11]:        N - "noncacheable". 0 - cacheable, 1 - uncacheable.
Bit[10]:        D - "dirty". 1 - allow writes, 0 and any store using this translation will be trapped.
Bit[9]:         V - "valid". 0 - any address matching this entry will cause an exception.
Bit[8]:         G - "global". When set in an entry the entry will match only on VPN, regardless of ASID.
Bit[7..0]:      0 - Ignored. They always return zero, regardless of data written.

### Index

Bit[31]:        P - Set when a tlbp instruction(TLB probe, used to see if the TLB can translate a particular VPN) failed to find a valid translation.
Bits[30..14]:   X - Ignored.
Bits[13..8]:    Index - Which TLB entry will be read/written by appropriate instruction.
Bits[7..0]:     X - Ignored.

### Random

Bits[31..14]:   X - Ignored.
Bits[13..8]:    Random - Free running counter used by tlbwr to write a new TLB entry into a random location. Initialized to 63 on reset, and decremented to 8. When it reaches 8, it wraps back to 63 and starts again.
Bits[7..0]:     X - Ignored.

### Context

Bits[31..21]:   PTEBase - A location which just stores what is put in it. In the "standard" refill handler, this will be the high-order bits of
the 1MB aligned starting address of a memory-resident page table.
Bits[20..2]:    Bad VNP - Following an address exception, this holds the high-order bits of the address. The same as BadVaddr. If the system
uses the "standard" TLB refill exception handling code the 32-bit value formed by Context is directly usable as a pointer to the memory-resident page table.
Bits[1..0]:     0 - Can be written with any value, but always read zero.

## MMU control instructions

tlbr    -   Read TLB entry at index
tlbwi   -   Write TLB entry at index

These two instructions move MMU data between the TLB entry selected by the Index register and the EntryHi/EntryLo registers.

tlbwr   -   Write TLB entry selected by Random

Copies the contents of EntryHi/EntryLo into the TLB entry indexed by the random register. This is used in a TLB refill exception handler to
write a new TLB entry, but tlbwi will be used anywhere else.

tlbp    -   TLB loopup

Searches(probes) the TLB for an entry whose VPN and ASID matches those currently in EntryHi and stores the index
of that entry in the index register which is negative if nothing matches. 
Note that tlbp does not fetch data, so the instruction following a tlbp must not be a load or store.

### TLB programming interface

TLB entries are setup by writing the required fields into EntryHi and EntryLo, and using tlbwi or tlbwr
to copy that entry into TLB proper.

When handling a TLB refill exception, the EntryHi register are automatically setup with the required VPN
and current ASID.

Be very careful not to create a duplicate entry into the TLB. A duplicate entry occurs if two entries
match on the VPN/ASID pair. If the TLB contains duplicate entries, an attempt to translate such an address
or probe for it will produce a fatal "TLB shutdown" condition which will set the TS bit in SR register.
It can only be cleared by a hardware reset.

TLB entries are rarely read. To read a TLB entry you first need to save the current EntryHi register 
because it's ASID field might be important. Then write the correct VPN and ASID to the EntryHi register.
You execute the tlbp instruction to get the index of the TLB entry which matches the VPN/ASID pair, and
then you can use tlbr to read the TLB entry at that index into the EntryHi/EntryLo registers.

### Using ASIDs

When setting up a TLB entry with an ASID and the EntryLo G-bit zero, those entries will only ever match
a program address when the CPU's ASID is the same. This allows software to map 64 different address spaces
without having the OS clear out the TLB during a context change.

In typical usage, a new task is assigned an un-initialized ASID. The first time the task is invoked,
it will will miss in the TLB, allowing an assignment of ASID. If the system runs out of ASIDs, it will flush
the TLB and mark all tasks as "new". Thus, as each task is re-entered, it will be assigned a new ASID.

There are no way to mark a TLB entry from being discarded, however, TLB entries are discarded using the
random counter which counts down from 63 to 8. This means that TLB entries at entries 0 through 7 cannot
be discarded. They are described as "wired". They will therefore not be discarded unless explicitly done
using the control instructions.

## Code example: Initializing the TLB

The following code example initializes the TLB to ensure no match on any kuseg or kseg2 addresses.
This is important. The TLB cannot be initialized to zero as these are all kuseg addresses and would
cause duplicate entries, which again will cause a fatal "TLB shutdown" requiring a hardware reset.

mips_init_tlb:
    mfc0    t0, C0_ENTRYHI              # Save ASID.
    mtc0    zero, C0_ENTRYLO            # tlblo = !valid
    li      a1, NTLBID << TLBIDX_SHIFT  # index
    li      a0, KSEG1_BASE              # tblhi = impossible vpn

    .set    noreorder
1:  
    subu    a1, 1 << TLBIDX_SHIFT
    mtc0    a0, C0_ENTRYHI              # Here we set EntryHi to contain an impossible vpn.
    mtc0    a1, C0_INDEX
    bnez    a1, 1b                      # If a1 is not zero, go back to 1.
    # Because tlbwi is in branch delay slot, we are looping through the TLB writing
    # an entry at each index with impossible vpns.
    tlbwi                               # BDSLOT. Write EntryHi/EntryLo at index.
    .set    reorder

    mtc0    t0, C0_ENTRYHI              # Restore ASID.
    jr      ra

## Code example: Fast kuseg refill handler

The following code example shows the translation mechanism that MIPS architects imagined for a 
Unix-like OS. It builds a page table in memory that consists of a linear array of one-word entries,
indexed by the VPN, whose format is matched to the bitfields of the EntryLo register.

However, since each 4KB(page size) of user address space takes 4 bytes of table space, the entire 2GB
user address space(kuseg) needs a 2MB table, which is a lot of memory. Most user address spaces are used
at the bottom for code and data, and at the top for the downward growing stack. This makes it difficult
to place the page table. The solution is to place the page table in virtual memory itself, in the kseg2
region. This solves two problems: it saves physical memory because the gap between code/data and the stack
will not be referenced, and it provides an easy mechanism for remapping a new user page table when 
switching contexts without having to find enough virtual addresses in the OS to map all the page
tables at once.

The Context register aids in doing this. If the page table starts at a 1MB boundary(since it's in
virtual memory, any gap won't use physical memory) and the Context PTEBase field is filled with the
high-order bits of the page table starting address, then following a user refill exception the Context
register will contain the address of the entry needed for the refill.

    .set    noreorder
    .set    noat
xcpt_vecfastutlb:
    mfc0    k1, C0_CONTEXT  # Get the address for the required page table entry.
    mfc0    k0, C0_EPC      # Store the EPC
    lw      k1, 0(k1)       # Load the page table entry at the Context address.
    nop
    mtc0    k1, C0_ENTRYLO  # Write the page table entry to EntryLo.
    nop
    tlbwr                   # Write EntryHi(automatically setup) and EntryLo into a random TLB entry.
    jr      k0              # Return to caller.
    rfe                     # Restore processor state.
xcpt_endfastutlb:
    .set    at
    .set    reorder

## Simulating dirty bits.

An OS often wants to keep track of whether a page is modified since the OS last obtained it.
Non-modified pages are cheap to discard because they can be easily replaced.

Modified pages are referred to as "dirty" in OS parlance, and the OS must take care of them until
the application program exits or the dirty page is saved away to a backing store.

To help out with this feature, it's common for CISC CPUs to maintain a bit in the memory-resident
page table to indicate that a write-operation to the page has occured, but the MIPS architecture does
not directly implement this. 

The EntryLo D-bit is a write-enable bit, and when set to zero, it signals
that the page is read-only. To simulate dirty bits, the OS should mark new pages with the D-bit cleared.
That page is then considered write-protected and a trap will occur when the page is first modified.
System software can identify this as a legitimate write, but use the event to set a "modified" bit in the
memory-resident tables. It should also set the D-bit in EntryLo so the write can continue.


# Cross-compilation

Compilation is done with the gcc-mips-linux-gnu cross-compilation toolchain.

## Assembling

When assembling with mips-linux-gnu-as, specify the following options:

- -mips1: Compile according to the MIPS I ISA.
- -g: Generate debugging information.
- -o [output_file]: Provide the correct name for your object file.
- -O0: Prevent removal of unneeded NOPs and branch swapping.
- -msoft-float: Floating point instructions is not supported yet. Do not allow floating-point instructions.
- -alg=[file_name]: Write listing with assembly code and general information to file [file_name].

## Dumping

When dumping the object file with mips-linux-gnu-objdump, specify the following options:

**Specify one of these**:
- -g: To specify debug information in object file. 
- -S: To Intermix source code with disassembly.
- -d: To display assembler contents of executable sections.
- -D: To display assembler contents of all sections.

**Optional**:
- -l: Include line numers and filenames in output.
- -Mreg-names=32,reg-names=r3000

## Linking

When linking the object file with mips-linux-gnu-ld, specify the following options:

- -T [file_name]: Read linker script.
- --no-gc-sections: Don't remove unused sections.
- -Map [file_name]: Write a linker map to [file_name].
- --oformat binary: Create binary file.
- --print-output-format: Print default output format.

# Utils

Several utilities are provided to configure and setup the system.

## mkrom

The mkrom utility is used to allocate a ROM-device with the contents provided in
the binary file of size n. If size is omitted, it uses the size of the binary
file.

Usage: mkrom path/binary-file path/rom-name --size=n

# Change log

- 12.05.2024 vCPU: Remove vCPU_state. Add buffers between pipeline stages.

- 10.05.2024: Version 2.0
