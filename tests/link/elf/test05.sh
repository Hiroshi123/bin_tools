#!/bin/bash

g++ -fPIC -c ./tests/link/elf/test05.cpp -o ./tests/link/elf/test05.obj
# gcc -nostdlib -c ./tests/link/elf/test03_2.c -o ./tests/link/elf/test03_2.obj
./zld -f elf64 -l libc.so.6 -ef main ./tests/link/elf/test05.obj -o ./tests/link/elf/test05.o
chmod 777 ./tests/link/elf/test05.o
./tests/link/elf/test05.o
val=$?
if [ "$val" -eq 0 ];
then
    echo "ok"
else
    echo "fail"
fi


