
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
static void* DynamicOffset;
static uint8_t FIRST_PHDR = 1;
static Elf64_Shdr* SymTableSectionHeader = 0;

extern Config* Confp;
extern PhdrList InitPhdr;

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
  ehdr->e_type = Confp->outfile_type;
  /* if (Confp->outfile_type == 0) { */    
  /* } else if (Confp->outfile_type == 1) { */
  /*   ehdr->e_type = ET_DYN; */
  /* } */
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
  // ehdr->e_shnum = Confp->shdr_num;
  ehdr->e_shstrndx = 0;//Confp->shstrndx;// SHN_UNDEF;
  __os__write(FileDescriptor, ehdr, sizeof(Elf64_Ehdr));
}

static int convert_sh_to_ph_flags(int sh_flags) {
  int ph_flags = 0;
  if (sh_flags & SHF_WRITE) {
    ph_flags |= PF_W;
  }
  if (sh_flags & SHF_ALLOC) {
    ph_flags |= PF_R;
  }
  if (sh_flags & SHF_EXECINSTR) {
    ph_flags |= PF_X;
  }
  return ph_flags;
}

static SectionContainer* DynamicSectionContainer;
static void* DynamicPhdrOffset;

static int convert_sh_to_ph_type(Elf64_Shdr* shdr) {
  int sh_type = shdr->sh_type;
  int ph_type = 0;
  if (sh_type == SHT_DYNAMIC) {
    // DynamicShdr = shdr;
    // 
    ph_type = PT_LOAD;// PT_DYNAMIC;
  } else {
    ph_type = sh_type;
  }
  return ph_type;
}

void add_nonload_optional_header() {
  printf("w,%p\n", DynamicSectionContainer);
  SectionContainer* sec1 = DynamicSectionContainer;
  SectionChain* sec2 = sec1->init;
  Elf64_Shdr* shdr = sec2->p;
  Elf64_Phdr* phdr = __malloc(sizeof(Elf64_Phdr));
  phdr->p_type = PT_DYNAMIC;// convert_sh_to_ph_type(shdr->sh_type);
  phdr->p_flags = convert_sh_to_ph_flags(shdr->sh_flags);
  phdr->p_offset = DynamicPhdrOffset;//RawDataOffset;
  phdr->p_vaddr = sec1->virtual_address;
  phdr->p_paddr = sec1->virtual_address;
  phdr->p_filesz = sec1->size;
  phdr->p_memsz = sec1->size;
  phdr->p_align = Confp->output_vaddr_alignment;
  __os__write(FileDescriptor, phdr, sizeof(Elf64_Phdr));
}

/* void set_dynamic_phdr() { */
/*   Elf64_Phdr* phdr = __malloc(sizeof(Elf64_Phdr)); */
/*   phdr->p_type = shdr->sh_type; */
/*   phdr->p_flags = convert_sh_to_ph_flags(shdr->sh_flags); */
/*   phdr->p_offset = RawDataOffset; */
/*   phdr->p_vaddr = sec1->virtual_address; */
/*   phdr->p_paddr = sec1->virtual_address; */
/*   __os__write(FileDescriptor, phdr, sizeof(Elf64_Phdr)); */
/*   RawDataOffset += ((phdr->p_filesz + 3) >> 2) << 2;   */
/* } */

/* static PhdrList InitPhdr = {}; */

PhdrList* alloc_phdr(SectionContainer* sec1, Elf64_Shdr* shdr) {

  PhdrList* plist =__malloc(sizeof(PhdrList));
  plist->next = 0;
  Elf64_Phdr* phdr = __malloc(sizeof(Elf64_Phdr));
  plist->phdr = phdr;
  if (shdr->sh_type == SHT_DYNAMIC) {
      phdr->p_type = PT_LOAD;
      DynamicPhdrOffset = RawDataOffset;
      DynamicSectionContainer = sec1;
      // SectionContainer* 
  } else {
    phdr->p_type = shdr->sh_type;// convert_sh_to_ph_type(shdr);
  }
  phdr->p_flags = convert_sh_to_ph_flags(shdr->sh_flags);
  phdr->p_offset = RawDataOffset;
  phdr->p_vaddr = sec1->virtual_address;
  phdr->p_paddr = sec1->virtual_address;
  if (FIRST_PHDR) {
    Confp->entry_address = sec1->virtual_address;
    phdr->p_filesz = Confp->program_header_num * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr);
    phdr->p_memsz = phdr->p_filesz;
    // 1st program header should contain the range of virtual address
    // that program header itself posesses.
    phdr->p_vaddr -= phdr->p_filesz;
    phdr->p_paddr -= phdr->p_filesz;
    FIRST_PHDR = 0;
  }
  phdr->p_filesz += sec1->size;
  phdr->p_memsz += sec1->size;
  phdr->p_align = Confp->output_vaddr_alignment;  
  RawDataOffset += ((phdr->p_filesz + 3/*shdr->sh_addralign3*/) >> 2) << 2;
}

int match_phdr() {
  return 0;
}

void update_phdr() {
  
}

static void __write_program_header(void* arg1) {
  SectionContainer* sec1 = arg1;
  SectionChain* sec2 = sec1->init;  
  Elf64_Shdr* shdr = sec2->p;
  // add program header
  if (sec2 == 0) {
    return;
  }
  if (shdr->sh_type != SHT_PROGBITS && shdr->sh_type != SHT_DYNAMIC) {
    RawDataOffset += ((sec1->size + 3/*shdr->sh_addralign3*/) >> 2) << 2;
    return;
  }
  PhdrList* _plist = &InitPhdr;
  for (;_plist->next;_plist = _plist->next) {
    Elf64_Phdr* phdr = _plist->phdr;
    if (phdr) {
      printf("%p\n", phdr);
      if (match_phdr()) {
	update_phdr();
	return;
      }
    }
  }
  _plist->next = alloc_phdr(sec1, shdr);
  shdr->sh_addr = sec1->virtual_address;//VirtualAddressOffset;
  
  /*
  if (shdr->sh_type == SHT_PROGBITS || shdr->sh_type == SHT_DYNAMIC) {
    Elf64_Phdr* phdr = __malloc(sizeof(Elf64_Phdr));
    plist->phdr = phdr;
    _plist->next = plist;
    if (shdr->sh_type == SHT_DYNAMIC) {
      phdr->p_type = PT_LOAD;
      DynamicPhdrOffset = RawDataOffset;
      DynamicSectionContainer = sec1;
      // SectionContainer* 
    } else {
      phdr->p_type = shdr->sh_type;// convert_sh_to_ph_type(shdr);
    }
    phdr->p_flags = convert_sh_to_ph_flags(shdr->sh_flags);
    phdr->p_offset = RawDataOffset;
    phdr->p_vaddr = sec1->virtual_address;
    phdr->p_paddr = sec1->virtual_address;
    if (FIRST_PHDR) {
      Confp->entry_address = sec1->virtual_address;
      phdr->p_filesz = Confp->program_header_num * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr);
      phdr->p_memsz = phdr->p_filesz;
      // 1st program header should contain the range of virtual address
      // that program header itself posesses.
      phdr->p_vaddr -= phdr->p_filesz;
      phdr->p_paddr -= phdr->p_filesz;
      FIRST_PHDR = 0;
    }
    phdr->p_filesz += sec1->size;
    phdr->p_memsz += sec1->size;
    phdr->p_align = Confp->output_vaddr_alignment;
    __os__write(FileDescriptor, phdr, sizeof(Elf64_Phdr));
    RawDataOffset += ((phdr->p_filesz + 3) >> 2) << 2;
    shdr->sh_addr = sec1->virtual_address;//VirtualAddressOffset;
    // program_header_num += 1;
    // VirtualAddressOffset += 0x200000 + sec1->size;
  } else {
    // Even the section is not added, you need to add RawDataOffset.
    RawDataOffset += ((sec1->size + 3) >> 2) << 2;
    // sec1->size;//((sec1->size + 3) >> 2) << 2;
  }
  */
}

static void write_program_header() {
  printf("ok\n");
  PhdrList* _plist = &InitPhdr;
  Elf64_Phdr* phdr;
  SectionContainer* scon;
  for (;_plist;_plist = _plist->next) {
    printf("w:%p\n", _plist);
    phdr = _plist->phdr;
    if (phdr) {
      printf("w:%p\n", _plist->phdr);
      __os__write(FileDescriptor, phdr, sizeof(Elf64_Phdr));
    }
  }
}

static void write_fixed_program_header() {

  printf("fixed\n");
  // prepare two program headers.
  Elf64_Phdr* phdr = __malloc(sizeof(Elf64_Phdr));
  phdr->p_type = PT_LOAD;
  phdr->p_flags = PF_R | PF_W | PF_X;
  phdr->p_offset = 0;
  // sizeof(Elf64_Ehdr) + 2 * sizeof(Elf64_Phdr);
  phdr->p_vaddr = Confp->base_address;
  phdr->p_paddr = Confp->base_address;
  phdr->p_filesz = Confp->out_size;
  phdr->p_memsz = Confp->out_size;
  phdr->p_align = 0x200000;
  __os__write(FileDescriptor, phdr, sizeof(Elf64_Phdr));
  
  phdr = __malloc(sizeof(Elf64_Phdr));
  phdr->p_type = PT_DYNAMIC;
  phdr->p_flags = PF_R | PF_W;
  phdr->p_offset = DynamicOffset;
  phdr->p_vaddr = Confp->base_address + DynamicOffset;
  phdr->p_paddr = Confp->base_address + DynamicOffset;
  phdr->p_filesz = Confp->dynamic_entry_num * sizeof(Elf64_Dyn);
  phdr->p_memsz = Confp->dynamic_entry_num * sizeof(Elf64_Dyn);
  phdr->p_align = 0x200000;
  __os__write(FileDescriptor, phdr, sizeof(Elf64_Phdr));
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
  if (sc->init == 0) {
    Confp->shdr_num++;
    return;
  }
  printf("write raw data,%s\n",
	 sc->name);
  SectionChain* sec2 = sc->init;
  Elf64_Shdr* shdr = sec2->p;
  int cp = 0;
  int cp2 = 0;
  // alignment needs to be taken into account here.
  cp = __os__seek(FileDescriptor, 0, 1);
  if (!strcmp(sc->name, ".dynamic")) {
    DynamicOffset = cp;
  }
  // cp2 = ((cp + /*shdr->sh_addralign*/3) >> 2) << 2;
  // cp = __os__seek(FileDescriptor, cp2 - cp, 1);  
  for (;sec2;sec2 = sec2->this) {
    shdr = sec2->p;
    cp = __os__seek(FileDescriptor, 0, 1);
    printf("write raw data,%p,%p,%p,%p\n",
	   FileOffset, /*FileOffset + */shdr->sh_offset, shdr->sh_size, sc->virtual_address);

    if (shdr->sh_size)
      __os__write(FileDescriptor, shdr->sh_offset, shdr->sh_size);
    shdr->sh_offset = cp;
  }
  printf("kk\n");
  /* if (shdr->sh_type == SHT_STRTAB) { */
  /*   if (Confp->shstrndx == 0) { */
  /*     Confp->shstrndx = Confp->shdr_num; */
  /*   } else { */
  /*     Confp->strndx = Confp->shdr_num; */
  /*   } */
  /* } */
  Confp->shdr_num++;
}

static void __write_raw_data() {
  PhdrList* _plist = &InitPhdr;
  Elf64_Phdr* phdr;
  SectionContainer* sc;
  for (;_plist;_plist = _plist->next) {
    printf("w\n");
  }
}

static void write_section_header(void* arg1) {
  SectionContainer* sc = arg1;
  if (sc->init == 0) {
    Elf64_Shdr* shdr = __malloc(sizeof(Elf64_Shdr));
    __os__write(FileDescriptor, shdr, sizeof(Elf64_Shdr));
    return;
  }
  SectionChain* sec2 = sc->init;
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
}

void gen(char* fname) {

  FileDescriptor = open__(fname, O_CREAT | O_WRONLY | O_TRUNC);
  // __os__seek(FileDescriptor, sizeof(Elf64_Ehdr), 0);
  __os__seek
    (FileDescriptor,
     sizeof(Elf64_Ehdr) + Confp->program_header_num * sizeof(Elf64_Phdr),
     0);
  // iterate_section_container(write_program_header);  
  // __os__seek(FileDescriptor, 0xb0, 1);
  // if you do not allocate raw_data before program header, you do not know the offset.
  // FileOffset = __os__seek(FileDescriptor, 0, 1);
  iterate_section_container(write_raw_data);
  /* SectionHeaderOffset = __os__seek(FileDescriptor, 0, 1); */
  /* iterate_section_container(write_section_header); */
  __os__seek(FileDescriptor, 0, 0);
  write_elf_header();
  write_fixed_program_header();
  
}

