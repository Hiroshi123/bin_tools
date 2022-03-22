
#include <stdio.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"

// this file is obsoleted as it is

PhdrList InitPhdr = {};

static uint8_t FIRST_PHDR = 1;
extern Config* Confp;
static void* RawDataOffset = 0;

static int convert_sh_to_ph_type(Elf64_Shdr* shdr) {
  int sh_type = shdr->sh_type;
  int ph_type = 0;
  switch (sh_type) {
    case SHT_PROGBITS:
      ph_type = PT_LOAD;
      break;
    case SHT_DYNAMIC:
      ph_type = PT_LOAD;
      break;
    case SHT_DYNSYM:
      ph_type = PT_LOAD;
      break;
    case SHT_STRTAB:
      ph_type = PT_LOAD;
      break;
    default:
      ph_type = sh_type;
      printf("no type\n");
      break;
  }
  /* if (sh_type == SHT_DYNAMIC) { */
  /*   ph_type = PT_LOAD;// PT_DYNAMIC; */
  /* }  else if (sh_type == SHT_DYNSYM) { */
  /* } else { */
  /*   ph_type = sh_type; */
  /* } */
  return ph_type;
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

static PhdrList* alloc_phdr(SectionContainer* sec1, Elf64_Shdr* shdr) {
  PhdrList* plist = __malloc(sizeof(PhdrList));
  plist->scon_init = sec1;
  plist->scon_tail = sec1;
  plist->next = 0;
  Elf64_Phdr* phdr = __malloc(sizeof(Elf64_Phdr));
  plist->phdr = phdr;
  if (shdr->sh_type == SHT_DYNAMIC) {
    phdr->p_type = PT_DYNAMIC;
    // DynamicPhdrOffset = RawDataOffset;
    // DynamicSectionContainer = sec1;
    // SectionContainer*
  } else {
    phdr->p_type = PT_LOAD;
  }
  /* else if (shdr->sh_type == SHT_STRTAB  || shdr->sh_type == SHT_DYNSYM) { */
  /*   phdr->p_type = PT_LOAD;// convert_sh_to_ph_type(shdr); */
  /* } */
  phdr->p_flags = convert_sh_to_ph_flags(shdr->sh_flags);
  phdr->p_offset = RawDataOffset;
  phdr->p_vaddr = sec1->virtual_address;
  phdr->p_paddr = sec1->virtual_address;
  if (FIRST_PHDR) {
    Confp->entry_address = sec1->virtual_address;
    phdr->p_filesz =
        Confp->program_header_num * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr);
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
  RawDataOffset += ((phdr->p_filesz + 3 /*shdr->sh_addralign3*/) >> 2) << 2;
  return plist;
}

static int match_phdr(PhdrList* _plist, Elf64_Shdr* shdr) {
  Elf64_Phdr* phdr = _plist->phdr;
  int pt_type = convert_sh_to_ph_type(shdr);
  int pt_flags = convert_sh_to_ph_flags(shdr->sh_flags);
  printf("match,%p,%p,%p,%p\n", phdr->p_type, pt_type, phdr->p_flags, pt_flags);
  if (phdr->p_type == pt_type /*PT_LOAD*/ && phdr->p_flags == pt_flags) {
    printf("w,%p\n", phdr->p_type);
    return 1;
  }
  return 0;
}

static void update_phdr() {}

/* void count_program_header(void* arg1) { */
/*   Confp->program_header_num += 1;   */
/* } */

SectionContainer* NonAllocatedSectionContainerInit = 0;
SectionContainer* NonAllocatedSectionContainer = 0;

void set_program_header(void* arg1) {
  SectionContainer* sec1 = arg1;
  SectionChain* sec2 = sec1->init;
  printf("a\n");
  if (!sec2) return;
  Elf64_Shdr* shdr = sec2->p;
  if (!shdr) return;

  printf("type:%p,%p\n", shdr->sh_type, shdr->sh_flags);
  if ((shdr->sh_flags & SHF_ALLOC) == 0) {
    printf("no alloc\n");
    if (NonAllocatedSectionContainer) {
      NonAllocatedSectionContainer->next = sec1;
    } else {
      NonAllocatedSectionContainerInit = sec1;
    }
    NonAllocatedSectionContainer = sec1;
    return;
  }
  if (shdr->sh_type != SHT_PROGBITS && shdr->sh_type != SHT_DYNAMIC) {
    // RawDataOffset += ((sec1->size + 3/*shdr->sh_addralign3*/) >> 2) << 2;
    // return;
  }
  PhdrList* pre = &InitPhdr;
  PhdrList* _plist = pre->next;
  Elf64_Phdr* phdr;
  SectionContainer* sc;
  /* _plist = _plist->next; */
  for (; _plist; _plist = _plist->next) {
    pre = _plist;
    phdr = _plist->phdr;
    // printf("sc:%p\n", _plist->scon);
    if (phdr) {
      // phdr,Scon,next
      if (match_phdr(_plist, shdr)) {
        update_phdr();
        // _plist->scon_tail = _plist->scon_;
        sc = _plist->scon_tail;
        sc->next = sec1;
        if (shdr->sh_type == SHT_DYNAMIC) {
          break;
        }

        return;
      } else {
        printf("rep\n");
      }
    }
  }
  pre->next = alloc_phdr(sec1, shdr);
  // shdr->sh_addr = sec1->virtual_address;
  Confp->program_header_num += 1;
}

void sort_section_container_complete() {
  NonAllocatedSectionContainer->next = 0;
  printf("------------\n");
  Elf64_Phdr* phdr;
  PhdrList* _plist = &InitPhdr;
  // Elf64_Phdr* phdr;
  SectionContainer* scon1;
  SectionContainer* scon2;
  for (; _plist->next; _plist = _plist->next) {
    if (_plist->next) {
      scon1 = _plist->scon_tail;
      scon2 = _plist->next->scon_init;
      if (scon1) {
        scon1->next = scon2;
      }
    }
    printf("s,%p,%p\n", _plist->scon_init, _plist->scon_tail);
  }
  scon1 = _plist->scon_tail;
  scon1->next = NonAllocatedSectionContainerInit;
}
