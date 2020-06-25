
CC=gcc
cflags=-w -nostdlib -fno-jump-tables
header=-I./include
sodir=lib/so
objdir=obj/elf/build
builddir=build
coredir=src/core

crt0=$(objdir)/crt0.o
base_shared=$(sodir)/__z__os.so $(sodir)/__z__mem.so $(sodir)/__z__logger.so

###############################################################

top : pre zbuild __z__loader.so

pre :
	mkdir -p $(objdir)
	mkdir -p $(sodir)

zbuild : $(objdir)/build.o $(crt0) $(base_shared) $(sodir)/__z__build.so
	./zld $(crt0) $< -v -l $(base_shared) $(sodir)/__z__build.so -o $@

$(objdir)/build.o : src/tools/zbuild.c
	$(CC) $(header) $(cflags) -c $< -o $@

buildfile=$(objdir)/config.o $(objdir)/variable.o $(objdir)/rule.o $(objdir)/parse_makefile.o

$(sodir)/__z__build.so : $(buildfile) $(base_shared) $(sodir)/__z__std.so
	./zld $(buildfile) -l $(base_shared) $(sodir)/__z__std.so -o $@

$(objdir)/parse_makefile.o : $(coredir)/build/parse_makefile.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(objdir)/rule.o : $(coredir)/build/rule.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(objdir)/config.o : $(coredir)/build/config.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(objdir)/variable.o : $(coredir)/build/variable.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(sodir)/__z__std.so : $(objdir)/_hash_table.o $(objdir)/thread.o
	./zld $^ -l $(sodir)/__z__os.so $(sodir)/__z__mem.so  -o $@

$(objdir)/_hash_table.o : $(coredir)/std/hash_table.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(objdir)/thread.o : $(coredir)/std/thread.c
	$(CC) $(header) $(cflags) -c $< -o $@

########################################################
# standard shared library

# create os

$(sodir)/__z__os.so : $(objdir)/x64_syscall.o
	./zld $^ -o $@

$(objdir)/x64_syscall.o : $(coredir)/os/linux/x64_syscall.asm
	nasm -felf64 $^ -v -o $@

# create logger

$(sodir)/__z__logger.so : $(objdir)/logger.obj
	./zld $^ -l $(sodir)/__z__os.so -o $@

$(objdir)/logger.obj : $(coredir)/log/logger.c
	$(CC) -nostdlib -c -w $(header) $^ -o $@

# create alloc
$(objdir)/__alloc.obj : $(coredir)/memory/__alloc.c
	$(CC) -nostdlib -c -w $(header) $^ -o $@

$(sodir)/__z__mem.so : $(objdir)/__alloc.obj $(sodir)/__z__os.so $(sodir)/__z__logger.so
	./zld $< -l $(sodir)/__z__os.so $(sodir)/__z__logger.so -o $@

$(objdir)/crt0.o : src/core/loader/elf/crt0.asm
	nasm -felf64 $(header) $^ -o $@

########################################################

# create loader
__z__loader.so : $(objdir)/loader_asm.obj $(objdir)/loader.obj
	./zld $^ -l $(sodir)/__z__os.so $(sodir)/__z__logger.so $(sodir)/__z__mem.so -o $@

$(objdir)/loader_asm.obj : $(coredir)/loader/elf/loader.asm
	nasm -felf64 $(header) $^ -o $@

$(objdir)/loader.obj : $(coredir)/loader/elf/loader.c
	$(CC) -nostdlib -c $^ -w $(header)  -o $@

########################################################

clean : 
	rm -rf $(objdir)/*

