# Emulator

## Components

- Back-end(host): Virtual machine to execute IR-code. The VM is run as an interpreter that receives command from the emulator back-end which it executes on the host.
- Back-end(emulator): Emulates the target-machine. It compiles machine code on the target-machine to IR-code, and sends it to the interpreter for execution.
- Front-end(emulator): Connects to the emulator back-end through an API. The front-end can be used to display an emulator GUI, used as a display for the emulated machine, or both.
