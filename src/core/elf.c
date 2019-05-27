
#include "elf.h"
#include "memory.h"
#include "types.h"
#include <string.h>
#include <stdio.h>

#define DEBUG

p_guest load_elf32(uint8_t* page_head, info_on_elf32* _e) {
  
  Elf32_Ehdr* ehdr = (Elf32_Ehdr *)page_head;
  Elf32_Phdr* phdr = (uint8_t*)ehdr + ehdr->e_phoff;
  Elf32_Phdr* phdr_end = phdr + ehdr->e_phnum;
  heap* h1;
  // you should also read offset from file to be mapped directly.
  uint32_t map_size = ((phdr->p_memsz + 0x1000) & 0xfffff000);
  uint32_t bss_size;
  p_guest start_addr;
  for (;phdr!=phdr_end;phdr++) {
    h1 = guest_mmap(phdr->p_vaddr,map_size,1,0);
    memcpy
      (h1->begin,
       page_head + phdr->p_offset,
       phdr->p_filesz
       );
    bss_size = phdr->p_filesz - phdr->p_filesz;
    if (bss_size > 0) {
      memset(h1->begin + bss_size, 0, bss_size);
    }
    // if (ehdr->e_entry ) {
    _e->text_v_addr = phdr->p_vaddr;
    _e->text_p_addr = phdr->p_paddr;
    start_addr = ehdr->e_entry;// + phdr->p_vaddr - phdr->p_paddr;
    // }
  }
  return start_addr;
}

void load_elf(uint8_t* page_head) {
  if (*((uint8_t*)page_head + 4) == 1) {    
    // load_elf32(page_head);
  } else if (*((uint8_t*)page_head + 4) == 2) {
    
  }
}

void read_elf32(const uint8_t* head, info_on_elf32* _e) {
  memset(_e ,0 , sizeof(info_on_elf));
  Elf32_Ehdr* h = (Elf32_Ehdr *)head;
  _e->ehdr_p = (Elf32_Ehdr *)head;
  _e->phdr_p = (Elf32_Phdr *)(head + h->e_phoff);
  _e->shdr_head = (Elf32_Shdr *)(head + h->e_shoff);
  _e->shdr_tail = (Elf32_Shdr *)
    ((size_t)_e->shdr_head +
     h->e_shentsize * h->e_shnum);
  _e->shstr_offset = (char *)
    ((Elf32_Shdr *)
     ((size_t)_e->shdr_head +
      (size_t)(h->e_shentsize *
	       h->e_shstrndx)))->sh_offset+(size_t)head;
  Elf32_Shdr *shdr;
  char *sh_name;
  char i = 0;
  uint8_t mark = 0;
  for (shdr = _e->shdr_head; shdr != _e->shdr_tail; shdr++, i++) {
    sh_name = &(_e->shstr_offset)[shdr->sh_name];
    printf("section:%s\n",sh_name);
    if (shdr->sh_type == SHT_SYMTAB) {
      _e->symbol_p = (Elf64_Sym *)(shdr->sh_offset + head);
      _e->symbol_size = shdr->sh_size;
      continue;
    }
    else if (!strcmp(sh_name, ".strtab")) {
      _e->str_p = (char *)(shdr->sh_offset + (size_t)head);
    } else if (!strcmp(sh_name, ".text")) {
      /* _e->text_shndx = (char)i; */
      _e->text_p = (char*)((size_t)head + shdr->sh_offset);
    }
  }
}

char read_elf64(const char *const page_for_elf,
                            info_on_elf *_e) {
  // filling all of offset information in a prepared struct from a file which
  // are mapped.
  memset(_e ,0 , sizeof(info_on_elf));
  
  printf("%x\n",*((uint8_t*)page_for_elf + 4));
  if (*((uint8_t*)page_for_elf + 4) == 1) {
    printf("!!%x\n",*((uint8_t*)page_for_elf + 4));  
    // read_elf32(page_for_elf, _e);
    return 2;
  }
  _e->ehdr_p = (Elf64_Ehdr *)page_for_elf;
  _e->phdr_p = (Elf64_Phdr *)(page_for_elf + _e->ehdr_p->e_phoff);
  _e->shdr_head = (Elf64_Shdr *)(page_for_elf + _e->ehdr_p->e_shoff);
  _e->shdr_tail = (Elf64_Shdr *)
    ((size_t)_e->shdr_head +
     _e->ehdr_p->e_shentsize * _e->ehdr_p->e_shnum);
  _e->shstr_offset = (char *)
    ((Elf64_Shdr *)
     ((size_t)_e->shdr_head +
      (size_t)(_e->ehdr_p->e_shentsize *
	       _e->ehdr_p->e_shstrndx)))->sh_offset+(size_t)page_for_elf;
  
  // what you need to grab by next loop is filling below pointer in properly,
  // They are
  // 1. pointer to string table,
  // 2. pointer to .rela.plt section
  // 3. pointer to .text section
  // 4. pointer to .symbol section
  
  Elf64_Shdr *shdr;
  char *sh_name;
  char i = 0;
  uint8_t mark = 0;
  for (shdr = _e->shdr_head; shdr != _e->shdr_tail; shdr++, i++) {
    sh_name = &(_e->shstr_offset)[shdr->sh_name];
#ifdef DEBUG
    printf("section:%s\n",sh_name);
#endif
    if (shdr->sh_type == SHT_SYMTAB) {
      _e->symbol_p = (Elf64_Sym *)(shdr->sh_offset + page_for_elf);
      _e->symbol_size = shdr->sh_size;
      mark |= 1;
      // error....
      // If there is no symbol table, then you must generate an error.
      continue;
    } else if (shdr->sh_type == SHT_DYNSYM) {
      _e->dyn_sym_p = (Elf64_Sym *)shdr->sh_offset;
      _e->dyn_sym_size = shdr->sh_size;
      mark |= 1 << 1;
      continue;
    } else if (shdr->sh_type == SHT_STRTAB) {
      if (!strcmp(sh_name, ".strtab")) {
        _e->str_p = (char *)(shdr->sh_offset + (size_t)page_for_elf);
	mark |= 1 << 2;
      } else if (!strcmp(sh_name, ".dynstr")) {
        _e->dyn_str_p = (char *)shdr->sh_offset;
        mark |= 1 << 3;
      }
      continue;
    } else if (shdr->sh_type == SHT_RELA) {
      if (!strcmp(sh_name, ".rela.text")) {
	_e->rela_text_p = (Elf64_Rela *)(shdr->sh_offset + (size_t)page_for_elf);
	_e->rela_text_size = shdr->sh_size;
      }
      if (!strcmp(sh_name, ".rela.plt")) {
        _e->rela_plt_p = (Elf64_Rela *)shdr->sh_offset;
        _e->rela_plt_size = shdr->sh_size;
        mark |= 1 << 4;
      } else if (!strcmp(sh_name, ".rela.dyn")) {
        _e->rela_dyn_p = (Elf64_Rela *)shdr->sh_offset;
        _e->rela_dyn_size = shdr->sh_size;
        mark |= 1 << 5;
      }
      continue;
    } else if (shdr->sh_type == SHT_PROGBITS) {
      if (!strcmp(sh_name, ".text")) {
        _e->text_shndx = (char)i;
	_e->text_p = (char*)((size_t)page_for_elf + shdr->sh_offset);
        mark |= 1 << 7;
      } else if (!strcmp(sh_name, ".plt.got")) {
        _e->plt_got_p = (plt_got *)shdr->sh_offset;
        _e->plt_got_size = shdr->sh_size;
        mark |= 1 << 6;
      }
    }
  }
  if ((mark & 0b111111) != 0b111111) {
    if ((mark & 128) != 128) {
      printf("no .text section");
      return 0;
    }
    /* if ((mark & 16) != 16) { */
    /*   /\* _e->rela_plt_p = (Elf64_Rela*)shdr->sh_offset; *\/ */
    /*   /\* _e->rela_plt_size = shdr->sh_size; *\/ */
    /*   printf("no .rela.plt section.\n.rela.dyn is used instead."); */
    /*   exit(0); */
    /* } */
    if ((mark & 1) != 1) {
      if ((mark & 2) != 2) {
        printf("error:either dynamic or normal symbol table must be on this "
               "elf file\n");
	return 0;
      }
      printf("no symbol table entry on section header table.\n");
      printf("dynamic symbol table will be used instead.\n");
    }
    if ((mark & 4) != 4) {
      if ((mark & 8) != 8) {
	// printf("error:either dynamic or normal symbol table must be on this
        // elf file\n");
        // exit(0);
      }
      printf("no string table entry on section header table.\n");
      printf("dynamic string table will be used instead.\n");
    }
  }
  return 1;
}

void* get_name_of_f_on_elf32(p_guest f_addr, info_on_elf32* _e) {

  const Elf32_Sym *symbol_begin = _e->symbol_p;
  const Elf32_Sym *symbol_end = (Elf32_Sym *)((size_t)symbol_begin + (size_t)_e->symbol_size);
  Elf32_Sym *p = (Elf32_Sym *)symbol_begin;
  // p_guest offset = _e->text_v_addr - _e->text_p_addr;
  for (; p != symbol_end; p++) {
    char* _fname = &(_e->str_p)[p->st_name];
    if (p->st_value == f_addr) {
      printf("match\n");
      return _fname;
    }
  }
  return 0;
}

char* get_addr_of_f(const char* fname, info_on_elf* _e) {
  const Elf64_Sym *symbol_begin = _e->symbol_p;
  const Elf64_Sym *symbol_end = (Elf64_Sym *)((size_t)symbol_begin + (size_t)_e->symbol_size);
  Elf64_Sym *p = (Elf64_Sym *)symbol_begin;
  for (; p != symbol_end; p++) {
    char* _fname = &(_e->str_p)[p->st_name];
    if (!strcmp(fname, _fname)) {
      return (char*)(_e->text_p+p->st_value);
    }
  }
  return 0;
}

char* do_relocs(const char* query, info_on_elf* _e1, info_on_elf* _e2) {
  
  const Elf64_Sym *symbol_begin = _e1->symbol_p;
  const Elf64_Sym *symbol_end = (Elf64_Sym *)((size_t)symbol_begin + (size_t)_e1->symbol_size);
  Elf64_Sym *p = (Elf64_Sym *)symbol_begin;
  char* fname;
  uint32_t caller_head, caller_tail;
  Elf64_Rela* r;
  Elf64_Rela* rela_end;
  
  for (; p != symbol_end; p++) {
    // a subject function which needs to be relocated must be
    // global & function previously.
    if (ELF64_ST_BIND(p->st_info) == STB_GLOBAL &&
        ELF64_ST_TYPE(p->st_info) == STT_FUNC) {
      // a subject function must be on .text section.
      if (p->st_shndx == _e1->text_shndx) {
	fname = &(_e1->str_p)[p->st_name];
	// if the entry of name on symtable matches with a queried function,
	// go to relocation mode.
	if (!memcmp(fname, query, strlen(query))) {
	  caller_head = p->st_value;
	  caller_tail = (uint32_t)p->st_value + p->st_size;
	  if(!_e1->rela_text_p) {
	    printf("no relocation entry\n");
	    return 0;
	  }
	  r = (Elf64_Rela*)_e1->rela_text_p;
	  rela_end =
	    (Elf64_Rela*)((uint64_t)_e1->rela_text_p + _e1->rela_text_size);
	  // to collect callees, you need to take a look at entry of .rela.text
	  // section.
	  for (;r!=rela_end;r++) {
	    // if the offset of the entry is within a range of the head & tail of
	    // a function which is searched, then the relocation entry must be a callee
	    // from the function.
	    if (caller_head < r->r_offset && r->r_offset < caller_tail) {
	      char* callee_name = _e1->str_p + ((symbol_begin+ELF64_R_SYM(r->r_info))->st_name);
	      uint32_t* caller_arg_addr = (uint32_t*)(_e1->text_p + r->r_offset);
	      char* caller_next_addr = (char*)(caller_arg_addr + 1);	      
	      char* callee_addr = get_addr_of_f(callee_name, _e2);
	      if (*caller_arg_addr) {
		printf("callee was already filled\n");
		return (char*)0x1;		
	      }
	      if (!callee_addr) {
		printf("callee was not found\n");
		return (char*)0x0;
	      }
#ifdef DEBUG	      
	      printf("callee:%s\n",callee_name);
#endif	      
	      *caller_arg_addr = (uint32_t)(callee_addr - caller_next_addr);
	      char* res = do_relocs(callee_name, _e2, _e1);
	      if (!res) {
		printf("no mock function on this test function\n");
		return _e1->text_p + p->st_value;
	      }
	    }
	  }
	  return _e1->text_p + p->st_value;
	}
      }
    }
  }
  return (char*)0;
}

