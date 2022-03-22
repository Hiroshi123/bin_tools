
gcc -c ./tests/link/elf/test04.c -fPIC -o ./tests/link/elf/test04.obj
gcc -c ./tests/link/elf/test04_2.c -fPIC -o ./tests/link/elf/test04_2.obj

./zld -f elf64 -l libc.so.6 -ef _start_c ./tests/link/elf/test04.obj ./tests/link/elf/test04_2.obj -o ./tests/link/elf/test04.o
chmod 777 ./tests/link/elf/test04.o
./tests/link/elf/test04.o




