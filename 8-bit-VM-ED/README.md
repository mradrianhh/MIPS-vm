# 8-bit-VM - version 2.0

## Introduction

The goal of this project is to evaluate a PoC software-archicture design for implementing an emulated computer. When reached, we hope to have a design where you can run the virtual machine and provide it a binary file. The machine will then upload the binary data into it's virtual memory and begin execution. While executing, the machine acts as an interpreter that translates the instructions to the host for actual execution.

At this moment, the project attempts to emulate a computer consisting only of a CPU, a clock and some memory. The computer has a very simplified instruction set whose only aim is to accomodate testing of the architecture, and translation of instructions to the host is not in scope yet.

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