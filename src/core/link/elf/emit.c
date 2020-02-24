
#include <stdio.h>

#include <limits.h>
#include <fcntl.h>

#include "elf.h"
#include "link.h"
#include "alloc.h"

/* extern SectionChain* InitialSection; */
static void* RawDataOffset = 0;
static int FileOffset = 0;
static int FileDescriptor = 0;
static void* SectionHeaderOffset;
static uint8_t FIRST_PHDR = 1;
static Elf64_Shdr* SymTableSectionHeader = 0;

extern Config* Confp;

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
  ehdr->e_entry = Confp->entry_address;
  ehdr->e_phoff = sizeof(Elf64_Ehdr);
  // can you fix section offset at this stage?
  // if not, should wait writing.
  ehdr->e_shoff = SectionHeaderOffset;
  ehdr->e_flags = 0;
  ehdr->e_ehsize = sizeof(Elf64_Ehdr);
  ehdr->e_phentsize = sizeof(Elf64_Phdr);
  ehdr->e_phnum = Confp->program_header_num;
  ehdr->e_shentsize = sizeof(Elf64_Ehdr);
  // 0x3c
  ehdr->e_shnum = Confp->shdr_num;
  ehdr->e_shstrndx = Confp->shstrndx;// SHN_UNDEF;
  __os__write(FileDescriptor, ehdr, sizeof(Elf64_Ehdr));
}

static void write_program_header(void* arg1) {
  SectionContainer* sec1 = arg1;
  // arg1;
  SectionChain* sec2 = sec1->this;
  Elf64_Shdr* shdr = sec2->p;
  /* if (sec1->size == 0) { */
  /*   for (;sec2;sec2 = sec2->this) { */
  /*     sec1->size += ((Elf64_Shdr*)(sec2->p))->sh_size; */
  /*   } */
  /* } */
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
    phdr->p_vaddr = sec1->virtual_address;
    phdr->p_paddr = sec1->virtual_address;
    if (FIRST_PHDR) {
      phdr->p_filesz = Confp->program_header_num * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr);
      phdr->p_memsz = phdr->p_filesz;
      // 1st program header should contain the range of virtual address
      // that program header itself posesses.
      phdr->p_vaddr -= phdr->p_filesz;
      phdr->p_paddr -= phdr->p_filesz;
      printf("called\n");
      FIRST_PHDR = 0;
    }
    phdr->p_filesz += sec1->size;
    phdr->p_memsz += sec1->size;
    phdr->p_align = Confp->output_vaddr_alignment;
    __os__write(FileDescriptor, phdr, sizeof(Elf64_Phdr));
    RawDataOffset += ((phdr->p_filesz + 3) >> 2) << 2;
    // program_header_num += 1;
    shdr->sh_addr = sec1->virtual_address;//VirtualAddressOffset;
    // VirtualAddressOffset += 0x200000 + sec1->size;
  }
}

static void merge_section_chain() {
  SectionContainer* sec1;
  SectionChain* sec2;
  Elf64_Shdr* shdr;
  int previous_flags = 0;
  for (sec1 = Confp->initial_section;sec1;sec1 = sec1->next) {
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
  int cp = 0;
  int cp2 = 0;
  for (;sec2;sec2 = sec2->this) {
    cp = __os__seek(FileDescriptor, 0, 1);
    cp2 = ((cp + 3) >> 2) << 2;
    cp = __os__seek(FileDescriptor, cp2 - cp, 1);    
    __os__write(FileDescriptor, shdr->sh_offset, shdr->sh_size);
    printf("write raw data,%p,%p,%p\n", FileOffset,FileOffset + shdr->sh_offset, cp);
    shdr->sh_offset = cp;
  }
  if (shdr->sh_type == SHT_STRTAB) {
    if (Confp->shstrndx == 0) {
      Confp->shstrndx = Confp->shdr_num;
    } else {
      Confp->strndx = Confp->shdr_num;
    }
  }
  Confp->shdr_num++;
  /* __os__seek(FileDescriptor, sizeof(Elf64_Ehdr), 0);   */  
}

static void write_section_header(void* arg1) {
  SectionContainer* sc = arg1;
  SectionChain* sec2 = sc->this;
  Elf64_Shdr* shdr = sec2->p;
  shdr->sh_size = sc->size;
  printf("sh name : %p\n", shdr->sh_name);
  printf("sh size : %p\n", shdr->sh_size);
  printf("sh addr : %p\n", shdr->sh_addr);
  printf("sh offset : %p\n", shdr->sh_offset);
  printf("-------------------------\n");
  if (shdr->sh_type == SHT_SYMTAB) {
    shdr->sh_link = Confp->strndx;
    // shdr->sh_info = Confp->strndx;    
  }
  __os__write(FileDescriptor, shdr, sizeof(Elf64_Shdr));
  /* for (;sec2;sec2 = sec2->this) { */
  /*   __os__write(FileDescriptor, shdr, sizeof(Elf64_Shdr)); */
  /*   printf("write section header\n"); */
  /*   // size += ((Elf64_Shdr*)(sec2->p))->sh_size; */
  /* } */
  /* NumberOfSectionHeader += 1; */
}

void gen() {
  /* RawDataOffset = Confp->program_header_num * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr); */
  char* fname = "out01.o";
  FileDescriptor = open__(fname, O_CREAT | O_WRONLY | O_TRUNC);
  // printf("gen!,%d\n", fd);
  __os__seek(FileDescriptor, sizeof(Elf64_Ehdr), 0);
  /* program_header_num = 2; */
  iterate_section(write_program_header);
  // __os__seek(FileDescriptor, 0xb0, 1);  
  // if you do not allocate raw_data before program header, you do not know the offset.
  FileOffset = __os__seek(FileDescriptor, 0, 1);
  iterate_section(write_raw_data);
  SectionHeaderOffset = __os__seek(FileDescriptor, 0, 1);
  printf("sec head:%p\n", SectionHeaderOffset);
  iterate_section(write_section_header);  
  __os__seek(FileDescriptor, 0, 0);
  write_elf_header();  
}


