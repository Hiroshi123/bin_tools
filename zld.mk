
cc=gcc
header=-I./include
sodir=lib/so
objdir=obj/elf/link
coredir=src/core

#####################################################

make : pre ld_linux.o

pre :
	mkdir -p $(objdir)
	mkdir -p $(sodir)

#####################################################

ld_linux.o : $(objdir)/ld_linux.obj $(sodir)/objectfile.so.0 $(sodir)/logger.so.0 $(sodir)/link.so.0 $(sodir)/os.so.0 $(sodir)/alloc.so.0
	$(cc) -w $^ -o $@

$(objdir)/ld_linux.obj : src/tools/ld_linux.c
	$(cc) -w -c $(header) $^ -o $@

#####################################################

$(sodir)/link.so.0 : $(objdir)/pack.obj $(objdir)/hashtable.obj $(objdir)/phdr.obj $(objdir)/static_data.obj $(objdir)/symbol.obj $(objdir)/object.obj $(objdir)/elf_callback.obj $(objdir)/callback.obj $(objdir)/reloc.obj $(objdir)/emit.obj $(objdir)/section.obj $(objdir)/elf_section.obj $(objdir)/dynamic.obj
	$(cc) -w -shared $^ -o $@

$(objdir)/callback.obj : $(coredir)/link/callback.asm
	nasm -felf64 $^ -o $@

$(objdir)/hashtable.obj : $(coredir)/link/hashtable.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/phdr.obj : $(coredir)/link/elf/phdr.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/static_data.obj : $(coredir)/link/elf/static_data.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/reloc.obj : $(coredir)/link/elf/reloc.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/emit.obj : $(coredir)/link/elf/emit.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/elf_callback.obj : $(coredir)/link/elf/callback.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/symbol.obj : $(coredir)/link/elf/symbol.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/object.obj : $(coredir)/link/object.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/elf_section.obj : $(coredir)/link/elf/section.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/section.obj : $(coredir)/link/section.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/dynamic.obj : $(coredir)/link/elf/dynamic.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/pack.obj : $(coredir)/pack/pack.c
	$(cc) -fPIC -w -c $(header) $^ -o $@


##################################################################

$(sodir)/os.so.0 : $(objdir)/os.obj $(objdir)/mem.obj
	$(cc) -w -shared $^ -o $@

$(objdir)/os.obj : $(coredir)/os/file_io.c
	$(cc) -w -c $^ -o $@

$(objdir)/mem.obj : $(coredir)/os/mem.c
	$(cc) -fPIC -w -c $^ -o $@

##################################################################

$(sodir)/logger.so.0 : $(objdir)/logger.obj $(sodir)/os.so.0
	$(cc) -w -shared $^ -o $@

$(objdir)/logger.obj : $(coredir)/log/logger.c
	gcc -fPIC -w -c -I./include $^ -o $@

$(sodir)/alloc.so.0 : $(objdir)/alloc.obj $(sodir)/logger.so.0 $(sodir)/os.so.0
	gcc -w -shared $^ -o $@

$(objdir)/alloc.obj : src/core/memory/alloc.c
	gcc -fPIC -w -c -I./include $^ -o $@

##################################################################

$(sodir)/objectfile.so.0 : $(objdir)/read_elf.obj $(objdir)/read_elf_callback.obj
	$(cc) -w -shared $^ -o $@

$(objdir)/read_elf.obj : $(coredir)/objectfile/elf/read_elf.asm
	nasm -felf64 $^ -o $@

$(objdir)/read_elf_callback.obj : $(coredir)/objectfile/elf/read_elf_callback.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

##################################################################

clean :
	rm obj/elf/link/*


