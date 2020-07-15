

# assembly .asm file to win32/win64 architecture

nasm -fwin64 ./tests/link/coff/test01_1.asm -o ./tests/link/coff/test01_1.obj
nasm -fwin64 ./tests/link/coff/test01_2.asm -o ./tests/link/coff/test01_2.obj

./zld -f win64 ./tests/link/coff/test01_1.obj ./tests/link/coff/test01_2.obj -o ./tests/link/coff/test01.exe


