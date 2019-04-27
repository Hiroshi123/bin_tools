
# /usr/local/bin/nasm -f macho64 asm01.asm -o asm01.o

/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/data.asm -o data.o

/usr/local/bin/nasm -f macho64 test01.asm -o test01.o

/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/gen.asm -o gen.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/common.asm -o common.o

/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/utils.asm -o utils_asm.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/prim_inst.asm -o prim_inst.o

/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0x00.asm -o inst_0x00.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0x40.asm -o inst_0x40.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0x50.asm -o inst_0x50.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0x60.asm -o inst_0x60.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0x70.asm -o inst_0x70.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0x80.asm -o inst_0x80.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0x90.asm -o inst_0x90.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0xa0.asm -o inst_0xa0.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0xb0.asm -o inst_0xb0.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0xc0.asm -o inst_0xc0.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0xd0.asm -o inst_0xd0.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0xe0.asm -o inst_0xe0.o
/usr/local/bin/nasm -f macho64 ./src/core/asm/x64/inst_0xf0.asm -o inst_0xf0.o


cc -w ex12.c -Wl,inst_0x00.o inst_0x40.o inst_0x50.o inst_0x60.o inst_0x70.o inst_0x80.o inst_0x90.o inst_0xa0.o inst_0xb0.o inst_0xc0.o inst_0xd0.o inst_0xe0.o inst_0xf0.o utils_asm.o prim_inst.o test01.o gen.o common.o data.o -o ex12.o
