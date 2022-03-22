
gcc -nostdlib -c ./tests/link/elf/test03.c -o ./tests/link/elf/test03.obj
gcc -nostdlib -c ./tests/link/elf/test03_2.c -o ./tests/link/elf/test03_2.obj
./zld -f elf64 -l libc.so.6 ./tests/link/elf/test03.obj ./tests/link/elf/test03_2.obj -o ./tests/link/elf/test03.o
chmod 777 ./tests/link/elf/test03.o
./tests/link/elf/test03.o

