


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


// definition of general symbol table entry
// each object format (elf, macho-o, coff)
// holds its own symbol table.
// To generalize them and let it smaller for numebrs of iterations
// to detect the possible addressing from cancdidate "e8" statement,
// it is extracted.
typedef struct gen_symtable {
  // addr means the head address of symbol table.
  // 32bit machine address is holded on this mmember for
  // brevity
  uint64_t st_value;
  // size is the number of byte the binary that the function holds.
  // these contains nop statemenet after returning.
  // Note, mach-o and coff does not have the sihze of function in default that
  // the elf allocates for.
  // The value needs to be computed
  // (sorting by the address, and grab the offset of the next function, if not the beggining of the next section)    
  uint32_t st_size;
  // name is the offset to the string name of that funciton from head
  // address of the mapped object format.
  // Note this is not the pointer to the address of the name.
  // this requires extra calculation to reach the address of name of the funciton,
  // but can be within 4byte.
  // TODO. if the given object format is beyond the range of addressing space which
  // which waa computed as?? 
  uint32_t st_name; 
} gen_symt;

typedef struct {
  gen_symt *map_f_begin;
  gen_symt *map_f_end;
  gen_symt *map_iat_begin;
  gen_symt *map_iat_end;
} function_map;

// call table begins

typedef struct {
  uint64_t addr;
  uint32_t st_name;
  uint32_t length;
} call_table;

