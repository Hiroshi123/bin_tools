
CC=gcc
cflags=-w -nostdlib -fno-jump-tables -fPIC
header=-I./include
sodir=lib/so
objdir=obj/elf/zob
builddir=build
coredir=src/core
soflags=-w -nostdlib -fPIC

crt0=obj/elf/crt0.o
base_shared=$(sodir)/__z__os.so $(sodir)/__z__mem.so $(sodir)/__z__logger.so

asm=src/core/asm/x86

###############################################################

# top : pre zob $(sodir)/__z__dis_x86.so $(sodir)/__z__obs.so $(sodir)/__z__obj.so

top : pre zob $(sodir)/__z__obs.so $(sodir)/__z__obj.so $(objdir)/__z__loader.obj $(objdir)/__z__stub.obj

pre :
	mkdir -p $(objdir)
	mkdir -p $(sodir)

zob : $(objdir)/zob.o
	./zld $(crt0) $< -v -l $(base_shared) $(sodir)/__z__obj.so $(sodir)/__z__std.so $(sodir)/__z__obs.so -o $@

$(objdir)/zob.o : src/tools/zob.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(sodir)/__z__dis_x86.so: $(objdir)/data.o $(objdir)/common.o $(objdir)/gen.o $(objdir)/prim_inst.o $(objdir)/inst_0x00.o $(objdir)/inst_0x10.o $(objdir)/inst_0x20.o $(objdir)/inst_0x30.o $(objdir)/inst_0x40.o $(objdir)/inst_0x50.o $(objdir)/inst_0x60.o $(objdir)/inst_0x70.o $(objdir)/inst_0x80.o $(objdir)/inst_0x90.o $(objdir)/inst_0xa0.o $(objdir)/inst_0xb0.o $(objdir)/inst_0xc0.o $(objdir)/inst_0xd0.o $(objdir)/inst_0xe0.o $(objdir)/inst_0xf0.o $(objdir)/inst_0x0f00.o $(objdir)/inst_0x0f10.o $(objdir)/inst_0x0f20.o $(objdir)/inst_0x0f40.o $(objdir)/inst_0x0f50.o $(objdir)/inst_0x0f60.o $(objdir)/inst_0x0f70.o $(objdir)/inst_0x0f80.o $(objdir)/inst_0x0f90.o $(objdir)/inst_0x0fa0.o $(objdir)/inst_0x0fb0.o $(objdir)/inst_0x0fc0.o $(objdir)/inst_0x0fd0.o $(objdir)/inst_0x0fe0.o $(objdir)/inst_0x0ff0.o $(objdir)/disasm.o
	./zld $^ -l $(base_shared) $(sodir)/__z__std.so -v -o  $@

$(objdir)/data.o : $(asm)/data.asm
	nasm -felf64 -i./include/ $< -o $@

$(objdir)/common.o : $(asm)/common.asm
	nasm -felf64 $< -o $@

$(objdir)/gen.o : $(asm)/gen.asm
	nasm -felf64 $< -o $@

$(objdir)/prim_inst.o : $(asm)/prim_inst.asm
	nasm -felf64 -i./include/ $< -o $@

$(objdir)/inst_0x00.o: $(asm)/inst_0x00.asm
	nasm -felf64 -i./include/ $< -o $@
$(objdir)/inst_0x10.o: $(asm)/inst_0x10.asm
	nasm -felf64 -i./include/  $< -o $@
$(objdir)/inst_0x20.o: $(asm)/inst_0x20.asm
	nasm -felf64 -i./include/  $< -o $@
$(objdir)/inst_0x30.o: $(asm)/inst_0x30.asm
	nasm -felf64 -i./include/  $< -o $@
$(objdir)/inst_0x40.o: $(asm)/inst_0x40.asm
	nasm -felf64 -i./include/  $< -o $@
$(objdir)/inst_0x50.o: $(asm)/inst_0x50.asm
	nasm -felf64 -i./include/  $< -o $@
$(objdir)/inst_0x60.o: $(asm)/inst_0x60.asm
	nasm -felf64 -i./include/  $< -o $@
$(objdir)/inst_0x70.o: $(asm)/inst_0x70.asm
	nasm -felf64 -i./include/ $< -o $@
$(objdir)/inst_0x80.o: $(asm)/inst_0x80.asm
	nasm -felf64 -i./include/ $< -o $@
$(objdir)/inst_0x90.o: $(asm)/inst_0x90.asm
	nasm -felf64 -i./include/ $< -o $@
$(objdir)/inst_0xa0.o: $(asm)/inst_0xa0.asm
	nasm -felf64 -i./include/ $< -o $@
$(objdir)/inst_0xb0.o: $(asm)/inst_0xb0.asm
	nasm -felf64 -i./include/ $< -o $@
$(objdir)/inst_0xc0.o: $(asm)/inst_0xc0.asm
	nasm -felf64 -i./include/ $< -o $@
$(objdir)/inst_0xd0.o: $(asm)/inst_0xd0.asm
	nasm -felf64 -i./include/ $< -o $@
$(objdir)/inst_0xe0.o: $(asm)/inst_0xe0.asm
	nasm -felf64 -i./include/ $< -o $@
$(objdir)/inst_0xf0.o: $(asm)/inst_0xf0.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f00.o: $(asm)/inst_0x0f00.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f10.o: $(asm)/inst_0x0f10.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f20.o: $(asm)/inst_0x0f20.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f30.o: $(asm)/inst_0x0f30.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f40.o: $(asm)/inst_0x0f40.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f50.o: $(asm)/inst_0x0f50.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f60.o: $(asm)/inst_0x0f60.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f70.o: $(asm)/inst_0x0f70.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f80.o: $(asm)/inst_0x0f80.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0f90.o: $(asm)/inst_0x0f90.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0fa0.o: $(asm)/inst_0x0fa0.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0fb0.o: $(asm)/inst_0x0fb0.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0fc0.o: $(asm)/inst_0x0fc0.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0fd0.o: $(asm)/inst_0x0fd0.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0fe0.o: $(asm)/inst_0x0fe0.asm
	nasm -i./include/ -felf64 $< -o $@
$(objdir)/inst_0x0ff0.o: $(asm)/inst_0x0ff0.asm
	nasm -i./include/ -felf64 $< -o $@

$(objdir)/disasm.o: $(asm)/disasm.c
	$(CC) $(header) $(cflags) -c $< -o $@

$(sodir)/__z__obs.so : $(objdir)/t01_asm.obj $(objdir)/t01.obj
	./zld $^ -v -l $(base_shared) -o $@

$(objdir)/t01_asm.obj : src/core/obs/t01.asm
	nasm -felf64 $< -o $@

$(objdir)/t01.obj : src/core/obs/t01.c
	$(CC) $(header) $(cflags) -c $< -o $@

clean :
	rm -rf $(objdir)/*

# do not confuse with objectfile.so.0 which is used for linker.
# this is dirty separation, which needs to be fixed later on.
$(sodir)/__z__obj.so : $(objdir)/objformat.obj $(objdir)/read_elf.obj $(objdir)/read_elf_asm.obj $(objdir)/read_coff.obj $(objdir)/read_pe.obj
	./zld $^ -l $(base_shared) -o $@

#  $(objdir)/read_elf_callback.obj

$(objdir)/objformat.obj : $(coredir)/objectfile/__objformat.c
	$(CC) -w $(cflags) -c $(header) $^ -o $@

$(objdir)/read_elf_asm.obj : $(coredir)/objectfile/elf/read_elf.asm
	nasm -felf64 $^ -o $@

$(objdir)/read_elf.obj : $(coredir)/objectfile/elf/__read_elf.c
	$(CC) -w $(cflags) -c $(header) $^ -o $@

$(objdir)/read_coff.obj : $(coredir)/objectfile/coff_pe/read_coff.asm
	nasm -felf64 $^ -o $@

$(objdir)/read_pe.obj : $(coredir)/objectfile/coff_pe/__read_pe.c
	$(CC) -w $(cflags) -c $(header) $^ -o $@


# no DT_NEEDED loader which are used for loading unpacked executable

$(objdir)/__z__loader.obj : $(objdir)/loader_asm.obj $(objdir)/nodeps_elf_loader.obj
	./zld -nointerp -v $^ -o $@

# $(objdir)/__z__stub.obj : $(objdir)/loader_asm.obj $(objdir)/stub.obj
# 	./zld -nointerp -v $^ -o $@

$(objdir)/loader_asm.obj : $(coredir)/loader/elf/loader.asm
	nasm -felf64 $(header) $^ -o $@

$(objdir)/nodeps_elf_loader.obj : $(coredir)/loader/elf/nodeps_loader.c
	$(CC) $(soflags) -c $^ -w $(header)  -o $@

$(objdir)/__z__stub.obj : $(coredir)/loader/elf/stub.c
	$(CC) $(soflags) -c $^ -w $(header)  -o $@

