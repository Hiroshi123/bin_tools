
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define X86_NEAR_REL_CALL_OPCODE 0xe8
#define X86_NEAR_REL_CALL_OPERAND_N 4

/*
This piece of command will generate call graph anaöizing a binary file.
The call graph can be generated as .dot format which can be converted to
any image format.
The function call dependencies can also be set as a section header.
*/

typedef struct {

  // info on elf header itself
  Elf64_Ehdr *ehdr_p;
  Elf64_Phdr *phdr_p;
  Elf64_Shdr *shdr_head;
  Elf64_Shdr *shdr_tail;
  char *shstr_offset;

  // pointer to the header of .text section
  char *text_p;
  // section header index of .text section
  char text_shndx;
  // pointer to the head of .rela.plt section
  Elf64_Rela *rela_plt_p;
  // size of .rela.plt
  unsigned int rela_plt_size;

  // pointer to the head of .dynsym section
  Elf64_Sym *dyn_sym_p;
  // size of .dynsym
  unsigned int dyn_sym_size;

  // pointer to the head of symbol table
  Elf64_Sym *symbol_p;
  // siye of symbol table
  unsigned int symbol_size;

  // pointer to the head of .dynstr section
  char *dyn_str_p;

  // pointer to the head of string table
  char *str_p;
} info_on_elf;

// workspace is the part of symbol table entry extracted from original symbol
// table. normal function is mapped on the head of page and comes up towards
// positive direciton. plt function coversely start the tail of mmaped page and
// step down towards negative direciton,

struct info_on_workspace {
  Elf64_Sym *map_f_begin;
  Elf64_Sym *map_f_end;
  // assumes, map_f_end is smaller than map_plt_end
  Elf64_Sym *map_plt_end;
  Elf64_Sym *map_plt_begin;
};

typedef struct {
  Elf64_Addr addr;
  Elf64_Word st_name;
  Elf64_Word length;
} call_table;

char check_elf(uint8_t *e) {
  return (e[EI_MAG0] == ELFMAG0 && e[EI_MAG1] == ELFMAG1 &&
          e[EI_MAG2] == ELFMAG2 && e[EI_MAG3] == ELFMAG3);
}

// original elf file is going to be mapped on the beginning on virtual memory.

int map_elf(void **page_for_elf, const char *fname, size_t *map_size) {
  int fd = open(fname, O_RDWR);
  if (fd == -1) {
    exit(0);
  }
  struct stat stbuf;
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    exit(0);
  }
  *map_size = (stbuf.st_size + 0x1000) & 0xf000;
  *page_for_elf = mmap((void *)0, *map_size, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_FIXED, fd, 0);
  if (*page_for_elf == MAP_FAILED) {
    close(fd);
    exit(0);
  }
  if (!check_elf((uint8_t *)*page_for_elf)) {
    close(fd);
    exit(0);
  }
  return fd;
}

void retrieve_info_from_sht(char *page_for_elf, info_on_elf *_e) {
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
  for (shdr = _e->shdr_head; shdr != _e->shdr_tail; shdr++, i++) {
    sh_name = &_e->shstr_offset[shdr->sh_name];
    if (shdr->sh_type == SHT_SYMTAB) {
      _e->symbol_p = (Elf64_Sym *)shdr->sh_offset;
      _e->symbol_size = shdr->sh_size;
      // error....
      // If there is no symbol table, then you must generate an error.
      continue;
    } else if (shdr->sh_type == SHT_DYNSYM) {
      _e->dyn_sym_p = (Elf64_Sym *)shdr->sh_offset;
      _e->dyn_sym_size = shdr->sh_size;
      continue;
    } else if (shdr->sh_type == SHT_STRTAB) {
      if (!strcmp(sh_name, ".dynstr")) {
        _e->dyn_str_p = (char *)shdr->sh_offset;
      } else if (!strcmp(sh_name, ".strtab")) {
        _e->str_p = (char *)shdr->sh_offset;
      }
      continue;
    } else if (shdr->sh_type == SHT_RELA) {
      if (!strcmp(sh_name, ".rela.plt")) {
        _e->rela_plt_p = (Elf64_Rela *)shdr->sh_offset;
        _e->rela_plt_size = shdr->sh_size;
      }
      continue;
    } else if (shdr->sh_type == SHT_PROGBITS) {
      if (!strcmp(sh_name, ".text")) {
        _e->text_shndx = (char)i;
      }
    }
  }
}

void *map_smaller_symbol_table(void *map_begin, size_t map_size) {
  void *page_for_smt = mmap(map_begin, map_size, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  if (page_for_smt == MAP_FAILED) {
    printf("error!\n");
  }
  return page_for_smt;
}

void retrieve_info_from_symt(info_on_elf *_in, struct info_on_workspace *_out) {

  // iteration inside symbol table
  /* Elf64_Sym* symbol_begin; */
  Elf64_Sym *symbol_end =
      (Elf64_Sym *)((size_t)_in->symbol_p + (size_t)_in->symbol_size);
  for (; _in->symbol_p != symbol_end; _in->symbol_p++) {
    if (ELF64_ST_BIND(_in->symbol_p->st_info) == STB_GLOBAL &&
        ELF64_ST_TYPE(_in->symbol_p->st_info) == STT_FUNC) {
      if (_in->symbol_p->st_shndx == _in->text_shndx) {
        // this is the condition where this entry is normal func on .text
        // section.

        // memcpy from original page to workspace page.
        memcpy(_out->map_f_end, _in->symbol_p, sizeof(Elf64_Sym));
        _out->map_f_end++;
      } else {
        // this is the condition where this entry is @plt.
        memcpy(_out->map_plt_end - sizeof(Elf64_Sym), _in->symbol_p,
               sizeof(Elf64_Sym));
        _out->map_plt_end--;
      }
    }
  }
}

void fill_plt_offset(info_on_elf *_in, struct info_on_workspace *_out){

    // todo.
    // 1. acquire got address from .rela.plt section
    // 2. get plt address from the value put on got address.
    // -> note that got address initially has return address to the next address
    // of callee plt.
    // 3. get .dynstr section index from leftest 4byte of info value on
    // .rela.plt section
    // 4. get name of the function on plt with information on .dynstr
    // 5. feed plt of extracted symbol table

};

void *map_call_table(void *last_map_offset, size_t map_size) {
  void *page_for_out = mmap(last_map_offset, map_size, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  if (page_for_out == MAP_FAILED) {
    printf("error\n");
    exit(0);
  }
  return page_for_out;
};

Elf64_Word iterate_f_on_symt(struct info_on_workspace *_in, size_t addr) {

  Elf64_Sym *cp = _in->map_f_begin;
  char i;
  // iterate f table
  for (i = 0; cp != _in->map_f_end; cp++) {
    if (cp->st_value == addr) {
      return cp->st_name;
    }
    i++;
  }
  // iterate plt table
  /* cp = _in->map_plt_begin; */
  /* for (;cp!=_in->map_plt_end;cp++) { */
  /*   printf("zzz!\n"); */
  /*   if (cp->st_value == addr) { */
  /*     return i; */
  /*   } */
  /*   i++; */
  /* } */
  return -1;
}

uint32_t *parse_binary(struct info_on_workspace *_in, uint8_t *ip, uint8_t *end,
                       uint32_t *out_p) {

  int count;
  uint32_t *len_p = out_p - 1;
  Elf64_Word caller_name;
  int offset_p;
  for (count = 0; ip != end;) {
    if (*ip == X86_NEAR_REL_CALL_OPCODE) {
      ip += 1;
      offset_p = *(size_t *)ip;
      ip += X86_NEAR_REL_CALL_OPERAND_N;
      caller_name = iterate_f_on_symt(_in, (size_t)ip + offset_p);
      if (caller_name != -1) {
        count++;
        *out_p = caller_name;
        out_p++;
      } else {
        ip -= 4;
      }
    } else {
      ip++;
    }
  }
  out_p += -count & 0x03;
  /* if (((size_t)out_p) % 16 != 0) { */
  /*   printf("error!!%d\n", out_p); */
  /* } */
  *len_p = count;
  return out_p;
}

call_table *iterate_on_func(struct info_on_workspace *_in, call_table *out_p) {
  Elf64_Sym *cp = _in->map_f_begin;
  char i;
  uint8_t *end_p;
  // iterate function not on plt.
  for (i = 0; cp != _in->map_f_end; cp++, i++) {
    if (cp->st_value != 0x00) {
      out_p->addr = cp->st_value;
      out_p->st_name = cp->st_name;
      out_p++;
      end_p = (char *)((size_t)cp->st_value + (size_t)cp->st_size);
      out_p =
          parse_binary(_in, (uint8_t *)cp->st_value, end_p, (uint32_t *)out_p);
    } else {
      printf("error\n");
      // error.
    }
  }
  return out_p;
}

char *gen_title(char *p) {
  const char *prefix = "digraph g ";
  p = stpcpy(p, prefix);
  *p = 0x7b;
  p++;
  *p = 0x0a;
  p++;
  return p;
}

char *emit_call_graph(call_table *cp, call_table *end, info_on_elf *_e,
                      char *out_p) {

  char *begin = out_p;
  out_p = gen_title(out_p);

  for (; cp != end; cp++) {
    int len = cp->length;
    if (len != 0) {
      char *callee_name = &_e->str_p[cp->st_name];
      cp++;
      uint32_t *elem = (uint32_t *)cp;
      for (; len != 0; elem++, len--) {
        out_p = stpcpy(out_p, callee_name);
        *out_p = 0x2d;
        out_p++;
        *out_p = 0x3e;
        out_p++;
        out_p = stpcpy(out_p, &_e->str_p[*elem]);
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

char *call_graph_file_map(void *map_begin, size_t map_size, const char *fname) {

  int fd = open(fname, O_RDWR | O_CREAT | O_TRUNC);
  if (fd == -1) {
    printf("error");
  }
  char c = 0x20;
  lseek(fd, map_size, SEEK_SET);
  write(fd, &c, sizeof(char));
  lseek(fd, 0, SEEK_SET);
  char *r = (char *)mmap(map_begin, map_size, PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_FIXED, fd, 0);
  if (r == MAP_FAILED) {
    printf("error\n");
    exit(0);
  }
  return r;
}

int main(int argc, char **argv) {

  size_t PAGE_SIZE = getpagesize();
  char attach = 0;
  char generate = 0;
  if (argc < 2) {
    printf("An argument needs to be provided.\n");
    exit(0);
  }
  if (!strcmp(argv[1], "-ag")) {
    attach = generate = 1;
  } else if (!strcmp(argv[1], "-a")) {
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
  size_t file_size;
  size_t map_size = PAGE_SIZE;
  size_t output_map_size = PAGE_SIZE;

  void *elf_p;
  void *symt_p;
  call_table *call_table_begin;
  char *call_graph_f_map;

  // first of all, a given object format file needs to be mapped on a memory.
  int fd = map_elf(&elf_p, fname, &file_size);

  // second, you will retrieve necessary inforamtion from its section header
  // table.
  info_on_elf _info_on_elf;
  retrieve_info_from_sht(elf_p, &_info_on_elf);

  // third, you just need to map(prepare) workspace to iterate smaller symbol
  // table to reduce computational cost on later stage.

  /* last_map_offset = ((size_t)file_size + 0x1000) & 0x000; */
  last_map_offset = ((size_t)file_size & 0x000)
                        ? ((size_t)file_size + PAGE_SIZE) & 0xf000
                        : file_size;
  symt_p = map_smaller_symbol_table((void *)last_map_offset, map_size);
  last_map_offset = ((size_t)symt_p + PAGE_SIZE) & 0xf000;
  // fourth, you will extract the entries which is needed, and mapped to the
  // previously-prepared workspace mapping.
  struct info_on_workspace _info_on_workspace;
  _info_on_workspace.map_f_begin = (Elf64_Sym *)symt_p;
  _info_on_workspace.map_f_end = (Elf64_Sym *)symt_p;
  _info_on_workspace.map_plt_begin = (Elf64_Sym *)((size_t)symt_p + map_size);
  _info_on_workspace.map_plt_end = (Elf64_Sym *)((size_t)symt_p + map_size);

  retrieve_info_from_symt(&_info_on_elf, &_info_on_workspace);

  // 5th, procedure linkage table needs to be filled
  fill_plt_offset(&_info_on_elf, &_info_on_workspace);

  // 6th, outcome is pasted here which i call call table.
  call_table_begin =
      (call_table *)map_call_table((void *)last_map_offset, output_map_size);
  last_map_offset = ((size_t)call_table_begin + PAGE_SIZE) & 0xf000;

  // 7th (and this is main process), iterate workspace and generate
  // a call graph dependencies table looking up .text area of each functions.
  call_table *call_table_end =
      iterate_on_func(&_info_on_workspace, (call_table *)call_table_begin);

  // attach call graph table on a given elf format
  if (attach) {
    // TODO.
    // conversion is not yet implemented.
    // need to make sure where the new section should be inserted.
    if (msync(elf_p, file_size, MS_SYNC) != 0) {
      printf("error!\n");
      exit(0);
    }
  }

  // emit_call_graph
  if (generate) {
    // char* a = strchr(argv[2];
    const char *fname = "graph.dot";
    call_graph_f_map =
        call_graph_file_map((void *)last_map_offset, map_size, fname);
    emit_call_graph(call_table_begin, call_table_end, &_info_on_elf,
                    call_graph_f_map);
    if (msync(call_graph_f_map, PAGE_SIZE, 0) != 0) {
      printf("error\n");
      exit(0);
    }
  }

  munmap(elf_p, file_size);
  munmap(symt_p, map_size);
  munmap(call_table_begin, map_size);
  if (generate)
    munmap(call_graph_f_map, map_size);
  close(fd);

  exit(1);
};
