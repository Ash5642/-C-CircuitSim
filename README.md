# -C-CircuitSim
Features
  -Simulates both combinational and sequential digital circuits with dynamic input & output sizes.
  -Load circuits from files: load circuits from circuit text files. Circuit files should have a _CircuitFile_ header.
  -Save circuit to file: circuit loaded in memory can be saved to an output fike.
  -Save resulting output to file, which can be used to verify the output of the circuit loaded in memory. Output files should have a _ResFile_ header.
  -Merge circuits: circuits can be merged in memory, remapping outputs of the destination to inputs of inputs of the source.
  -Dynamic allocation: can support a large component count limited by memory size.
Problems:
  -Memory leakeage, overflow and use after free problems somewhat common.
