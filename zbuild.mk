
CC=gcc
cflags=-w -nostdlib -fno-jump-tables -fPIC
soflags=-w -nostdlib -fPIC
header=-I./include
sodir=lib/so
objdir=obj/elf/build
builddir=build
coredir=src/core

crt0=$(objdir)/crt0.o
base_shared=$(sodir)/__z__os.so $(sodir)/__z__mem.so $(sodir)/__z__logger.so

###############################################################

top : pre zbuild __z__loader.so $(sodir)/__z__obj.so

pre :
	mkdir -p $(objdir)
	mkdir -p $(sodir)

zbuild : $(objdir)/build.o $(crt0) $(base_shared) $(sodir)/__z__build.so
	./zld $(crt0) $< -v -l $(base_shared) $(sodir)/__z__build.so -o $@

$(objdir)/build.o : src/tools/zbuild.c
	$(CC) $(header) $(cflags) -c $< -o $@

buildfile=$(objdir)/config.o $(objdir)/variable.o $(objdir)/rule.o $(objdir)/parse_makefile.o

$(sodir)/__z__build.so : $(buildfile) $(base_shared) $(sodir)/__z__std.so
	./zld $(buildfile) -v -l $(base_shared) $(sodir)/__z__std.so -o $@

$(objdir)/parse_makefile.o : $(coredir)/build/parse_makefile.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(objdir)/rule.o : $(coredir)/build/rule.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(objdir)/config.o : $(coredir)/build/config.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(objdir)/variable.o : $(coredir)/build/variable.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(sodir)/__z__std.so : $(objdir)/_hash_table.o $(objdir)/thread.o $(objdir)/string.o
	./zld $^ -l $(base_shared)  -o $@

$(objdir)/_hash_table.o : $(coredir)/std/hash_table.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(objdir)/thread.o : $(coredir)/std/thread.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(objdir)/string.o : $(coredir)/std/string.c
	$(CC) $(header) $(cflags) -c $< -o $@

########################################################
# standard shared library

# create os

$(sodir)/__z__os.so : $(objdir)/x64_syscall.o
	./zld $^ -o $@

$(objdir)/x64_syscall.o : $(coredir)/os/linux/x64_syscall.asm
	nasm -felf64 $^ -o $@

# create logger

$(sodir)/__z__logger.so : $(objdir)/logger.obj
	./zld $^ -l $(sodir)/__z__os.so -o $@

$(objdir)/logger.obj : $(coredir)/log/logger.c
	$(CC) $(soflags) -c $(header) $^ -o $@

$(sodir)/__z__mem.so : $(objdir)/__alloc.obj $(sodir)/__z__os.so $(sodir)/__z__logger.so
	./zld $< -l $(sodir)/__z__os.so $(sodir)/__z__logger.so -o $@

# create alloc
$(objdir)/__alloc.obj : $(coredir)/memory/__alloc.c
	$(CC) $(soflags) -c $(header) $^ -o $@

$(objdir)/crt0.o : src/core/loader/elf/crt0.asm
	nasm -felf64 $(header) $^ -o $@

########################################################

# create loader
__z__loader.so : $(objdir)/loader_asm.obj $(objdir)/elf_loader.obj $(objdir)/pe_loader.obj
	./zld -nointerp -v $^ -l $(sodir)/__z__os.so $(sodir)/__z__logger.so $(sodir)/__z__mem.so $(sodir)/__z__obj.so -o $@

$(objdir)/loader_asm.obj : $(coredir)/loader/elf/loader.asm
	nasm -felf64 $(header) $^ -o $@

$(objdir)/elf_loader.obj : $(coredir)/loader/elf/loader.c
	$(CC) $(soflags) -c $^ -w $(header)  -o $@

$(objdir)/pe_loader.obj : $(coredir)/loader/pe/loader.c
	$(CC) $(soflags) -c $^ -w $(header)  -o $@

# do not confuse with objectfile.so.0 which is used for linker.
# this is dirty separation, which needs to be fixed later on.
$(sodir)/__z__obj.so : $(objdir)/objformat.obj $(objdir)/read_elf.obj $(objdir)/read_coff.obj $(objdir)/read_pe.obj
	./zld $^ -l $(sodir)/__z__os.so $(sodir)/__z__logger.so $(sodir)/__z__mem.so -o $@

#  $(objdir)/read_elf_callback.obj

$(objdir)/objformat.obj : $(coredir)/objectfile/__objformat.c
	$(CC) $(soflags) -c $(header) $^ -o $@

$(objdir)/read_elf.obj : $(coredir)/objectfile/elf/read_elf.asm
	nasm -felf64 $^ -o $@

$(objdir)/read_coff.obj : $(coredir)/objectfile/coff_pe/read_coff.asm
	nasm -felf64 $^ -o $@

$(objdir)/read_pe.obj : $(coredir)/objectfile/coff_pe/__read_pe.c
	$(CC) -w $(soflags) -c $(header) $^ -o $@

########################################################

clean : 
	rm -rf $(objdir)/*

