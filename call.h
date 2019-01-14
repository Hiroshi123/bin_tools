

#define X86_NEAR_REL_CALL_OPCODE 0xe8
#define X86_NEAR_REL_CALL_OPERAND_N 4

static void *HEAP_HEADER_ADDR_HEAD;
static void *HEAP_HEADER_ADDR_P;
static void *HEAP_HEADER_ADDR_TAIL;
static size_t PAGE_SIZE;

static char COUNT = 0;

typedef struct __attribute__((__packed__)) {
  // first argument of mmap
  void *begin;
  // second argument of mmap
  // size_t size;
  uint16_t page_num;
  // 3rd & 4th argument of mmap
  uint32_t flags; // prot & map flags
  // 5th & 6th arugment
  uint16_t file;
  // subsequent mapping
  // struct __heap* next;
} heap;

// tyWRITWRITbpedef struct __heap heap;

typedef struct __attribute__((__packed__)) {
  uint16_t _ff25;
  uint32_t rel_offset;
  uint16_t _6690;
} plt_got;

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
  // size of .rela.plt or .rela.dyn
  unsigned int rela_plt_size;
  // pointer to the head of .rela.dyn section
  Elf64_Rela *rela_dyn_p;
  // size of .rela.plt or .rela.dyn
  unsigned int rela_dyn_size;

  // .plt.got (in case, got address has 0)
  plt_got *plt_got_p;
  Elf64_Xword plt_got_size;

  // pointer to the head of .dynsym section
  Elf64_Sym *dyn_sym_p;
  // size of .dynsym
  unsigned int dyn_sym_size;

  // symbol table_offset

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

// workspace_sym is a bit of modification from Elf64_sym.
// On 24byte, first 8byte is
typedef struct __attribute__((__packed__)) {
  Elf64_Word st_name;
  Elf64_Word st_plt_value;
  Elf64_Addr st_value;
  Elf64_Xword st_size;
} workspace_sym;

typedef struct {
  workspace_sym *map_f_begin;
  workspace_sym *map_f_end;
  // assumes, map_f_end is smaller than map_plt_end
  workspace_sym *map_plt_end;
  workspace_sym *map_plt_begin;
} info_on_workspace;

typedef struct {
  Elf64_Addr addr;
  Elf64_Word st_name;
  Elf64_Word length;
} call_table;
