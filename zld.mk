
cc=gcc
header=-I./include
sodir=lib/so
objdir=obj/elf/link
coredir=src/core

#####################################################

make : pre zld

pre :
	mkdir -p $(objdir)
	mkdir -p $(sodir)

#####################################################

zld : $(objdir)/zld.obj $(sodir)/objectfile.so.0 $(sodir)/__z__logger.so.0 $(sodir)/link.so.0 $(sodir)/os.so.0 $(sodir)/alloc.so.1
	$(cc) -w $^ -o $@

$(objdir)/zld.obj : src/tools/zld.c
	$(cc) -w -c $(header) $^ -o $@

#####################################################

$(sodir)/link.so.0 : $(objdir)/pack.obj $(objdir)/hashtable.obj $(objdir)/phdr.obj $(objdir)/static_data.obj $(objdir)/elf_symbol.obj $(objdir)/object.obj $(objdir)/elf_callback.obj $(objdir)/callback.obj $(objdir)/elf_reloc.obj $(objdir)/elf_emit.obj $(objdir)/section.obj $(objdir)/elf_section.obj $(objdir)/dynamic.obj $(objdir)/coff_callback.obj $(objdir)/coff_reloc.obj $(objdir)/coff_import.obj $(objdir)/coff_export.obj $(objdir)/coff_section.obj $(objdir)/coff_symbol.obj $(objdir)/coff_emit.obj
	$(cc) -w -shared $^ -o $@

$(objdir)/callback.obj : $(coredir)/link/callback.asm
	nasm -felf64 $^ -o $@

$(objdir)/hashtable.obj : $(coredir)/link/hashtable.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

###############################################################
# ELF
###############################################################

$(objdir)/phdr.obj : $(coredir)/link/elf/phdr.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/static_data.obj : $(coredir)/link/elf/static_data.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/elf_reloc.obj : $(coredir)/link/elf/reloc.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/dynamic.obj : $(coredir)/link/elf/dynamic.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/elf_emit.obj : $(coredir)/link/elf/emit.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/elf_callback.obj : $(coredir)/link/elf/callback.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/elf_symbol.obj : $(coredir)/link/elf/symbol.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/elf_section.obj : $(coredir)/link/elf/section.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

###############################################################
# COFF
###############################################################

$(objdir)/coff_callback.obj : $(coredir)/link/coff/callback.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/coff_reloc.obj : $(coredir)/link/coff/reloc.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/coff_import.obj : $(coredir)/link/coff/import.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/coff_export.obj : $(coredir)/link/coff/export.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/coff_section.obj : $(coredir)/link/coff/section.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/coff_symbol.obj : $(coredir)/link/coff/symbol.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/coff_emit.obj : $(coredir)/link/coff/emit.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

###############################################################
# common
###############################################################

$(objdir)/object.obj : $(coredir)/link/object.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/section.obj : $(coredir)/link/section.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

$(objdir)/pack.obj : $(coredir)/pack/pack.c
	$(cc) -fPIC -w -c $(header) $^ -o $@

##################################################################

$(sodir)/os.so.0 : $(objdir)/x64_syscall.o
	$(cc) -w -shared $^ -o $@

$(objdir)/x64_syscall.o : $(coredir)/os/linux/x64_syscall.asm
	nasm -felf64 $^ -o $@

$(objdir)/os.obj : $(coredir)/os/file_io.c
	$(cc) -w -c $^ -o $@

$(objdir)/mem.obj : $(coredir)/os/mem.c
	$(cc) -fPIC -w -c $^ -o $@

##################################################################

$(sodir)/logger.so.0 : $(objdir)/logger.obj $(sodir)/os.so.0
	$(cc) -w -shared $^ -o $@

$(objdir)/logger.obj : $(coredir)/log/logger.c
	gcc -fPIC -w -c -I./include $^ -o $@

$(sodir)/alloc.so.1 : $(objdir)/alloc.obj $(sodir)/logger.so.0 $(sodir)/os.so.0
	gcc -w -shared $^ -o $@

$(objdir)/alloc.obj : src/core/memory/__alloc.c
	gcc -fPIC -w -c -I./include $^ -o $@

##################################################################

$(sodir)/objectfile.so.0 : $(objdir)/objformat.obj $(objdir)/read_elf.obj $(objdir)/read_coff.obj $(sodir)/os.so.0
	$(cc) -w -nostdlib -shared $^ -o $@

#  $(objdir)/read_elf_callback.obj

$(objdir)/objformat.obj : $(coredir)/objectfile/__objformat.c
	$(cc) -fPIC -w -nostdlib -c $(header) $^ -o $@

$(objdir)/read_coff.obj : $(coredir)/objectfile/coff_pe/read_coff.asm
	nasm -felf64 $^ -o $@

$(objdir)/read_elf.obj : $(coredir)/objectfile/elf/read_elf.asm
	nasm -felf64 $^ -o $@

$(objdir)/read_elf_callback.obj : $(coredir)/objectfile/elf/read_elf_callback.c
	$(cc) -fPIC -w -nostdlib -c $(header) $^ -o $@

##################################################################

clean :
	rm obj/elf/link/*


