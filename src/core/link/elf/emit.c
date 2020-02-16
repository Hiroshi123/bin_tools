
#include <stdio.h>

#include <limits.h>
#include <fcntl.h>

#include "elf.h"
#include "link.h"
#include "alloc.h"

extern SectionChain* InitialSection;
static void* RawDataOffset = 0;
static int FileDescriptor = 0;
static void* SectionHeaderOffset;
static int NumberOfSectionHeader = 0;
static int NumberOfProgramHeader = 2;
static void* VirtualAddressOffset = 0x400000;
static void* EntryAddress = 0x4000b0;

static void write_elf_header() {
  
  Elf64_Ehdr* ehdr = __malloc(sizeof(Elf64_Ehdr));
  ehdr->e_ident[0] = 0x7f;
  ehdr->e_ident[1] = 'E';
  ehdr->e_ident[2] = 'L';
  ehdr->e_ident[3] = 'F';
  // EI_CLASS
  ehdr->e_ident[4] = 0x02;
  // EI_DATA
  ehdr->e_ident[5] = 0x01;
  // EI_VERSION
  ehdr->e_ident[6] = 0x01;
  // 7,8,9 is special but be 0 meanwhile.
  // and the rest is padded by 0.  
  ehdr->e_type = ET_EXEC;
  ehdr->e_machine = R_IA64_PLTOFF64MSB/**/;
  ehdr->e_version = EV_CURRENT;
  ehdr->e_entry = EntryAddress;
  ehdr->e_phoff = sizeof(Elf64_Ehdr);
  // can you fix section offset at this stage?
  // if not, should wait writing.
  ehdr->e_shoff = 0;//SectionHeaderOffset;
  ehdr->e_flags = 0;
  ehdr->e_ehsize = sizeof(Elf64_Ehdr);
  ehdr->e_phentsize = sizeof(Elf64_Phdr);
  ehdr->e_phnum = NumberOfProgramHeader;
  ehdr->e_shentsize = sizeof(Elf64_Ehdr);
  // 0x3c
  ehdr->e_shnum = NumberOfSectionHeader;
  ehdr->e_shstrndx = SHN_UNDEF;
  __os__write(FileDescriptor, ehdr, sizeof(Elf64_Ehdr));
}

static uint8_t FIRST_PHDR = 1;

static void write_program_header(void* arg1) {
  SectionContainer* sec1 = arg1;
  // arg1;
  SectionChain* sec2 = sec1->this;
  Elf64_Shdr* shdr = sec2->p;
  int size = 0;
  if (!sec1->size) {
    for (;sec2;sec2 = sec2->this) {
      size += ((Elf64_Shdr*)(sec2->p))->sh_size;
    }
    sec1->size = size;
    if (FIRST_PHDR) {
      size += NumberOfProgramHeader * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr);
      FIRST_PHDR = 0;
    }
  }
  printf
    ("!!!%p,%p,%p,%p\n",
     shdr,
     shdr->sh_type,
     shdr->sh_flags,
     shdr->sh_addr
     // shdr->sh_name,
     );
  // add program header
  /* if (shdr->sh_type == 1/\*SHT_PROGBITS*\/) { */
  /*   printf("prog bits,%d\n", size); */
  /*   if (previous_flags != shdr->sh_flags) {       */
  /*     previous_flags = shdr->sh_flags; */
  /*     // same_flags = 0; */
  /*   } else { */
  /*     // same_flags = 1; */
  /*   } */
  /* } */
  
  if (shdr->sh_type == 1/*SHT_PROGBITS*/) {
    Elf64_Phdr* phdr = __malloc(sizeof(Elf64_Phdr));
    phdr->p_type = shdr->sh_type;// type;
    /* // flags(Attribute) */
    // PF_X | PF_W | PF_R
    printf(".. %d,%d\n", PF_X, shdr->sh_flags);
    int ph_flags = 0;
    if (shdr->sh_flags & SHF_WRITE) {
      ph_flags |= PF_W;
    }
    if (shdr->sh_flags & SHF_ALLOC) {
      ph_flags |= PF_R;
    }
    if (shdr->sh_flags & SHF_EXECINSTR) {
      ph_flags |= PF_X;
    }
    phdr->p_flags = ph_flags;
    phdr->p_offset = RawDataOffset;
    phdr->p_vaddr = VirtualAddressOffset;
    phdr->p_paddr = VirtualAddressOffset;
    phdr->p_filesz = size;
    phdr->p_memsz = size;
    phdr->p_align = 0x200000;
    __os__write(FileDescriptor, phdr, sizeof(Elf64_Phdr));
    RawDataOffset += size;
    // NumberOfProgramHeader += 1;
    shdr->sh_addr = VirtualAddressOffset;
    VirtualAddressOffset += 0x200000 + size;
  }
}

static void merge_section_chain() {
  SectionContainer* sec1;
  SectionChain* sec2;
  Elf64_Shdr* shdr;
  int previous_flags = 0;
  for (sec1 = InitialSection;sec1;sec1 = sec1->next) {
    sec2 = sec1->this;
    shdr = sec2->p;
    for (sec2 = sec1->this;sec2;sec2 = sec2->this) {
      // what will be merged???
      // section
      printf("%p,%p,%p,%p\n",
    	     shdr,
    	     shdr->sh_type,
    	     shdr->sh_flags,
    	     shdr->sh_addr
	     // shdr->sh_name,
    	     );
    }
  }
}

static void write_raw_data(void* arg1) {
  SectionContainer* sc = arg1;
  SectionChain* sec2 = sc->this;  
  Elf64_Shdr* shdr = sec2->p;
  void* cp = 0;
  for (;sec2;sec2 = sec2->this) {
    cp = __os__seek(FileDescriptor, 0, 1);
    __os__write(FileDescriptor, shdr->sh_offset, shdr->sh_size);
    shdr->sh_offset = cp;
    printf("write raw data,%p\n", cp);
      // size += ((Elf64_Shdr*)(sec2->p))->sh_size;
  }
  /* __os__seek(FileDescriptor, sizeof(Elf64_Ehdr), 0);   */  
}

static void write_section_header(void* arg1) {
  SectionContainer* sc = arg1;
  SectionChain* sec2 = sc->this;
  Elf64_Shdr* shdr = sec2->p;
  shdr->sh_size = sc->size;
  // printf("%p\n", shdr->sh_name);
  // shdr->sh_addr = VirtualAddressOffset;
  // virtual_address = sec2->virtual_address;
  // shdr->
  __os__write(FileDescriptor, shdr, sizeof(Elf64_Shdr));    
  /* for (;sec2;sec2 = sec2->this) { */
  /*   __os__write(FileDescriptor, shdr, sizeof(Elf64_Shdr)); */
  /*   printf("write section header\n"); */
  /*   // size += ((Elf64_Shdr*)(sec2->p))->sh_size; */
  /* } */
  /* NumberOfSectionHeader += 1; */
}

static void iterate_section(void* callback_f) {
  
  printf("initial section : %p\n",InitialSection);
  
  SectionContainer* sec1;
  SectionChain* sec2;
  Elf64_Shdr* shdr;
  int previous_flags = 0;
  int size = 0;
  int same_flags = 0;
  for (sec1 = InitialSection;sec1;sec1 = sec1->next) {
    printf("s:%p,%p,%p\n", sec1, sec1->this, sec1->next);
    callback_arg2_linux(sec1, callback_f);    
  }
}

void gen() {
  char* fname = "out01.o";
  FileDescriptor = open__(fname, O_CREAT | O_WRONLY | O_TRUNC);
  // printf("gen!,%d\n", fd);
  __os__seek(FileDescriptor, sizeof(Elf64_Ehdr), 0);
  /* NumberOfProgramHeader = 2; */
  iterate_section(write_program_header);
  // __os__seek(FileDescriptor, 0xb0, 1);  
  // if you do not allocate raw_data before program header, you do not know the offset.
  iterate_section(write_raw_data);
  SectionHeaderOffset = __os__seek(FileDescriptor, 0, 1);
  printf("sec head:%p\n", SectionHeaderOffset);
  iterate_section(write_section_header);  
  __os__seek(FileDescriptor, 0, 0);
  write_elf_header();
  
}


