#!/bin/bash

g++ -c -m32 ./tests/link/elf/test05.cpp -o ./tests/link/elf/test06.obj
# gcc -nostdlib -c ./tests/link/elf/test03_2.c -o ./tests/link/elf/test03_2.obj
./zld -f elf32 -l libc.so.6 -ef main ./tests/link/elf/test06.obj -o ./tests/link/elf/test06.o
#chmod 777 ./tests/link/elf/test06.o
#./tests/link/elf/test06.o

