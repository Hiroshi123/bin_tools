    gcc -fPIC -w -c -I./include src/core/objectfile/objformat.c -o obj/elf/objformat.obj &&\
    gcc -w -shared obj/elf/objformat.obj -o lib/so/obj_util.so.0 && \

    gcc -w -c src/core/os/file_io.c -o obj/elf/os.obj && \
    gcc -fPIC -w -c src/core/os/mem.c -o obj/elf/mem.obj && \
    gcc -w -shared obj/elf/os.obj obj/elf/mem.obj -o lib/so/os.so.0 && \

    gcc -fPIC -w -c -I./include src/core/log/logger.c -o obj/elf/logger.obj && \
    gcc -w -shared obj/elf/logger.obj lib/so/os.so.0 -ldl -o lib/so/logger.so.0 && \
    
    gcc -fPIC -w -c -I./include src/core/memory/alloc.c -o obj/elf/alloc.obj && \
    gcc -w -shared obj/elf/alloc.obj lib/so/logger.so.0 lib/so/os.so.0 -o lib/so/alloc.so.0 && \

    nasm -felf64 src/core/objectfile/elf/read_elf.asm -o obj/elf/read_elf.obj && \
    gcc -fPIC -w -c -I./include src/core/objectfile/elf/read_elf_callback.c -o obj/elf/read_elf_callback.obj && \
    gcc -w -shared obj/elf/read_elf.obj obj/elf/read_elf_callback.obj -o lib/so/objectfile.so.0 && \
    
    nasm -felf64 src/core/link/callback.asm -o obj/elf/callback.obj && \
    gcc -fPIC -w -c -I./include src/core/link/hashtable.c -o obj/elf/hashtable.obj && \
    gcc -fPIC -w -c -I./include src/core/link/elf/phdr.c -o obj/elf/phdr.obj && \
    gcc -fPIC -w -c -I./include src/core/link/elf/static_data.c -o obj/elf/static_data.obj && \    
    gcc -fPIC -w -c -I./include src/core/link/elf/reloc.c -o obj/elf/reloc.obj && \
    gcc -fPIC -w -c -I./include src/core/link/elf/emit.c -o obj/elf/emit.obj && \
    gcc -fPIC -w -c -I./include src/core/link/elf/callback.c -o obj/elf/elf_callback.obj && \
    gcc -fPIC -w -c -I./include src/core/link/elf/symbol.c -o obj/elf/symbol.obj && \
    gcc -fPIC -w -c -I./include src/core/link/object.c -o obj/elf/object.obj && \
    gcc -fPIC -w -c -I./include src/core/link/elf/section.c -o obj/elf/elf_section.obj && \
    gcc -fPIC -w -c -I./include src/core/link/section.c -o obj/elf/section.obj && \
    gcc -fPIC -w -c -I./include src/core/link/elf/dynamic.c -o obj/elf/dynamic.obj && \

    gcc -fPIC -w -c -I./include src/core/pack/pack.c -o obj/elf/pack.obj && \

    gcc -w -shared obj/elf/pack.obj obj/elf/hashtable.obj obj/elf/phdr.obj obj/elf/static_data.obj obj/elf/symbol.obj obj/elf/object.obj obj/elf/elf_callback.obj obj/elf/callback.obj obj/elf/reloc.obj obj/elf/emit.obj obj/elf/section.obj obj/elf/elf_section.obj obj/elf/dynamic.obj -o lib/so/link.so.0 && \
    gcc -w -c -I./include src/tools/ld_linux.c -o obj/elf/ld_linux.obj && \ 
    gcc -w obj/elf/ld_linux.obj lib/so/objectfile.so.0 lib/so/logger.so.0 lib/so/link.so.0 lib/so/os.so.0 lib/so/alloc.so.0 \
    -o ./ld_linux.o

