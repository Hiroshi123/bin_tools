

#define SECTION_ALIGNMENT 0x1000
#define FILE_ALIGNMENT 0x200

#define GET_NAME(X,Y) (*(uint32_t*)X == 0) ? (char*) ((size_t)Y+*((uint32_t*)X+1)) : X->N.ShortName;

typedef struct _SectionChain SectionChain;
typedef struct _ObjectChain ObjectChain;
typedef struct _SymbolChain SymbolChain;
typedef struct _SectionContainer SectionContainer;
typedef struct _SymbolChain3 SymbolChain3;

struct _SymbolChain {
  SymbolChain* next;
  IMAGE_SYMBOL* p;
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
struct _SectionChain {
  // section_num
  void* data;
  SectionChain* next;
  SectionChain* this;    
  union {
    size_t num;
    IMAGE_SECTION_HEADER* p;
  };
  // pointer to object
  ObjectChain* obj;  
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
  // this is not for strcmp() but get all of entry under this object chain.
  // it will point entry if the bit is set, then the entry is stored on the record.
  // this is used for constructing export table virtual address.
  // Note you do not need to compare anything when you wanna just get a list not for resolution.
  SymbolChain* symbol_chain_head;
  SymbolChain* symbol_chain_tail;
  SectionChain* section_chain_head;
  SectionChain* section_chain_tail;
  // size_t otherwise
  IMAGE_SYMBOL* symbol_table_p;
  uint8_t* str_table_p;
};

typedef struct __attribute__((__packed__)) _CoffReloc {
  uint32_t VirtualAddress;
  uint32_t SymbolTableIndex;
  uint16_t Type;
} CoffReloc;

typedef struct __attribute__((__packed__)) _CallbackArgIn {
  size_t* virtual_address;
  char* name;
  size_t* type;
} CallbackArgIn;

struct _SymbolChain3 {
  SymbolChain3* next;
  SymbolChain3* this;
  char* name;
};

