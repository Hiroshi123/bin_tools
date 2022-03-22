#!/bin/bash


nasm -felf64 ./tests/link/elf/test01.asm -o ./tests/link/elf/test01.obj
# nasm -felf64 ./tests/link/coff/test01_2.asm -o ./tests/link/coff/test01_2.obj
./zld -f elf64 ./tests/link/elf/test01.obj -o ./tests/link/elf/test01.o
val=$?
if [ "$val" -eq 0 ];
then
    echo "linker error"
fi

chmod 777 ./tests/link/elf/test01.o
./tests/link/elf/test01.o
rm ./tests/link/elf/test01.obj

