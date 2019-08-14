


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

// definition of general symbol table entry
// each object format (elf, macho-o, coff)
// holds its own symbol table.
// To generalize them and let it smaller for numebrs of iterations
// to detect the possible addressing from cancdidate "e8" statement,
// it is extracted.
// Note that general symbol table can be mapped on multiple pages but needs to be
// contiguous in between.

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

// you need to prepare at least 2 general symbol table.
// One is for ordinal functions and its implementation is contained on the object file.
// Another is for dynamic linked functions where its implementation is not defined on the
// object file. On Windows, it exists as image extry import directory, elf it is on
// a global offset table.

typedef struct {
  gen_symt *map_f_begin;
  gen_symt *map_f_end;
  gen_symt *map_dlf_begin;
  gen_symt *map_dlf_end;
} function_map;

// call table is 16byte struct which describes following 3.
// 1. beginning offset address
// 2. function name
// 3. length of functions which was called from this function
// Note that call table can be mapped on multiple pages but needs to be
// contiguous in between.

// Note that list of caller function's name is followed after
// the call table.
// These are allocated at every 16byte unit.
// For instance, if a function calles just another function,
// the list consisits of 16byte nevertheless a function consumes
// just 4byte. In this case, rest of byte are padded as 0 to indicate
// they are empty.

typedef struct {
  uint64_t addr;
  uint32_t st_name;
  uint32_t length;
} call_table;

typedef struct {
  call_table* map_begin;
  call_table* map_end;
} out_map;

