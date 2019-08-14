
#include "macho.h"
#include "memory.h"
#include "objformat.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/stat.h>

uint32_t GUEST_UPPER_UPPER32;

extern uint64_t EXPORT(meta_page_ptr);
extern uint64_t EXPORT(fd_num);
extern char* PAGE_PATH;
extern p_guest EXPORT(current_page_tail);

// what it does is just copying data from A to B.

void map_lc_segment64() {
  
}

void read_macho64(void* head, info_on_macho* macho, uint8_t do_map) {
  _mach_header_64* mh = (_mach_header_64 *)head;
  /* printf("%d:%d:%d\n",mh->filetype, mh->ncmds,mh->sizeofcmds);   */
  if (mh->magic != MH_MAGIC_64) {
    printf("error\n");
  }
  // now load_command will be started from here.
  _load_command* p = (_load_command*)(mh+1);
  uint8_t c = 0;
  _segment_command_64* seg;
  _section_64* sec;
  uint32_t nsyms;
  int fd;
  p_host tmp_page_tail;
  // After mach-header, you are going to iterate on
  // load commands.
  // As length of load commands are differed by the command,
  // you are not able to set the final pointer that you should
  // reach at the end of the loop.
  for (;c<mh->ncmds;c++,p = (_load_command*)((char*)p + p->cmdsize)) {
#ifdef DEBUG
    printf("load_commands:%u,%d\n",p->cmd, p->cmdsize);
#endif
    // after detecting kinds of the load command, you need to cast
    // the pointer of the load commands to the each specific commands,
    switch (p->cmd) {
    case LC_SYMTAB: {
      _symtab_command* _sym1 = (_symtab_command *)p;
#ifndef DEBUG
      printf("offset:%u,%u,%u,%u\n",
	     _sym1->symoff,
	     _sym1->nsyms,
	     _sym1->stroff,
	     _sym1->strsize);
#endif
      macho->sym_begin = (_nlist_64*)((size_t)mh+_sym1->symoff);
      nsyms = _sym1->nsyms;
      macho->sym_end = (size_t)macho->sym_begin + _sym1->nsyms*sizeof(_nlist_64);
      macho->str_begin = (_nlist_64*)((size_t)mh+_sym1->stroff);
      macho->str_end = (size_t)macho->str_begin + _sym1->strsize;
      break;
    }
    case LC_DYSYMTAB: {
      _dysymtab_command* _sym2 = (_dysymtab_command *)p;
      macho->dysym_cmd_begin = (_dysymtab_command *)p;
      macho->dysym_begin = (_nlist_64*)((size_t)mh+(_sym2->indirectsymoff&0x0000ffff));
      macho->dysym_end = (_nlist_64*)((size_t)macho->dysym_begin + (_sym2->nindirectsyms)*sizeof(uint32_t));
      break;
    }
    case LC_SEGMENT_64: {
      seg = (_segment_command_64*)p;
      // page zero seems to be set as the beginning of section intending setting the
      // offset of virtual address.
      // This mapping should be seperated from normal mapping as its range is large and
      // initialize as 0.
      if (do_map && !strcmp(seg->segname,SEG_PAGEZERO)) {
	GUEST_UPPER_UPPER32 = seg->vmaddr + seg->filesize;
	if (0xffffffff & seg->vmaddr) {
	  fprintf(stderr, "address is starting from non zero\n");
	}
      } else if (do_map && !strcmp(seg->segname,SEG_TEXT)) {
	uint32_t guest_addr = EXPORT(current_page_tail) + seg->vmaddr - GUEST_UPPER_UPPER32;
	if (seg->vmsize != seg->filesize) {
	  fprintf(stderr, "file size must be equal to vmsize.\n something strange\n");
	}
	uint32_t map_size = ((seg->filesize + 0x0fff) & 0xfffff000);
	uint32_t flags = 0; // tmp maxprot(initprot) should be fed here.
	uint64_t name_or_parent_addr = "";
	/* dir_name = strrchr(PAGE_PATH,"/");	 */
	/* DIR* dir = opendir("mydir"); */
	/* if (dir) {	   */
	/* } else if (ENOENT == errno) {	   */
	/* } */
	/* mkdir(,0x777); */
	fd = open(&PAGE_PATH, O_RDWR | O_CREAT | O_TRUNC);
	EXPORT(fd_num) += 1;
	heap* h = guest_mmap
	  (guest_addr, map_size, flags, name_or_parent_addr, fd);
	set_page_path2(get_page_path2() + 1);
	memcpy(h->begin,(size_t)mh + seg->fileoff,seg->filesize);
        tmp_page_tail = guest_addr + map_size;
      } else if (do_map && !strcmp(seg->segname,SEG_DATA)) {
	uint32_t guest_addr = EXPORT(current_page_tail) + seg->vmaddr - GUEST_UPPER_UPPER32;
	// note that seg->filesize will be smaller than vmsize(memory size)
	// in case vmsize contains uninitialized section such as bss.
	// they are going to be manually initialized as 0 by this loader.
	uint32_t map_size = ((seg->vmsize + 0x0fff) & 0xfffff000);
	uint32_t flags = 0; // tmp maxprot(initprot) should be fed here.
	uint64_t name_or_parent_addr = "";
	printf("page path:%s\n", &PAGE_PATH);
	fd = open(&PAGE_PATH, O_RDWR | O_CREAT | O_TRUNC);
	EXPORT(fd_num) += 1;
	heap* h = guest_mmap
	  (guest_addr, map_size, flags, name_or_parent_addr, fd);
	set_page_path2(get_page_path2() + 1);
	// initialization of data section.
	memcpy(h->begin,(size_t)mh + seg->fileoff,seg->filesize);
	tmp_page_tail = guest_addr + map_size;
	// initialization of bss section.
	uint32_t bss_size = seg->vmsize - seg->filesize;
	/* printf("bss:%x,%x,%x,%x\n", bss_size,h->begin,(size_t)mh + seg->fileoff,seg->filesize); */
	// memset(0, h->begin + seg->filesize, bss_size);
      } else if (do_map && !strcmp(seg->segname,SEG_LINKEDIT)) {
	if (seg->vmsize != seg->filesize) {
	  fprintf(stderr, "file size must be equal to vmsize.\n something strange\n");
	}
	uint32_t guest_addr = EXPORT(current_page_tail) + seg->vmaddr - GUEST_UPPER_UPPER32;
	uint32_t map_size = ((seg->filesize + 0x0fff) & 0xfffff000);
	uint32_t flags = 0; // tmp maxprot(initprot) should be fed here.
	uint64_t name_or_parent_addr = "";
	fd = open(&PAGE_PATH, O_RDWR | O_CREAT | O_TRUNC);
	EXPORT(fd_num) += 1;
	heap* h = guest_mmap
	  (guest_addr, map_size, flags, name_or_parent_addr, fd);
	set_page_path2(get_page_path2() + 1);
	memcpy(h->begin,(size_t)mh + seg->fileoff,seg->filesize);
	tmp_page_tail = guest_addr + map_size;
      }
      _section_64* sec_begin = (_section_64*)(seg+1);
      _section_64* sec_end = (_section_64*)((size_t)sec_begin + seg->nsects * sizeof(_section_64));
      for (sec=sec_begin;sec!=sec_end;sec++) {	
        if (!strcmp(sec->sectname,SECT_TEXT)) {
	  macho->text_begin = (size_t)mh + sec->offset;
	  macho->text_end = (size_t)macho->text_begin+sec->size;
	  // relocation information needs to be retrived.
	  macho->reloff = (size_t)mh + sec->reloff;
	  macho->nreloc = sec->nreloc;
	}
	// this is stubs section which behaves procedure linkage table on elf format.	  
	if (sec->flags && (S_SYMBOL_STUBS || S_ATTR_SOME_INSTRUCTIONS) ==
	    S_LAZY_SYMBOL_POINTERS || S_ATTR_SOME_INSTRUCTIONS) {
	  macho->symbol_stubs_begin = (char*)((size_t)mh + sec->offset);
	  macho->symbol_stubs_end = (char*)(macho->symbol_stubs_begin + sec->size);
	}
	// this section must be set of pointers towards dynamic symbol address.
	// it is global offset table on elf format.
	if (sec->flags && S_LAZY_SYMBOL_POINTERS == S_LAZY_SYMBOL_POINTERS) {
	  macho->lazy_symbol_begin = (char*)((size_t)mh + sec->offset);
	  macho->lazy_symbol_end = (char*)(macho->lazy_symbol_begin + sec->size);	    
	}
      }
      break;
    }
    case LC_DYLD_INFO_ONLY: {
      // this is the load command where export symbols are defined.
      struct dyld_info_command* info_cmd = (struct dyld_info_command*)(p+1);      
      break;
    }
    case LC_LOAD_DYLINKER:
      break;
    case LC_LOAD_DYLIB:
      // if it has multiple LC_LOAD_DYLIB has multiple,
      // then, record only the first one.
      if (macho->dylib_head == 0) {
	macho->dylib_head = p;
      }
      struct dylib* dy = (p+1);
      printf("%s\n",(uint8_t*)(dy + 1));// dy->name.offset);
      macho->dylib_n += 1;
      break;
    case LC_MAIN:{
      struct entry_point_command* entry_cmd = (struct entry_point_command*)p;
      macho->entry = (uint32_t) (0xffffffff & entry_cmd->entryoff);
      macho->stacksize = (uint32_t) (0xffffffff & entry_cmd->stacksize);
      break;
    }
    case LC_FUNCTION_STARTS:
      break;
    case LC_DATA_IN_CODE:
      break;      
    case LC_VERSION_MIN_MACOSX:
      break;
    }
  }
  EXPORT(current_page_tail) = tmp_page_tail;
}

void load_macho32(void* p, info_on_macho* macho) {

  // not yet...
  // read_macho64(p, macho, 1);
}


void load_macho64(void* p, info_on_macho* macho) {
  read_macho64(p, macho, 1);
}

// this funciton is relocation between 2 macho files.
// The relocation entry on macho which had been specified as 1st arugment
// is resolved against internally and against 2nd argument macho.
// Note that relocation from 2nd towards 1st is not done in this function.

char* do_reloc_all(/*const char* query,*/ info_on_macho* _e1, info_on_macho* _e2) {

  // 1. you go to relocation section,
  // 2. you get address which needs to be resolved(you do not know what is the caller in that stage)
  // 3. you access nlist(macho symbol table) and grab the symbol name accessing the string table
  // 4. if the name was not defined on another macho which was provided as second argument,
  
  uint64_t* p = _e1->reloff;
  uint64_t* end = p + _e1->nreloc;
  char resolved = 0;
  for (;p!=end;p++) {
    resolved = 0;
    // first 4 byte of relocation entry is the offset from beginning of text section
    // where you need to resolve.
    uint32_t* _p = (uint32_t*)p;
    // next 2 byte is the index of symbol table entry.
    uint16_t* __p = (uint16_t*)((uint8_t*)p + 4);    
    uint32_t* caller_arg_addr = (uint32_t*)(_e1->text_begin + *_p);
    if (*caller_arg_addr) {
      printf("callee was already filled\n");
      continue;
    }
    char* caller_next_addr = (char*)(caller_arg_addr+1);
    _nlist_64* nl = (_nlist_64*)(_e1->sym_begin + *__p);
    char* name = (char*)((size_t)_e1->str_begin + nl->n_un.n_strx);
#ifdef DEBUG
    printf("relocation entry:%s\n",name);
#endif
    // after you grab the address & name of callee,
    // you need to iterate symbol table of this macho and another macho
    // to have address of matched callee function.

    // Relocation priority is self(internal)-first, external-second.
    nl = _e1->sym_begin+_e1->dysym_cmd_begin->iextdefsym;
    _nlist_64* nl_end = nl+_e1->dysym_cmd_begin->nextdefsym;
    char* n;
    char* callee_addr;
    for (;nl!=nl_end;nl++) {
      n = (char*)((size_t)_e1->str_begin + nl->n_un.n_strx);
      if (!strcmp(name, n)) {
	callee_addr = _e1->text_begin + nl->n_value;
	*caller_arg_addr = (uint32_t)(callee_addr - caller_next_addr);
	resolved = 1;
	break;
      }
    }
    nl = _e2->sym_begin+_e2->dysym_cmd_begin->iextdefsym;
    nl_end = nl+_e2->dysym_cmd_begin->nextdefsym;
    for (;nl!=nl_end;nl++) {
      n = (char*)((size_t)_e2->str_begin + nl->n_un.n_strx);
      if (!strcmp(name, n)) {
	callee_addr = _e2->text_begin + nl->n_value;
	*caller_arg_addr = (uint32_t)(callee_addr - caller_next_addr);
	resolved = 1;
	break;
      }
    }
    if (!resolved) {
      printf("no relocation entry which can be resolved.\n");      
    }
  }
}

char* find_test(char* query, info_on_macho* _e1) {  
  _nlist_64* nl = (_nlist_64*)(_e1->sym_begin+_e1->dysym_cmd_begin->iextdefsym);
  _nlist_64* nl_end = nl+_e1->dysym_cmd_begin->nextdefsym;
  for (;nl!=nl_end;nl++) {
    char* fname = (char*)((size_t)_e1->str_begin + nl->n_un.n_strx);
    if (!memcmp(fname, query, strlen(query))) {
      return _e1->text_begin + nl->n_value;
    }
  }
  return 0;
}

// memory funcion will allocate something for it. you can just set

/* void set_macho_header(void* p) { */
/*   _mach_header_64* _p = (_mach_header_64*)p;   */
/*   _p->magic = 0; */
/*   _p->cputype = 0; */
/*   _p->cpusubtype = 0; */
/*   _p->cpusubtype = 0; */  
/* } */

void set_nlist(void* p) {
  _nlist_64* _p = (_nlist_64*)p;
  _p->n_type = 0;
  _p->n_sect = 0;
  _p->n_desc = 0;
  _p->n_value = 0;
}

void set_binary() {}

// 1. copy macho header(which are ) 
// 2. copy load command
// 3. you should just add 4 load command,

// 1. LC_SEGMENT_64
// 2. VERSION INFORMAION
// 3. SYMTAB
// 4. DYSYMTAB

// you should at first expect each size....
// youd need to add neither load commands nor section information..

// Set struct and you can feed some values for it.....

// before 864
// number of necessary section is also defined......

// external files
// SEGMENT_64

// 464

// LC_SEGMENT_64 needs 

// add LC_SEGMENT64
// you do not add any section in that stage....
// coping is addition...

// version is fixed...

// put temporarily on workspace symbol....
// put LC_SYMTAB without any entries.....
// meta information is defined after everything was set......
// nsyms 16byte + entry.....

// meta information for load command(fixed)
// meta information for section information (not fixed)
// section information & symbol table...

// fadd
// the last offset that you want to insert a function
// this is on symbol table.
// you should at first insert a new symbol table & name entry,
// rest of entry are going to be added individually.

// 16 (name)
// struct nlist {}

// 1. 

// LOAD_COMMAND(464 = 32 + 432)

// 

// SYMTAB & LD_SEGMENT_64 not copying them
//

// can you define patch format for it?????
// probably not....

// In macho, you relcoate 


// SECTIONS()

// .text section(we have lots of byte.. where would you like to insert????)

// you define bytes..
// 

// there is no caller-callee dependencies 

// in macOSX, it would be better to at first directly go to relocation information.
// in other words, it would be not good to guess set of callees from a caller because
// the relation is not obvious.
// instead, you should resolve all of relocation in a entry, because
// relocation informaiton has offset from the head of text section(address)
// and the index of symbol table

// if you get the pair of address & name,
// then you should search if the address is defined exzernally or internally.
// if the address is defined 

// relocation 

void iterate_on_symbol_table(info_on_macho* macho) {
  
  _nlist_64* nl = macho->sym_begin;
  printf("nl:%x\n",nl);
  char* name;
  for (;nl!=macho->sym_end;nl++) {
    printf("nl:%x\n",nl);
    name = (char*)((size_t)macho->str_begin + nl->n_un.n_strx);
    if (nl->n_type == 15) {
      if (memcmp(name,"_test__",7)) {
	
	// when you arrive a test case,
	// you need to iterate a relocation information entries & if you
	// find the entry whose callees are likely to be called from
	// a value....,
	// then you can relocate its entry...
	
	// value + .text beginning 
      }
    }
    /* if (!strcmp(name,"_f1")) { */
    /*   uint8_t* addr = (uint8_t*)(/\*(uint64_t)macho->mh+*\/((uint64_t)nl->n_value & 0x0000ffff)); */
    /*   int n=0; */
    /*   printf("%lx,%lx\n",addr,nl->n_value); */
    /*   // addr = (char*)((size_t)mh); */
    /*   for (;n<30;addr++,n++) { */
    /*     printf("%lx\n", *addr); */
    /* 	if (*addr == 0xe8) { */
    /* 	  //uint32_t* off = *(uint32_t*)(addr+1); */
    /* 	  // off + addr+5; */
    /* 	} */
    /*   } */
    /* } */
    printf("name:%s\n",name);
    printf("type:%d\n",nl->n_type);
    printf("sect:%d\n",nl->n_sect);
    printf("desc:%d\n",nl->n_desc);
    printf("value:%x\n",nl->n_value);
    printf("-------\n");
  }
  
  printf("-----ii--\n");
  
  uint32_t* dyn = macho->dysym_begin;
  printf("%lx,%lx\n",dyn,macho->dysym_end);
  for (;dyn!=macho->dysym_end;dyn++) {
    printf("%x,%x\n",dyn,*dyn);
    printf("-------\n");
  }
  printf("%x\n",macho->text_begin);
  printf("%x\n",macho->reloff);
  //printf("%x\n",macho->reloff);
  printf("%d\n",sizeof(_nlist_64));
  return;
}

heap* do_map(char* name) {
  int fd = open(name, O_RDONLY);
  EXPORT(fd_num) += 1;
  uint32_t header_size = 0;
  enum OBJECT_FORMAT o = detect_format(fd, &header_size);
  if (o == MACHO64) {
    EXPORT(meta_page_ptr) += sizeof(info_on_macho);
    struct stat stbuf;
    if (fstat(fd, &stbuf) == -1) {
      close(fd);
      fprintf(stderr, "stbuf error\n");
      return NULL;
    }
    printf("get.path:%02c\n",get_page_path1());

    set_page_path1(get_page_path1() + 1);
    printf("get.path:%02c\n",get_page_path1());
    printf("get.path:%02c\n",get_page_path2());
    printf("get.path:%d\n",get_page_path2());
    set_page_path2('0');
    printf("get.path:%02c\n",get_page_path2());
    printf("get.path:%d\n",get_page_path2());

    /* *(pp+3) = '0'; */
    heap * h = map_file(fd, stbuf.st_size, -1);
    load_macho64(h->begin, EXPORT(meta_page_ptr));
    // start_addr = ((info_on_macho*)(meta->begin))->entry;
    return h;
  }
  return NULL;
}

void do_reloc(const char* fname, info_on_macho* _e1) {
  
  // open();
  _load_command* p = _e1->dylib_head;
  char* name;
  int count = 0;
  for (;count < _e1->dylib_n;p = ((uint8_t*)p + p->cmdsize),count++) {
    name = (uint8_t*)p + sizeof(struct load_command) + sizeof(struct dylib);
    printf("!!!%s\n",name);
    // if name is not yet mapped, then start mapping,
    heap* h = search_page_by_name(name);
    if (!h) {
      printf("do mmapp!!!\n");
      h = do_map(name);
    }
    // you should get meta(info_on_elf) after mapping.
    
  }
}

p_host get_name_of_f_on_macho64(p_guest f_addr, info_on_macho* _e1) {
  
  /* printf("!!!%lx\n",f_addr); */
  _nlist_64* nl = (_nlist_64*)(_e1->sym_begin+_e1->dysym_cmd_begin->iextdefsym);
  // _nlist_64* nl_end = nl+_e1->dysym_cmd_begin->nextdefsym;
  _e1->dysym_cmd_begin->nextdefsym;  
  _nlist_64* nl_end = nl+_e1->dysym_cmd_begin->nextdefsym + _e1->dysym_cmd_begin->nundefsym;
  for (;nl!=nl_end;nl++) {
    char* fname = (char*)((size_t)_e1->str_begin + nl->n_un.n_strx);
    printf("%lx,%x,%x,%s\n",0x0000ffff & nl->n_value, nl->n_sect , nl->n_desc, fname);
    if (f_addr == (0x0000ffff & nl->n_value)) {

      if (!nl->n_value) {
	do_reloc(fname, _e1);
	
      }      
      printf("match!\n");
      /* return fname; */
    }
  }
  return 0;
}

