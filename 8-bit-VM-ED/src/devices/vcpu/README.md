# Fetch-Decode-Execute
At each clock_cycle/tick, a new instruction is fetched, 
the previous instruction is decoded
and the instruction before that is being executed.

This means that the fetch, execute and decode has to work in parallel.
In practice, we'll start of by treating them sequentially.
The parallelism of the pipeline is emulated by storing the previous state.

In practice, this means that the previous state is the input to the fetch-cycle,
and the fetch-cycle modifies the new state.
 
The same previous state is the input to the decode-cycle, 
and the decode-cycle modifies the new state.

The same previous state is the input to the execute-cycle,
and the execute-cycle modifies the new state.