# __z__tools

![Build Status](https://travis-ci.org/Hiroshi123/bin_tools.svg?branch=master)

Misc for reverse engineering.

### zld 

A linker which is intended for anti-malicious staff)  

Its documentation is on wiki(https://github.com/Hiroshi123/bin_tools/wiki/zld-documentation).

### zbuild 

buildtool which aims to be compatible with GNU make

### zemu (x86-64 emulator)

=> Still cannot be built

---

Duplicated and no longer maintained commands

* objtest (execution for functions in a pre-linked object format flle)
* callgen (call_graph_generation) 
* bipatch (binary patch command)

## Aim

It was designed from my necessity to understand a binary file which does not have any source code efficiently.

## build

```

make -f zld.mk
make -f zbuild.mk

```
