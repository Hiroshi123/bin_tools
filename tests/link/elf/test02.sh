
nasm -felf64 ./tests/link/elf/test02.asm -o ./tests/link/elf/test02.obj
nasm -felf64 ./tests/link/elf/test02_2.asm -o ./tests/link/elf/test02_2.obj

./zld ./tests/link/elf/test02.obj ./tests/link/elf/test02_2.obj -f elf64 -l libc.so.6 -interp-name /lib64/ld-linux-x86-64.so.2 -ef start  -o ./tests/link/elf/test02.o
chmod 777 ./tests/link/elf/test02.o
./tests/link/elf/test02.o
# rm ./tests/link/elf/test02.obj
