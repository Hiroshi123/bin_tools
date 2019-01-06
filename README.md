# bin_tools
call_graph_generation tool on binary file. Generally, misc for reverse engineering.

### Call graph generation tool

The underlying idea is detecting call instruction skimming through .text area of object format file, and generate call graph and attach call graph dependency new section on object format file. Since it has no heavy switch statement, but guess from binary and the jumped address, it is fast. 

It gets symbol table and guess where is the offset of each fucntion on .text area.

Every functon on PLT is the bottom node since the function is going to be linked later on in this current scheme.

#### Target object format file

- elf
- windows PE and Mach-o will be next target.

#### Target assembly

- x86 (so far only: relative near call)
- arm (intended)

#### Caution

Still under development.


