
# /usr/local/bin/nasm -f macho64 asm01.asm -o asm01.o

/usr/bin/nasm -f elf64 ./src/core/asm/x64/data.asm -o data.o

# /usr/bin/nasm -f elf64 test01.asm -o test01.o

cc -w -c -I./include src/core/memory.c -o obj/memory.o
cc -w -c -I./include src/core/objformat.c -o obj/objformat.o
cc -w -c -I./include src/core/elf.c -o obj/elf.o
# cc -w -c -I./include src/core/macho.c -o obj/macho.o
cc -w -c -I./include src/core/coff.c -o obj/coff.o
cc -w -c -I./include src/core/pe.c -o obj/pe.o

/usr/bin/nasm -f elf64 ./src/core/asm/x64/gen.asm -o gen.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/common.asm -o common.o

/usr/bin/nasm -f elf64 ./src/core/asm/x64/utils.asm -o utils_asm.o
/usr/bin/nasm -i./include/ -f elf64 ./src/core/asm/x64/prim_inst.asm -o prim_inst.o

/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x00.asm -o inst_0x00.o

/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x20.asm -o inst_0x20.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x30.asm -o inst_0x30.o

/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x40.asm -o inst_0x40.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x50.asm -o inst_0x50.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x60.asm -o inst_0x60.o
/usr/bin/nasm -i/root/myrepo/bin_tools/include/ -f elf64 ./src/core/asm/x64/inst_0x70.asm -o inst_0x70.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x80.asm -o inst_0x80.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x90.asm -o inst_0x90.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0xa0.asm -o inst_0xa0.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0xb0.asm -o inst_0xb0.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0xc0.asm -o inst_0xc0.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0xd0.asm -o inst_0xd0.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0xe0.asm -o inst_0xe0.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0xf0.asm -o inst_0xf0.o

/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x0f80.asm -o inst_0x0f80.o
/usr/bin/nasm -f elf64 ./src/core/asm/x64/inst_0x0fb0.asm -o inst_0x0fb0.o

cc -w ex12.c -I./include -Wl,inst_0x00.o inst_0x20.o inst_0x30.o inst_0x40.o inst_0x50.o inst_0x60.o inst_0x70.o inst_0x80.o inst_0x90.o inst_0xa0.o inst_0xb0.o inst_0xc0.o inst_0xd0.o inst_0xe0.o inst_0xf0.o inst_0x0f80.o inst_0x0fb0.o utils_asm.o prim_inst.o gen.o common.o data.o obj/memory.o obj/objformat.o obj/elf.o obj/pe.o obj/coff.o  -o ex12.o
