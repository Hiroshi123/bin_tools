
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "call.h"

/*
This piece of command will generate call graph anaöizing a binary file.
The call graph can be generated as .dot format which can be converted to
any image format.
The function call dependencies can also be set as a section header.
*/

__attribute__((constructor)) void set_heap_header() {
  PAGE_SIZE = getpagesize();
  void *tmp = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (tmp == MAP_FAILED) {
    printf("error\n");
    exit(0);
  }
  HEAP_HEADER_ADDR_HEAD = HEAP_HEADER_ADDR_P = tmp;
  HEAP_HEADER_ADDR_TAIL = tmp + PAGE_SIZE;
}

__attribute__((destructor)) void unset_heap_header() {

  heap *h = (heap *)HEAP_HEADER_ADDR_HEAD;
  for (; h != HEAP_HEADER_ADDR_P; h++) {
    munmap(h->begin, h->page_num * PAGE_SIZE);
    if (h->file)
      close(h->file);
  }
}

const char check_elf(const uint8_t const *const e) {
  return (e[EI_MAG0] == ELFMAG0 && e[EI_MAG1] == ELFMAG1 &&
          e[EI_MAG2] == ELFMAG2 && e[EI_MAG3] == ELFMAG3);
}

// original elf file is going to be mapped on the beginning on virtual memory.

const heap *map_elf(const void *const page_for_elf, const char *const fname) {
  int fd = open(fname, O_RDWR);
  if (fd == -1)
    exit(0);
  struct stat stbuf;
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    exit(0);
  }
  const size_t map_size = (stbuf.st_size + 0x1000) & 0xfffff000;
  void *begin = mmap(page_for_elf, map_size, PROT_READ /*|PROT_WRITE*/,
                     MAP_SHARED | MAP_FIXED, fd, 0);
  if (begin == MAP_FAILED) {
    printf("error:%u\n", errno);
    close(fd);
    exit(0);
  } else if (begin != page_for_elf)
    printf("mapped in a different memory.\n");
  if (!check_elf((uint8_t *)begin)) {
    printf("this is not elf format file...\n");
    close(fd);
    exit(0);
  }
  heap *h = (heap *)HEAP_HEADER_ADDR_P;
  h->begin = begin;
  h->page_num = map_size / PAGE_SIZE;
  h->flags = 1;
  h->file = (uint16_t)fd;
  HEAP_HEADER_ADDR_P = h + 1;
  return h;
}

void retrieve_info_from_sht(const char const *const page_for_elf,
                            info_on_elf *const _e) {

  // filling all of offset information in a prepared struct from a file which
  // are mapped.
  _e->ehdr_p = (Elf64_Ehdr *)page_for_elf;
  _e->phdr_p = (Elf64_Phdr *)_e->ehdr_p->e_phoff;
  _e->shdr_head = (Elf64_Shdr *)_e->ehdr_p->e_shoff;
  _e->shdr_tail = (Elf64_Shdr *)(_e->ehdr_p->e_shoff +
                                 _e->ehdr_p->e_shentsize * _e->ehdr_p->e_shnum);
  _e->shstr_offset = (char *)((Elf64_Shdr *)((size_t)_e->shdr_head +
                                             (size_t)(_e->ehdr_p->e_shentsize *
                                                      _e->ehdr_p->e_shstrndx)))
                         ->sh_offset;

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
    sh_name = &_e->shstr_offset[shdr->sh_name];
    if (shdr->sh_type == SHT_SYMTAB) {
      _e->symbol_p = (Elf64_Sym *)shdr->sh_offset;
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
        _e->str_p = (char *)shdr->sh_offset;
        mark |= 1 << 2;
      } else if (!strcmp(sh_name, ".dynstr")) {
        _e->dyn_str_p = (char *)shdr->sh_offset;
        mark |= 1 << 3;
      }
      continue;
    } else if (shdr->sh_type == SHT_RELA) {
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
      exit(0);
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
        exit(0);
      }
      _e->symbol_p = 0;
      _e->symbol_size = 0;
      printf("no symbol table entry on section header table.\n");
      printf("dynamic symbol table will be used instead.\n");
    }
    if ((mark & 4) != 4) {
      if ((mark & 8) != 8) {
        // printf("error:either dynamic or normal symbol table must be on this
        // elf file\n");
        exit(0);
      }
      _e->str_p = 0;
      printf("no string table entry on section header table.\n");
      printf("dynamic string table will be used instead.\n");
    }
  }
}

uint64_t estimate_size(const info_on_elf *const _in) {
  uint64_t count = 0;
  const Elf64_Sym *symbol_begin =
      _in->symbol_p ? _in->symbol_p : _in->dyn_sym_p;
  const Elf64_Sym *symbol_end =
      _in->symbol_p
          ? (Elf64_Sym *)((size_t)symbol_begin + (size_t)_in->symbol_size)
          : (Elf64_Sym *)((size_t)symbol_begin + (size_t)_in->dyn_sym_size);
  Elf64_Sym *p = symbol_begin;
  for (; p != symbol_end; p++) {
    if (ELF64_ST_BIND(p->st_info) == STB_GLOBAL &&
        ELF64_ST_TYPE(p->st_info) == STT_FUNC)
      count++;
  }
  return count;
}

const heap *map_smaller_symbol_table(const void *map_begin,
                                     const info_on_elf *const _e) {
  const size_t map_size =
      (estimate_size(_e) * sizeof(Elf64_Sym) + 0x1000) & 0xfffff000;
  void *addr = mmap(map_begin, map_size, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  if (addr == MAP_FAILED) {
    printf("error!\n");
  }
  heap *h = (heap *)HEAP_HEADER_ADDR_P;
  h->begin = addr;
  h->page_num = map_size / PAGE_SIZE;
  h->flags = 1;
  h->file = 0;
  HEAP_HEADER_ADDR_P = h + 1;
  return h;
}

void retrieve_info_from_symt(const info_on_elf *const _in,
                             info_on_workspace *const _out) {

  // iteration inside symbol table
  /* Elf64_Sym* symbol_begin; */

  const Elf64_Sym *symbol_begin =
      _in->symbol_p ? _in->symbol_p : _in->dyn_sym_p;
  const Elf64_Sym *symbol_end =
      _in->symbol_p
          ? (Elf64_Sym *)((size_t)symbol_begin + (size_t)_in->symbol_size)
          : (Elf64_Sym *)((size_t)symbol_begin + (size_t)_in->dyn_sym_size);
  Elf64_Sym *p = symbol_begin;
  for (; p != symbol_end; p++) {

    if (ELF64_ST_BIND(p->st_info) == STB_GLOBAL &&
        ELF64_ST_TYPE(p->st_info) == STT_FUNC) {
      if (p->st_shndx == _in->text_shndx) {
        // this is the condition where this entry is normal func on .text
        // section. memcpy from original page to workspace page.
        memcpy(_out->map_f_end, p, sizeof(Elf64_Sym));
        // in workspace, you need another 64byte for indicating address
        // not only st_value as .so make use of plt address for inter-function
        // calls. we will put it on st_size(64byte) as we do not use st_size as
        // its original sense.
        _out->map_f_end->st_plt_value = 0;
        _out->map_f_end++;
      } else {

        // this is the condition where this entry is @plt.
        _out->map_plt_end--;
        memcpy(_out->map_plt_end, p, sizeof(Elf64_Sym));
        _out->map_f_end->st_plt_value = 0;
      }
    }
  }
}

static inline size_t *get_plt_from_got(const size_t _x) {
  // Access plt.
  // the reason of (& 0x0000ffff) is removing virtual address which is
  // included on st_value on .rela.plt.
  // -6 means that initial got attributes one instruction(6bytes) behind the
  // head of @plt which needs to be subtracted to reach the head of it.
  return (size_t *)(*(size_t *)(_x - 0x200000) - 6);
}

void fill_plt_offset(const info_on_elf const *_e,
                     const info_on_workspace const *_w) {

  // todo.
  // 1. acquire got address from .rela.plt section
  // 2. get plt address from the value put on got address.
  // -> note that got address initially has return address to the next address
  // of callee plt.
  // 3. get .dynstr section index from leftest 4byte of info value on .rela.plt
  // section
  // 4. get name of the function on plt with information on .dynstr
  // 5. feed plt of extracted symbol table

  Elf64_Rela *rela_p;
  const Elf64_Rela *rela_plt_end =
      (Elf64_Rela *)((size_t)_e->rela_plt_p + _e->rela_plt_size);
  const Elf64_Rela *rela_dyn_end =
      (Elf64_Rela *)((size_t)_e->rela_dyn_p + _e->rela_dyn_size);
  const Elf64_Sym *dp = _e->dyn_sym_p;
  workspace_sym *sym_p;
  const Elf64_Sym *plt_p_begin = _w->map_plt_begin - 1;
  const Elf64_Sym *plt_p_end = _w->map_plt_end;
  const Elf64_Sym *p_begin = _w->map_f_begin;
  const Elf64_Sym *p_end = _w->map_f_end;
  const char *str_p = _e->str_p == 0 ? _e->dyn_str_p : _e->str_p;
  char num;
  char *name;
  size_t *plt;

  // iterate .rela.plt
  for (rela_p = _e->rela_plt_p; rela_p != rela_plt_end; rela_p++) {
    // get plt from got
    plt = get_plt_from_got((size_t)_e->ehdr_p + rela_p->r_offset);
    // get name
    name = &_e->dyn_str_p[(dp + ELF64_R_SYM(rela_p->r_info))->st_name];

    // iterate workspace of plt table.
    for (sym_p = plt_p_begin; sym_p != plt_p_end; sym_p--) {
      if (!strcmp(&str_p[sym_p->st_name], name)) {
        sym_p->st_plt_value = (Elf64_Word)plt;
        break;
      }
    }

    // iterate workspace of non-plt function table.
    // if this file is shared object,
    // function call on a same shared object is converted to calling via plt-got
    // scheme.
    for (sym_p = p_begin; sym_p != p_end; sym_p++) {
      if (!strcmp(&str_p[sym_p->st_name], name)) {
        sym_p->st_plt_value = (Elf64_Word)plt;
        break;
      }
    }
  }

  const plt_got *plt_got_begin = _e->plt_got_p;
  const plt_got *plt_got_end =
      (plt_got *)((size_t)_e->plt_got_p + _e->plt_got_size);
  plt_got *plt_got_p;
  // iterate .rela.dyn_str_p
  for (rela_p = _e->rela_dyn_p; rela_p != rela_dyn_end; rela_p++) {
    size_t got_p = (size_t)_e->ehdr_p + rela_p->r_offset;
    for (plt_got_p = plt_got_begin; plt_got_p != plt_got_end; plt_got_p++) {
      if (got_p ==
          (size_t)plt_got_p->rel_offset + (size_t)(&plt_got_p->_6690)) {
        name = &_e->dyn_str_p[(dp + ELF64_R_SYM(rela_p->r_info))->st_name];
        plt = plt_got_p;
        // iterate workspace of plt table.
        for (sym_p = plt_p_begin; sym_p != plt_p_end; sym_p--) {
          if (!strcmp(&str_p[sym_p->st_name], name)) {
            sym_p->st_plt_value = (Elf64_Word)plt;
            break;
          }
        }
        for (sym_p = p_begin; sym_p != p_end; sym_p++) {
          if (!strcmp(&str_p[sym_p->st_name], name)) {
            sym_p->st_plt_value = (Elf64_Word)plt;
            break;
          }
        }
      }
    }
    // if ( got address == .plt.got->rel_offset + %rip)
    // (size_t)_e->ehdr_p + rela_p->r_offset)
  }
};

const heap *map_call_table(const void *const addr) {

  const size_t map_size = PAGE_SIZE;
  void *begin = mmap(addr, map_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  if (begin == MAP_FAILED) {
    printf("err\n");
  }
  heap *h = (heap *)((heap *)HEAP_HEADER_ADDR_P - COUNT);
  if (!COUNT) {
    h->begin = begin;
    h->page_num = map_size / PAGE_SIZE;
    h->flags = 1;
    h->file = 0;
  } else {
    h->page_num += map_size / PAGE_SIZE;
  }
  COUNT++;
  HEAP_HEADER_ADDR_P += sizeof(heap);
  return h;
};

const Elf64_Word iterate_f_on_symt(const info_on_workspace *const _in,
                                   const size_t addr) {

  workspace_sym *cp = _in->map_f_begin;
  // iterate f table
  for (; cp != _in->map_f_end; cp++) {
    if (cp->st_value == addr || cp->st_plt_value == (Elf64_Word)addr) {
      return cp->st_name;
    }
  }
  // iterate plt table
  cp = _in->map_plt_begin - 1;
  for (; cp != _in->map_plt_end; cp--) {
    if (cp->st_value == addr || cp->st_plt_value == (Elf64_Word)addr)
      return cp->st_name;
  }
  return -1;
}

uint32_t *parse_binary(const info_on_workspace *const _in, const uint8_t *ip,
                       const uint8_t *const end, uint32_t *out_p,
                       const size_t map_end) {

  int count;
  uint32_t *len_p = out_p - 1;
  Elf64_Word caller_name;
  int offset_p;
  for (count = 0; ip < end;) {
    if (*ip == X86_NEAR_REL_CALL_OPCODE) {
      ip += 1;
      offset_p = *(size_t *)ip;
      ip += X86_NEAR_REL_CALL_OPERAND_N;
      caller_name = iterate_f_on_symt(_in, (size_t)ip + offset_p);
      if (caller_name != -1) {
        count++;
        if ((size_t)out_p == map_end) {
          map_call_table((void *)out_p);
        }
        *out_p = caller_name;
        out_p++;
      } else {
        ip -= X86_NEAR_REL_CALL_OPERAND_N;
      }
    } else {
      ip++;
    }
  }
  out_p += -count & 0x03;
  if (((size_t)out_p) % 16 != 0) {
    printf("error!!%d\n", out_p);
  }
  *len_p = count;
  return out_p;
}

const heap *iterate_on_func(const info_on_workspace *const _in, const heap *h) {

  call_table *out_p = (call_table *)h->begin;
  size_t map_end = (size_t)h->begin + h->page_num * PAGE_SIZE;
  workspace_sym *cp = _in->map_f_begin;
  char i;
  uint8_t *end_p;
  // iterate function not on plt.
  for (i = 0; cp != _in->map_f_end; cp++, i++) {
    if ((size_t)out_p == map_end) {
      map_call_table((void *)out_p);
    }
    if (cp->st_value != 0x00) {
      out_p->addr = cp->st_value;
      out_p->st_name = cp->st_name;
      out_p++;
      end_p = (char *)((size_t)cp->st_value + (size_t)cp->st_size);
      out_p = parse_binary(_in, (uint8_t *)cp->st_value, end_p,
                           (uint32_t *)out_p, map_end);
    } else {
      printf("error\n");
      // error.
    }
  }
  return h;
}

const char *gen_title(char *p) {
  const char *prefix = "digraph g ";
  p = stpcpy(p, prefix);
  *p = 0x7b;
  p++;
  *p = 0x0a;
  p++;
  return p;
}

const heap *call_graph_file_map(int fd, void *map_begin) {
  const char c = 0x20;
  const size_t offset = COUNT * PAGE_SIZE;
  lseek(fd, offset + PAGE_SIZE, SEEK_SET);
  write(fd, &c, sizeof(char));
  lseek(fd, offset, SEEK_SET);
  void *begin = (void *)mmap(map_begin, PAGE_SIZE, PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_FIXED, fd, offset);
  if (begin == MAP_FAILED) {
    printf("error\n");
    exit(0);
  }
  heap *h = (heap *)HEAP_HEADER_ADDR_P - COUNT;
  if (!COUNT) {
    h->begin = begin;
    h->page_num = 1;
    h->flags = 1; // not yet implemented..
    h->file = (uint16_t)fd;
  } else {
    h->page_num += 1;
  }
  COUNT++;
  HEAP_HEADER_ADDR_P += sizeof(heap);
  return h;
}

char *emit_call_graph(const call_table *cp, const call_table *const end,
                      const info_on_elf *const _e, const heap *const h) {

  char *out_p = (char *)h->begin;
  size_t map_end = (size_t)h->begin + h->page_num * PAGE_SIZE;
  out_p = gen_title(out_p);
  char *callee_name;
  uint32_t *elem;
  int len;
  const char *str_p = _e->str_p == 0 ? _e->dyn_str_p : _e->str_p;
  for (; cp != end; cp++) {
    len = cp->length;
    if (((size_t)out_p) + len * 30 > map_end) {
      call_graph_file_map((int)h->file, (void *)map_end);
      map_end += PAGE_SIZE;
    }
    if (len != 0) {
      callee_name = &str_p[cp->st_name];
      elem = (uint32_t *)(cp + 1);
      cp += ((len + 3) / 4);
      for (; len != 0; elem++, len--) {
        out_p = stpcpy(out_p, callee_name);
        *out_p = 0x2d;
        out_p++;
        *out_p = 0x3e;
        out_p++;
        out_p = stpcpy(out_p, &str_p[*elem]);
        *out_p = 0x0a;
        out_p++;
      }
    }
  }
  *out_p = 0x7d;
  out_p++;
  *out_p = 0x0a;
  out_p++;
  return out_p;
}

/* shdr.sh_name = 0; // needs to be changed later on */
/* shdr.sh_type  SHT_SHLIB   */
/* sh_flags -> 0 */
/* sh_addr -> 0 */
/* sh_offset -> index of inserted.. */
/* sh_size -> size */
/* sh_link -> 0 */
/* sh_info -> 0 */
/* sh_addralign -> 0 */
/* sh_entsize -> entry size must be preserved */

// first 8byte is virtual address to indicate the mark where
// you temporarily suspend memmove and switch to memmove from 2nd address.
// third address is the size of byte which needs to be copied.

void *conditional_memmove(char *p, char **pp) {
  char *begin = pp;
  for (;; p--) {
    for (; *pp != 0; pp += 3) {
      if (p == pp) {
        p = mempcpy(p, pp + 1, pp + 2);
      }
    }
    begin = pp;
  }
  return p;
};

// this function will insert 4 new data on existing mapping from a executble
// file. 4 elements which will be added is as follows.

// 1. An entry of section header of .call (64byte)
// 2. name of section header string table (6byte)
// 3. name of symbol table (16byte)
// 4. contents of the section (depends on how many funcitons are registered)

void convert_mmapped_area(info_on_elf *_e) {

  // entry of section header table
  Elf64_Shdr shdr;
  // symbol table entry
  Elf64_Sym sym;
  // section header string table entry
  const char name = ".call";
  // assuming we are in 64byte world,
  char **aa;
  // beginning of entry of symbol table
  *aa = _e->symbol_p;
  aa++;
  *aa = &shdr;
  aa++;
  *aa = sizeof(shdr);
  aa++;
  // beginning of sh_str entry
  *aa = _e->shdr_head;
  aa++;
  *aa = &name;
  aa++;
  *aa = sizeof(sym);
  aa++;
  // beginning of entry of symbol table
  *aa = _e->symbol_p;
  aa++;
  *aa = &sym;
  aa++;
  *aa = sizeof(sym);
  aa++;
  // beginning of offset of symbol table
  *aa = _e->symbol_p;
  aa++;
  // *aa = call_table_begin;
  *aa = 6;
  aa++;
  // 0 indicates end.
  *aa = 0;
  int call_table_len = 1;
  // sizeof(Elf64_Shdr) * sizeof(sym) + strlen(.call) + call_table_len
  int added = sizeof(Elf64_Shdr) + sizeof(sym) + strlen(&name) + call_table_len;
  // we are going to start from the end of the file + expected added size to the
  // file.
  conditional_memmove((char *)((size_t)_e->shdr_tail + added), aa);
}

int main(int argc, char **argv) {

  // exit(0);
  char attach = 0;
  char generate = 0;
  if (argc < 2) {
    printf("An argument needs to be provided.\n");
    exit(0);
  }
  if (!strcmp(argv[1], "-ag")) {
    attach = generate = 1;
  } else if (!strcmp(argv[1], "-a")) {
    // attach call graph to newly generated elf file
    attach = 1;
  } else if (!strcmp(argv[1], "-g")) {
    generate = 1;
  } else {
    printf("First argument needs to be -a(attach),-g(generate), -ag(both of "
           "them).\n");
    exit(0);
  }
  const char *fname = argv[2];
  size_t last_map_offset;
  const void *elf_p = 0x0;
  char *call_graph_f_map;

  // first of all, a given object format file needs to be mapped on a memory.
  const heap *h1 = map_elf(elf_p, fname);

  // second, you will retrieve necessary inforamtion from its section header
  // table.
  const info_on_elf _info_on_elf;
  retrieve_info_from_sht(h1->begin, &_info_on_elf);

  // third, you just need to map(prepare) workspace to iterate smaller symbol
  // table to reduce computational cost on later stage.

  last_map_offset = (size_t)h1->begin + h1->page_num * PAGE_SIZE;
  const heap *h2 =
      map_smaller_symbol_table((void *)last_map_offset, &_info_on_elf);
  // fourth, you will extract the entries which is needed, and mapped to the
  // previously-prepared workspace mapping.
  info_on_workspace _info_on_workspace;
  _info_on_workspace.map_f_begin = (Elf64_Sym *)last_map_offset;
  _info_on_workspace.map_f_end = (Elf64_Sym *)last_map_offset;
  _info_on_workspace.map_plt_begin =
      (Elf64_Sym *)((size_t)last_map_offset + h2->page_num * PAGE_SIZE);
  _info_on_workspace.map_plt_end =
      (Elf64_Sym *)((size_t)last_map_offset + h2->page_num * PAGE_SIZE);
  last_map_offset += h2->page_num * PAGE_SIZE;

  retrieve_info_from_symt(&_info_on_elf, &_info_on_workspace);

  // 5th, procedure linkage table needs to be filled
  fill_plt_offset(&_info_on_elf, &_info_on_workspace);

  // 6th, outcome is pasted here which i call call table.
  heap *h3 = (call_table *)map_call_table((void *)last_map_offset);
  // 7th (and this is main process), iterate workspace and generate
  // a call graph dependencies table looking up .text area of each functions.
  h3 = iterate_on_func(&_info_on_workspace, h3);
  last_map_offset += h3->page_num * PAGE_SIZE;

  // attach call graph table on a given elf format
  if (attach) {

    printf("attach mode is not yet implemented..\n");
    // convert_mmapped_area(elf_p , _info_on_elf, call_table_begin,
    // call_table_end);
    if (msync(h1->begin, h1->page_num, MS_SYNC) != 0) {
      printf("error!\n");
      exit(0);
    }
  }

  // emit_call_graph
  if (generate) {
    // reset global counter
    COUNT = 0;
    const char *fname = "graph.dot";
    int fd = open(fname, O_RDWR | O_CREAT | O_TRUNC);
    if (fd == -1) {
      printf("error");
      exit(0);
    }
    heap *h4;
    h4 = call_graph_file_map(fd, (void *)last_map_offset);
    emit_call_graph((call_table *)h3->begin,
                    (call_table *)(h3->begin + h3->page_num * PAGE_SIZE),
                    &_info_on_elf, h4);
    if (msync((char *)h4->begin, h4->page_num * PAGE_SIZE, 0) != 0) {
      printf("error\n");
      exit(0);
    }
  }
  printf("graph.dot is saved on current dir.\n");
  exit(1);
};
