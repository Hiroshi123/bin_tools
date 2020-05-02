
#include <stdint.h>

/* #define DEBUG 1 */
#define SECTION_ALIGNMENT 0x1000
#define FILE_ALIGNMENT 0x200
#define EMIT_DLL 1
#define EMIT_OBJ 2
#define EMIT_EXE 3

// #define ET_EXEC	2
// #define ET_DYN	3

#define GET_NAME(X,Y) (*(uint32_t*)X == 0) ? (char*) ((size_t)Y+*((uint32_t*)X+1)) : X->N.ShortName

#define UPDATE_STRTABLE_OFFSET(X,Y) if (*(uint32_t*)X == 0) *((uint32_t*)X+1) += Y

// (*(uint32_t*)X == 0) ? *(uint32_t*) ((size_t)Y+*((uint32_t*)X+1)) : X->N.ShortName;

typedef struct _SectionChain SectionChain;
typedef struct _ObjectChain ObjectChain;
typedef struct _SymbolChain SymbolChain;
typedef struct _SectionContainer SectionContainer;
typedef struct _SymbolChain3 SymbolChain3;

struct _SymbolChain {
  SymbolChain* next;
  /*IMAGE_SYMBOL*/char* p;
  SectionChain* schain;
  // this is the pointer to string itself
  char* name;
};

typedef struct SymbolHashTable {
  uint32_t nbucket;
  uint32_t nchain;
  size_t* bucket;
  SymbolChain* symbol_chain;
};

// this section list will be updated each time new object file is provided and
// concluded that it is not merged.
// 1. SectionChain<Container> which will contain list of section which will be merged eventually.
//  -> should have next(a pointer to a different section)
//  -> should have this(a pointer to a section which is contained in a same section)

// 2. SectionChain<Section> which represent each section and pointer to data.

struct _SectionContainer {
  SectionContainer* next;
  SectionChain* init;
  SectionChain* this;
  // this is a pointer on either ImageSection or ELF_Shdr
  uint32_t size;
  uint32_t virtual_address;
  void* name;
  void* candidate_list;
  /* ObjectChain* obj; */
  /* SymbolChain* sym_head; */
  /* SymbolChain* sym_tail; */
};

struct _SectionChain {
  // section_num
  void* data;
  SectionChain* next;
  SectionChain* this;
  union {
    size_t num;
    void*
    /*IMAGE_SECTION_HEADER*/ p;
  };
  // pointer to object
  ObjectChain* obj;
  SymbolChain* sym_head;
  SymbolChain* sym_tail;
  uint32_t virtual_address;  
};

// the role of object chain is to help symbol hash table to look-up
// function.
// each hash table contains a poiter to a record of symbol table.
// however, a record of symbol does not know where the actual string value is stored
// since symbol table and string table is seperated managed.
// To glue them, look-up function should be helped by a linked list named ObjectChain.
// After look-up function compute a hash of a given name of string, the entry which are accessed
// on a table needs to be validated if that is matched with the given string, otherwise trace
// the tagged chain.
// After finding first entry of symbol-table, look-up function will tag symbol-table and
// string table by traversing object-chain where each of them tells the range of symbol table entry
// and tagged pointer to symbol table.
struct _ObjectChain {
  ObjectChain* next;
  uint32_t symbol_num;
  uint32_t export_symbol_num;
  // this is not for strcmp() but get all of entry under this object chain.
  // it will point entry if the bit is set, then the entry is stored on the record.
  // this is used for constructing export table virtual address.
  // Note you do not need to compare anything when you wanna just get a list not for resolution.
  SymbolChain* symbol_chain_head;
  SymbolChain* symbol_chain_tail;
  SectionChain* section_chain_head;
  SectionChain* section_chain_tail;
  // size_t otherwise
  char*/*IMAGE_SYMBOL*/ symbol_table_p;
  uint8_t* str_table_p;
  // tables which put pointer to relocation section onto(only used for elf)
  void* reloc_section_head;
  void* reloc_section_tail;
  void* section_head;
};

typedef struct __attribute__((__packed__)) _CoffReloc {
  uint32_t VirtualAddress;
  uint32_t SymbolTableIndex;
  uint16_t Type;
} CoffReloc;

typedef struct __attribute__((__packed__)) _CallbackArgIn {
  union {
    size_t* virtual_address;
    size_t* section_name;
  };
  char* name;
  size_t* type;
  uint32_t storage_class;
} CallbackArgIn;


struct _SymbolChain3 {
  SymbolChain3* next;
  SymbolChain3* this;
  char* name;
  uint32_t ever;
};

/* typedef struct __attribute__((packed)) _PltCode { */
/*   uint16_t code __attribute__((packed)); */
/*   uint32_t data; */
/* } PltCode __attribute__((packed)); */

typedef struct {
    void* init;
    void* current;
} ListContainer;

typedef struct /*dt_hash_table */{
  uint32_t nbucket;
  uint32_t nchain;
  // it depends on above 2.
  // uint32_t bucket[0];
  // uint32_t chain[0];
} DtHashTable;

// Input configuration setting comes here.
typedef struct {
  int base_address;
  int out_size;
  uint8_t verbose;
  uint8_t pack;
  uint8_t nodynamic;
  uint8_t dynlib;
  char* outfile_name;
  uint8_t outfile_type;
  int dynamic_entry_num;
  int virtual_address_offset;
  int output_vaddr_alignment;
  void* entry_address;
  int program_header_num;
  int shdr_num;
  int shstrndx;
  int strndx;
  //  PhdrList;
  SectionContainer* initial_section;
  SectionContainer* current_section;
  ObjectChain* initial_object;
  ObjectChain* current_object;  
  struct SymbolHashTable ExportHashTable;
  struct SymbolHashTable DLLHashTable;
  struct SymbolHashTable DynamicImportHashTable;  
  void* mem;
} Config;

SectionContainer* alloc_section_container_init(uint32_t va, void* name, void* candidate_list, ListContainer* Sc);

SectionContainer* alloc_section_container(uint32_t va, void* name, void* candidate_list, ListContainer* Sc);

SectionContainer* match_section(char* name);

// object.c
ObjectChain* alloc_obj_chain_init(void* sym_begin, void* str_begin, uint32_t sym_num);
ObjectChain* _alloc_obj_chain(void* sym_begin, void* str_begin, uint32_t sym_num);
void update_object_chain(ObjectChain* oc, SectionChain* schain);
// section.c
void* alloc_section_chain(void* s, void* offset, SectionContainer* scon);

uint32_t elf_hash(const uint8_t* name);

#define M1(H,F,X) (H.bucket + (F(X) % H.nbucket))

#define M2(A,B,H,F,X) \
  A = (F(X) % H.nbucket);\
  B = H.bucket + A

