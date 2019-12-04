
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#include "coff.h"

#define DEBUG 1
#define SECTION_ALIGNMENT 0x1000
#define FILE_ALIGNMENT 0x200

#define GET_NAME(X,Y) (*(uint32_t*)X == 0) ? (char*) ((size_t)Y+*((uint32_t*)X+1)) : X->N.ShortName;

extern callback_arg3(void* arg1,void* arg2,void* f);
/* extern int run_through_coff_shdr3(void*,void*); */

typedef struct _SectionChain SectionChain;
typedef struct _ObjectChain ObjectChain;
typedef struct _SymbolChain SymbolChain;
typedef struct _SectionContainer SectionContainer;
typedef struct _HEAP HEAP;
typedef struct _DynamicResolvedSymbolTable DynamicResolvedSymbolTable;

struct _HEAP {
  HEAP* next;
  void* p;
};

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

struct _DynamicResolvedSymbolTable {
  DynamicResolvedSymbolTable* next;
  SymbolChain* this;
  char* dllname;
};

static DynamicResolvedSymbolTable* InitialImport = 0;
static DynamicResolvedSymbolTable* CurrentImport = &InitialImport;

static HEAP* CurrentHeap = 0;
static HEAP* InitHeap = &CurrentHeap;

static struct SymbolHashTable HashTable = {};
static struct SymbolHashTable DLLHashTable = {};

static SymbolChain* CurrentSymbol = 0;
static SymbolChain* InitialSymbol = &CurrentSymbol;
static SectionChain* InitialSection = 0;
static SectionChain* CurrentSection = 0;

static ObjectChain* InitialObject = 0;
static ObjectChain* CurrentObject = &InitialObject;

static uint8_t TotalSectionNum = 0;
static uint32_t TotalImageSize = 0;
static uint32_t TotalHeaderSize = 0;
static uint32_t CurrentFileOffset = 0;
static HANDLE hFile = 0;
static void* VirtualAddressOffset = 0x0;
static uint32_t EXPORT_SYMBOL_NUM = 0;
static uint32_t IMPORT_SYMBOL_NUM = 0;
static uint32_t ImportDirectoryLen = 0;
static uint32_t ExportDirectoryLen = 0;
static uint32_t ExportFuncCount = 0;

uint32_t elf_hash(const uint8_t* name) {
  uint32_t h = 0, g;
  for (; *name; name++) {
    h = (h << 4) + *name;
    if (g = h & 0xf0000000) {
      h ^= g >> 24;
    }
    h &= ~g;
  }
  return h;
}

void set_optional_data_dir(uint32_t va, uint32_t size) {
  
}

/*static inline */const char check_coff(const uint16_t* p) {
  return *p == 0x8664;
}

// 1st :: IMAGE_FILE_HEADER*

// 2nd :: IMAGE_SECTION_HEADER*
// 0x00 name
// 0x08 PhysicalAddress
// 0x0c VirtualAddress
// 0x10 SizeOfRawData . 5
// 0x14 PointerToRawData .
// 0x18 PointerToRelocation
// 0x1c PointerToLineNumbers
// 0x20 NumberOfRelocation
// 0x22 NumberOfLineNumbers
// 0x24 Charactestics

void* alloc_section_chain(void* obj ,IMAGE_SECTION_HEADER* s ,SectionChain* _s) {
  
  SectionChain* sec1;
  SectionChain* sec2;
  if (!_s) {
    sec1 = malloc(sizeof(SectionChain));
    sec1->num = 1;
    sec1->next = 0;
    TotalSectionNum += 1;
  } else {
    sec1 = _s;
  }
  sec2 = malloc(sizeof(SectionChain));
  sec2->p = s;
  sec2->data = obj ? obj + s->PointerToRawData : 0;// datap;
  sec2->this = 0;
  sec2->obj = obj ? CurrentObject : 0;
  sec2->next = 0;
  sec1->this = sec2;
  if (!InitialSection) {
    InitialSection = sec1;    
  } else {
    if (!_s) {
      CurrentSection->next = sec1;
    }
  }
  if (!_s)
    CurrentSection = sec1;
  
  if (CurrentObject->section_chain_head == 0) {
    CurrentObject->section_chain_head = sec2;
  } else {
    CurrentObject->section_chain_tail->next = sec2;
  }
  CurrentObject->section_chain_tail = sec2;  
  TotalImageSize += (s->SizeOfRawData + SECTION_ALIGNMENT) & 0xFFFFF000;
  return sec1;
}

SectionChain* check_section(IMAGE_SECTION_HEADER* _sec) {
  
  SectionChain* s = InitialSection;
  IMAGE_SECTION_HEADER* sec;
  for (;s;s = s->next) {
    sec = s->this->p;
    printf("!!%p,%p,%s,%s\n", s, s->this->p, sec->Name, _sec->Name);    
    if (!strcmp(sec->Name, _sec->Name)) {
      for (;s->this;s=s->this);
      printf("ss:%p\n",s);
      return s;
    }
  }
  return 0;
}

void* get_export_virtual_address(IMAGE_SYMBOL* is, ObjectChain* oc) {
  SectionChain* sc1 = oc->section_chain_head;
  // FIXME : if this is not the way that it should be,
  // change it to the proper way.
  int i = 0;
  IMAGE_SECTION_HEADER* ss;
  for (;sc1;sc1=sc1->next) {
    i++;
    if (i == is->SectionNumber) {
      ss = sc1->p;
      // Value is the offset from header of the section.
      // If you find another section above the section,
      // you need to know
      printf("!%s,%p,%p\n", ss->Name, ss->VirtualAddress, is->Value);
      return ss->VirtualAddress + is->Value;
    }
  }
  return 0;
}

void* lookup_symbol(char* name, size_t* address) {
  size_t* table_index = (HashTable.bucket + (elf_hash(name) % HashTable.nbucket));
  // if you do not find any entry on this table.
  if (*table_index == 0) {
    return address ? 0 : table_index;
  }
  SymbolChain* pre_chain;
  IMAGE_SYMBOL* is;
  ObjectChain* oc;
  char* _name;
  IMAGE_SYMBOL* begin;
  IMAGE_SYMBOL* end;
  // if you find any entry then trace the chain which was stored.
  pre_chain = *table_index;
  // check hash collision
  for (;pre_chain;pre_chain = pre_chain->next) {
    is = pre_chain->p;
    // find actual entry
    for (oc=InitialObject;oc;oc=oc->next) {
      begin = oc->symbol_table_p;
      end = begin + oc->symbol_num;
      if (begin<is && is<end) {
	_name = GET_NAME(is, oc->str_table_p);
	break;
      }
    }
    if (!strcmp(name,_name)) {
      printf("matched\n");
      if (address)
	*address = get_export_virtual_address(is, oc);
      return address ? is : 0;
    }
  }
  // the another way is return a last-put entry instead 0.
  return address ? 0 : pre_chain;
}

void* lookup_dynamic_symbol(char* name, size_t* address) {
  size_t* table_index = (DLLHashTable.bucket + (elf_hash(name) % DLLHashTable.nbucket));
  if (*table_index == 0) {
    return address ? 0 : table_index;
  }
  SymbolChain* pre_chain;
  char* _name;
  SymbolChain* s2;
  pre_chain = *table_index;
  for (;pre_chain;pre_chain = pre_chain->next) {
    s2 = pre_chain->p;
    _name = s2->p;
    if (!strcmp(name,_name)) {
      printf("found,%s\n", _name);
      return address ? s2->next : pre_chain->next;
      // return address ? _name : 0;
    }
  }
  return address ? 0 : pre_chain;
  // "kernel32" DLL(symbol)
  // "hash(ExitProcess)"
}

// 
void* alloc_dynamic_symbol(char* name, size_t* dllname) {
  size_t* ret = lookup_dynamic_symbol(name, 0);
  if (ret) {
    SymbolChain* chain = malloc(sizeof(SymbolChain));
    SymbolChain* chain2 = malloc(sizeof(SymbolChain));    
    chain->next = 0;
    chain->p = chain2;
    chain2->next = dllname;
    chain2->p = name;
    *ret = chain;
  } else {
    // should not be happened.
  }
}


void alloc_symbol_chain(char* name, void* is) {
  SymbolChain* chain = malloc(sizeof(SymbolChain));
  SymbolChain* pre_chain;
  chain->next = 0;
  chain->p = is;
  size_t* ret = lookup_symbol(name, 0);
  if (ret) *ret = chain;
  else {
    printf("should raise an error as symbol is overrlapping.\n");
  }
  printf("aaaa:%s,%p,%p,%p\n", name, ret,chain,*ret);  
  // you need to allocate another symbolchain for object chain.
  chain = malloc(sizeof(SymbolChain));
  chain->next = 0;
  chain->p = is;
  if (!CurrentObject->symbol_chain_head)
    CurrentObject->symbol_chain_head = chain;
  else
    CurrentObject->symbol_chain_tail->next = chain;
  CurrentObject->symbol_chain_tail = chain;
  /*
  if (*table_index == 0) {
    *table_index = chain->p;
  } else {
    // hash collision
    pre_chain = *table_index;
    for (;pre_chain->next;pre_chain = pre_chain->next);
    // you should also check the data itself is same.
  }
  */
}

void alloc_obj_chain(void* sym_begin, void* str_begin, uint32_t sym_num) {
  ObjectChain* sc = malloc(sizeof(ObjectChain));
  if (!InitialObject) {
    InitialObject = sc;
  } else {
    CurrentObject->next = sc;
  }
  CurrentObject = sc;
  CurrentObject->symbol_chain_head = 0;
  CurrentObject->symbol_chain_tail = 0;
  CurrentObject->symbol_table_p = sym_begin;
  CurrentObject->str_table_p = str_begin;
  CurrentObject->symbol_num = sym_num;
  CurrentObject->section_chain_head = 0;
  CurrentObject->section_chain_tail = 0;  
  CurrentObject->next = 0;
}

char read_coff(const char *const begin,
               info_on_coff *e) {

  IMAGE_FILE_HEADER* img_file_header = (IMAGE_FILE_HEADER*)begin;
  int sec_num = img_file_header->NumberOfSections;  
  IMAGE_SYMBOL* sym_begin = (IMAGE_SYMBOL*)(begin + img_file_header->PointerToSymbolTable);
  IMAGE_SYMBOL* sym_end = sym_begin + img_file_header->NumberOfSymbols;
  e->sym_begin = (IMAGE_SYMBOL*)(begin + img_file_header->PointerToSymbolTable);
  e->sym_end = e->sym_begin + img_file_header->NumberOfSymbols;
  IMAGE_SYMBOL* is = e->sym_begin;
  const char* str_begin = e->sym_end;  
  alloc_obj_chain(sym_begin, str_begin, img_file_header->NumberOfSymbols);
  
  e->sec_begin = (IMAGE_SECTION_HEADER*)(img_file_header+1);
  e->sec_end = e->sec_begin + sec_num;  
  IMAGE_SECTION_HEADER* s = e->sec_begin;
  char filled = 0;
  IMAGE_SECTION_HEADER* _sec;
  for (;s!=e->sec_end;s++) {
    // if you find an existing section which has the same section name as
    // the coming section name, then you need to merge them.
    // To do this, you need to check the list of existing section,
    _sec = check_section(s);
    alloc_section_chain(begin, s, _sec);
    if (s->NumberOfRelocations == 0/* && s->PointerToRelocations == 0*/) {}
    else s->PointerToRelocations += begin;
#ifdef DEBUG
    printf("%s\n",s->Name);
    printf("%x\n", s->NumberOfRelocations);
    printf("reloc:%x\n", s->PointerToRelocations);
    printf("relocN:%x\n", s->NumberOfRelocations);
    printf("%x\n", s->PointerToRawData);
    printf("%x\n", s->SizeOfRawData);
    printf("---\n");
#endif
  }
  char* name;
  for (;is!=e->sym_end;is++) {
    name = GET_NAME(is, str_begin);
    
    // StorageClass == 2 is import/export
    // difference between import and export is if it belongs to section(0) or not.
    // import section should belong to section(0) which means nothing.
    // export section belongs a section from 1.
    if (is->StorageClass == 2 && is->SectionNumber) {
      printf("alloc symbol\n");
      alloc_symbol_chain(name, is);
      // number of export symbols and its string size should be
      // counted for constructing export table on later stage.
      // EAT + ENT + ordinal(WORD)      
      ExportDirectoryLen += 4 + 2 + strlen(name) + 1;
      ExportFuncCount += 1;
    }
#ifdef DEBUG
    printf("val:%x\n",is->Value);
    printf("%s\n",name);
    printf("hash:%p\n", elf_hash(name));
    printf("hash index:%p\n", elf_hash(name) % HashTable.nbucket);    
    printf("value:%x\n", is->Value);
    printf("sectionNum:%x\n", is->SectionNumber);
    printf("type:%x\n", is->Type);
    printf("storageClass:%x\n", is->StorageClass);
    printf("auxNum:%x\n", is->NumberOfAuxSymbols);
    printf("----\n");
#endif
    // jump aux vector if it has
    for (;is!=is+is->NumberOfAuxSymbols;is++);
    // HashTable.bucket = 1;
  }
}

void* alloc_obj(char* fname) {

  HANDLE hFile = CreateFile
    (
     fname, GENERIC_ALL/* | GENERIC_EXECUTE*/, 0, NULL,
     OPEN_EXISTING/*CREATE_NEW*/, 0/*FILE_SHARE_READ*/, NULL
     );
  if (hFile == -1) {
    printf("file not found\n");
    return 0;
  }
  DWORD wReadSize;
  DWORD size = GetFileSize(hFile , NULL);
  if (size == -1) {
    printf("cannot get file size\n");
    return 0;
  }
  uint64_t* p = (LPSTR)malloc(size);
  ReadFile(hFile, p, size, &wReadSize , NULL);
  return p;
}

// section name
// virtual size(this should be smaller than actual size)
// virtual address
// size of raw data
// file pointer to raw data
// 1. file pointer to relocation table
// file pointer to line numbers
// number of relocations()
// number of line numbers
// flags(charactestics of this section)

// symbol table information
//

void* write_section(SectionChain* sc) {
  IMAGE_SECTION_HEADER* sec = sc->p;
  SectionChain* s = sc;
  uint32_t size = 0;
  for (;s;s=s->this) {
    size += s->p->SizeOfRawData;
    printf("ssss\n");
  }
  sec->Misc.VirtualSize = size;
  // sec->VirtualAddress = VirtualAddressOffset;
  // VirtualAddressOffset += (sec->Misc.VirtualSize + SECTION_ALIGNMENT) & 0xFFFFF000;
  // VirtualSize == 0 wont work out.
  if (!sec->Misc.VirtualSize)
    sec->Misc.VirtualSize = 1;
  // int original = sec->PointerToRawData;
  // CurrentFileOffset = (CurrentFileOffset + FILE_ALIGNMENT - 1) & CurrentFileOffset;
  /* sec->PointerToRawData = CurrentFileOffset; */
  /* CurrentFileOffset += (sec->SizeOfRawData + FILE_ALIGNMENT - 1) & (0 - FILE_ALIGNMENT); */
  sec->PointerToRelocations = 0;
  sec->PointerToLinenumbers = 0;
  sec->NumberOfRelocations = 0;
  sec->NumberOfLinenumbers = 0;
  /* sec->Characteristics = 0xE0000080; */
  DWORD dwWriteSize;
  WriteFile(hFile ,sec ,sizeof(IMAGE_SECTION_HEADER) ,&dwWriteSize , NULL);
  // sec->PointerToRawData = original;
  return sec;
}

void gen() {

  char dummy_name[] = "a01.exe";
  hFile = CreateFile
    (
     dummy_name , GENERIC_ALL/* | GENERIC_EXECUTE*/, 0, NULL,
     CREATE_ALWAYS, 0/*FILE_SHARE_READ*/, NULL
     );
  
  printf("%x\n",hFile);
  
  int size = sizeof(IMAGE_DOS_HEADER);
  IMAGE_DOS_HEADER* dos_header = malloc(size);
  memset(dos_header, 0, size);
  dos_header->e_magic = 0x5a4d;
  dos_header->e_cblp = 0xff;
  dos_header->e_lfarlc = 0x60;
  dos_header->e_lfanew = 0x40;
  
  size = sizeof(IMAGE_FILE_HEADER);
  IMAGE_FILE_HEADER* file_header = malloc(size);
  memset(file_header, 0, size);
  // 
  file_header->Machine = 0x8664;
  file_header->NumberOfSections = TotalSectionNum;
  file_header->TimeDateStamp = 0;
  file_header->PointerToSymbolTable = 0;
  file_header->NumberOfSymbols = 0;
  file_header->SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
  file_header->Characteristics = 0x22f;
  // 
  size = sizeof(IMAGE_OPTIONAL_HEADER64);
  IMAGE_OPTIONAL_HEADER64* optional_header = malloc(size);
  memset(optional_header, 0, size);
  // 0c10b or 0x20b
  optional_header->Magic = 0x20b;
  // values which could be anything...
  optional_header->MajorLinkerVersion = 0;
  optional_header->MinorLinkerVersion = 0;
  // these 3 needs to be correct.
  optional_header->SizeOfCode = 0;
  optional_header->SizeOfInitializedData = 0;
  optional_header->SizeOfUninitializedData = 0;
  // this is often set as 0x1000  
  optional_header->BaseOfCode = 0x1000;
  // 
  optional_header->AddressOfEntryPoint = 0x1000;
  
  ///////////////////////////////////////////////////////
  
  optional_header->MajorOperatingSystemVersion = 0;
  optional_header->MinorOperatingSystemVersion = 0;
  optional_header->MajorImageVersion = 0;
  optional_header->MinorImageVersion = 0;
  // subsystem needs 
  optional_header->MajorSubsystemVersion = 5;
  optional_header->MinorSubsystemVersion = 2;
  
  // 
  optional_header->ImageBase = 0x400000;// 0x000;
  // 
  TotalHeaderSize = sizeof(IMAGE_DOS_HEADER) + 4 + sizeof(IMAGE_FILE_HEADER)
    + sizeof(IMAGE_OPTIONAL_HEADER64) + sizeof(IMAGE_SECTION_HEADER) * TotalSectionNum;
  VirtualAddressOffset = ((TotalHeaderSize + SECTION_ALIGNMENT) & 0xFFFFF000);
  TotalImageSize += VirtualAddressOffset;
  optional_header->SizeOfImage =  + TotalImageSize;
  optional_header->SizeOfHeaders = TotalHeaderSize;
  // 
  optional_header->SectionAlignment = SECTION_ALIGNMENT;
  optional_header->FileAlignment = FILE_ALIGNMENT;
  // DLL characterstics which is probably most important
  optional_header->DllCharacteristics = 0;
  optional_header->Subsystem = 3;
  
  optional_header->SizeOfStackReserve;
  optional_header->SizeOfStackCommit;
  optional_header->SizeOfHeapReserve;
  optional_header->SizeOfHeapCommit;
  
  optional_header->LoaderFlags = 0;  
  // optional_header->NumberOfRvaAndSizes = 0;
  // optional_header->BaseOfData = 0;
  
  // optional_header->NumberOfRvaAndSizes = 0x10;

  // 
  // set_optional_data_dir(uint32_t va, uint32_t size);
  
  // Export Directory
  /* optional_header->DataDirectory[0].VirtualAddress = 0x11; */
  /* optional_header->DataDirectory[0].Size = 0x11; */
  /* // Import Directory */
  /* optional_header->DataDirectory[1].VirtualAddress = 0x11; */
  /* optional_header->DataDirectory[1].Size = 0x11; */
  
  optional_header->NumberOfRvaAndSizes = 0x10;
  
  printf("%p,%d\n", file_header,size);
  
  DWORD dwWriteSize;
  size = sizeof(IMAGE_DOS_HEADER);
  WriteFile(hFile , dos_header , size , &dwWriteSize , NULL);
  size = 4;
  DWORD nt_header_signature = 0x4550;
  WriteFile(hFile , &nt_header_signature , size , &dwWriteSize , NULL);
  size = sizeof(IMAGE_FILE_HEADER);
  WriteFile(hFile , file_header , size , &dwWriteSize , NULL);
  size = sizeof(IMAGE_OPTIONAL_HEADER);
  WriteFile(hFile , optional_header , size , &dwWriteSize , NULL);

  IMAGE_SECTION_HEADER* sec;
  
  CurrentFileOffset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
  CurrentFileOffset += TotalSectionNum * sizeof(IMAGE_SECTION_HEADER);
  printf("curp:%p,%p,%p\n", CurrentFileOffset,CurrentFileOffset + FILE_ALIGNMENT - 1,
	 FILE_ALIGNMENT);
  CurrentFileOffset = (CurrentFileOffset + FILE_ALIGNMENT - 1) & (0 - FILE_ALIGNMENT);
  printf("curp:%p\n", CurrentFileOffset);
  SectionChain* s = InitialSection;
  for (;s;s = s->next) {
    sec = s->this->p;
    // printf("!!%p,%p,%s\n", s, s->this->p, sec->Name);
    write_section(s->this);
  }
  // Actual Data is going to be fed.
  s = InitialSection;
  SectionChain* s1;
  for (;s;s = s->next) {
    sec = s->this->p;
    printf("!!pointer to raw,%p,%p\n", sec->PointerToRawData, sec->SizeOfRawData);
    if (sec->SizeOfRawData) {      
      DWORD fp = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
      printf("%p,%p,%p,%s\n",fp, sec->PointerToRawData,sec->PointerToRawData - fp, sec->Name);
      SetFilePointer(hFile, sec->PointerToRawData - fp, NULL, FILE_CURRENT);
      for (s1=s->this;s1;s1=s1->this) {
	WriteFile(hFile ,s1->data ,s1->p->SizeOfRawData ,&dwWriteSize , NULL);
      }
      printf("ww\n");
    }
  }
  CloseHandle(hFile);
  // read_section_list();  
}

void set_virtual_address() {
  SectionChain* sc = InitialSection;
  SectionChain* _sc;
  IMAGE_SECTION_HEADER* sh;
  uint32_t offset = 0x200;
  // Iterate on SectionContainer
  uint32_t virtual_address = 0x1000;    
  for (;sc;sc=sc->next) {
    // Iterate on SectionChain
    /* uint32_t virtual_size = 0; */
    for (_sc = sc->this;_sc;_sc=_sc->this) {  
      sh = _sc->p;
      sh->VirtualAddress = virtual_address;
      sh->PointerToRawData = offset;
      offset += sh->SizeOfRawData;
      printf("name:%s\n",sh->Name);
      printf("p:%p\n", sh->PointerToRawData);
      printf("s:%p\n", sh->SizeOfRawData);
      printf("v:%p\n", sh->VirtualAddress);
      printf("r:%p\n", sh->PointerToRelocations);      
      printf("d:%p\n", _sc->data);
      /* offset += (sh->SizeOfRawData + FILE_ALIGNMENT - 1) & (0 - FILE_ALIGNMENT); */
      // virtual_size += sh->SizeOfRawData;
      // VirtualAddressOffset += (sh->VirtualAddress + SECTION_ALIGNMENT) & 0xFFFFF000;
      // printf("offset:%p,%p\n",offset, VirtualAddressOffset);
    }
    offset = (offset + FILE_ALIGNMENT - 1) & (0 - FILE_ALIGNMENT);
    virtual_address = (virtual_address + SECTION_ALIGNMENT) & 0xFFFFF000;
  }
}

void add_export(SectionChain* _sec) {
  IMAGE_SECTION_HEADER* sec = _sec->p;
  // char* image_name = "a01.exe";
  void* export_data_p = malloc
    (ExportDirectoryLen + sizeof(IMAGE_EXPORT_DIRECTORY) + strlen("a01.exe"));
  void* export_data_begin = export_data_p;
  IMAGE_EXPORT_DIRECTORY* ied = export_data_p;
  printf("aa,%p\n", ied);
  ied->Characteristics = 0;
  ied->TimeDateStamp = 0;
  ied->MajorVersion = 0;
  ied->MinorVersion = 0;
  ied->Name = sec->VirtualAddress;
  ied->Base = 0;
  ied->NumberOfFunctions = ExportDirectoryLen;
  ied->NumberOfNames = ExportDirectoryLen;
  // added later on along with actual allocation
  ied->AddressOfFunctions = sec->VirtualAddress;
  ied->AddressOfNames = sec->VirtualAddress;
  ied->AddressOfNameOrdinals = sec->VirtualAddress;
  
  ObjectChain* oc;
  SymbolChain* sc;
  IMAGE_SYMBOL* is;
  char* name;
  char* image_name = ied+1;
  strcpy(image_name, "a01.exe");
  WORD* addressOfNameOrdinals = image_name + strlen(image_name) + 1;
  DWORD* addressOfFunctions = addressOfNameOrdinals + ExportFuncCount;
  char* addressOfNames = addressOfFunctions + ExportFuncCount;
  DWORD export_addr;
  if (export_data_p)
  for (;oc;oc=oc->next) {
    sc = oc->symbol_chain_head;
    for (;sc;sc=sc->next) {
      is = sc->p;
      name = GET_NAME(sc->p, oc->str_table_p);
      strcpy(addressOfNames, name);
      addressOfNames += strlen(name) + 1;
      export_addr = get_export_virtual_address(is, oc);
      addressOfFunctions = export_addr;
      printf("%s,%p,%d\n",name, sc->p, sc->p->StorageClass);
      // should contain some virtual address
      // addressOfNameOrdinals = 
    }
  }
}

void* add_dynamic_resolved_entry(char* name, char* dllname) {
  DynamicResolvedSymbolTable* e = InitialImport;
  SymbolChain* s1;
  for (;e;e=e->next) {
    if (!strcmp(e,dllname)) {
      s1 = e->this;
      for (;s1;s1=s1->next);
      SymbolChain* entry = malloc(sizeof(SymbolChain));
      entry->next = 0;
      entry->p = name;
      s1 = entry;
      return 0;
    }
  }
  DynamicResolvedSymbolTable* entry = malloc(sizeof(DynamicResolvedSymbolTable));
  entry->next = 0;
  entry->this = name;
  entry->dllname = dllname;  
  if (CurrentImport) {
    CurrentImport->next = entry;
  }
  CurrentImport = entry;
  return 1;
}

void alloc_import_directory(IMAGE_SECTION_HEADER* import_section) {

  void* import_data_begin = malloc(ImportDirectoryLen);
  uint8_t* begin = import_data_begin;
  IMAGE_IMPORT_DESCRIPTOR* iid = import_data_begin;
  // this thunk will point on the head of an array named image thunk data
  // (where each of them has a pointer to image import by name)
  /* void* dll_name = iid + 1; */
  /* iid->Name += (dll_name - import_data_begin); */
  /* printf("dll:%p\n", dll_name - import_data_begin); */
  /* char* kernel32 = "kernel32"; */
  /* strcpy(dll_name, kernel32); */
  /* IMAGE_THUNK_DATA* iat = dll_name + 8; */
  /* iid->OriginalFirstThunk += ((void*)iat - import_data_begin); */
  /* void* function_name = iat+1; */
  /* iid->FirstThunk += function_name - import_data_begin; */
  /* strcpy(function_name, "ExitProcess"); */
  
  DynamicResolvedSymbolTable* e = InitialImport;
  SymbolChain* s1;
  for (;e;e=e->next) {
    // OriginalFirstThunk(IAT),FirstThunk(INT),Name(DLLNAME) should be
    // initialized as section virtual address
    iid->OriginalFirstThunk = import_section->VirtualAddress;
    iid->TimeDateStamp = 0;
    iid->ForwarderChain = 0;    
    iid->Name = import_section->VirtualAddress;
    iid->FirstThunk = import_section->VirtualAddress;
    iid++;
  }
  uint8_t* c = iid;
  IMAGE_THUNK_DATA* iat = c;  
  for (e=InitialImport;e;e=e->next,iid++) {
    for (s1=e->this;s1;s1=s1->next) {
      iat->u1.AddressOfData = import_section->VirtualAddress;
      iat++;
    }
  }
  c = iat;
  iat = iid; // back to the begining of IAT
  iid = import_data_begin; // back to the beginning of IID
  for (e=InitialImport;e;e=e->next,iid++,iat++) {
    printf("DLL:%s\n",e->dllname);
    strcpy(c, e->dllname);    
    // DLL_NAME
    iid->Name += c - begin;
    c += strlen(c);
    // INT
    iid->OriginalFirstThunk += c - begin;    
    for (s1=e->this;s1;s1=s1->next) {
      printf("f:%s\n", s1->p);
      // IAT
      iat->u1.AddressOfData += c - begin;
      strcpy(c, s1->p);
      c += 2 + strlen(c);
    }
    iid->FirstThunk += (uint8_t*)iat - begin;
  }
}

void resolve_on_export_obj(CallbackArgIn* _in,uint32_t* addr) {
  char* name = _in->name;
  uint16_t type = _in->type;
  size_t* export_address = 0;
  IMAGE_SYMBOL* ret = lookup_symbol(name, &export_address);
  if (ret) {
    printf("callback called,%s,%d,%p,%d,%d,%p(!%x),%p\n",
	   name, type,ret,ret->SectionNumber,ret->Value,addr,*(uint64_t*)addr,export_address);
    if (type == IMAGE_REL_AMD64_ADDR32)
      *addr = export_address;
    else if (type == IMAGE_REL_AMD64_REL32)
      *addr = _in->virtual_address - export_address;
  } else {
    char* dllname = lookup_dynamic_symbol(name, 1);
    if (dllname) {
      void* new = add_dynamic_resolved_entry(name, dllname);
      if (new)
	ImportDirectoryLen += sizeof(IMAGE_IMPORT_DESCRIPTOR) + 1 + strlen(dllname);
      ImportDirectoryLen += 4 + 2 + strlen(name);
      printf("%s was resolved on %s\n", name, dllname);
    } else {
      printf("could not resolved.. \n");
    }
    // if you could not resolve, it will be re-evaluated with the dynamic loader at 2nd stage.
    // Dynamic loader
  }
}

void wrapper_f(void* arg1, void* arg2, void* f) {
  callback_arg3(arg1,arg2,f);
}

void do_reloc(void* f) {
  SectionChain* sec1 = InitialSection;
  SectionChain* sec2;
  ObjectChain* oc1;
  IMAGE_SYMBOL* is1;
  IMAGE_SECTION_HEADER* ish;
  int i = 0;
  CoffReloc* reloc;
  CallbackArgIn arg;
  uint32_t* addr;
  for (;sec1;sec1=sec1->next) {
    for (sec2=sec1->this;sec2;sec2=sec2->this) {
      if (ish=sec2->p) {
	reloc = ish->PointerToRelocations;
	oc1 = sec2->obj;
	for (i=0;i<ish->NumberOfRelocations;i++,reloc++) {
	  is1 = oc1->symbol_table_p + reloc->SymbolTableIndex;
	  arg.name = GET_NAME(is1, oc1->str_table_p);
	  arg.virtual_address = ish->VirtualAddress + reloc->VirtualAddress;
	  arg.type = reloc->Type;
	  addr = sec2->data + reloc->VirtualAddress;
	  if (*addr) {
	    printf("should skip!!\n");
	  }
	  //printf("a,%p,%d,%p,%p,%p\n",ish->VirtualAddress + reloc->VirtualAddress
	  //	 ,ish->NumberOfRelocations,addr,*addr,reloc->Type);
	  // call wrapper function of callback
	  wrapper_f(&arg,addr,f);
	  // printf("a,%p,%d,%p,%p\n",sec2,ish->NumberOfRelocations,addr,*addr);
	}
      }
    }
  }
}

void add_import(void* _is) {
  SectionChain* _sc = _is;
  IMAGE_SECTION_HEADER* import_section = _sc->this->p;
  // 1. know how much size is needed for dynamic relocation.
  // Number of entry function(Kernel32/ExitProcess)
  // Number of image_import_descriptor(DLL)
  // IMAGE_THUNK_DATA is GOT.
  // it originally refers image_import_by_name
  // SymbolChain referred from ObjectChain.symbol_chain will hold list
  // of SymbolChain and its entry is import(StorageClass=2/SectionIndex=0),
  // needs to be resolved.
  // At first, you must know how many DLL needs to be resolved.
  
#ifdef DEBUG
  printf("name:%s\n", import_section->Name);
  printf("virtual address:%p\n", import_section->VirtualAddress);
  printf("virtual size:%p\n", import_section->Misc.VirtualSize);
  printf("pointer to raw data:%p\n", import_section->PointerToRawData);
  printf("size of raw data:%p\n", import_section->SizeOfRawData);
#endif
  /* total_iid = import_section->VirtualAddress; */
  /* total_entry = 1; */
  /* total_strlen = strlen("ExitProcess"); */
  /* uint32_t total = total_iid * sizeof(IMAGE_IMPORT_DESCRIPTOR) + total_entry * 4 + total_strlen; */
  /* printf("total:%d\n"); */
  import_section->SizeOfRawData = ImportDirectoryLen;
  import_section->Characteristics = 0x60500020;
  
  /* void* import_data_begin = malloc(ImportDirectoryLen); */
  /* _sc->this->data = import_data_begin;   */
  /* IMAGE_IMPORT_DESCRIPTOR* iid = import_data_begin; */
  /* // this thunk will point on the head of an array named image thunk data */
  /* // (where each of them has a pointer to image import by name) */
  /* iid->OriginalFirstThunk = import_section->VirtualAddress; */
  /* iid->TimeDateStamp = 0; */
  /* iid->ForwarderChain = 0; */
  /* iid->Name = import_section->VirtualAddress; */
  /* iid->FirstThunk = import_section->VirtualAddress; */
  /* void* dll_name = iid + 1; */
  /* iid->Name += (dll_name - import_data_begin); */
  /* printf("dll:%p\n", dll_name - import_data_begin); */
  /* char* kernel32 = "kernel32"; */
  /* strcpy(dll_name, kernel32); */
  /* IMAGE_THUNK_DATA* iat = dll_name + 8; */
  /* iid->OriginalFirstThunk += ((void*)iat - import_data_begin); */
  /* void* function_name = iat+1; */
  /* iid->FirstThunk += function_name - import_data_begin; */
  /* strcpy(function_name, "ExitProcess"); */
  
  // 0xe8 (0x00 0x00 0x00 0x00)
  // 0. 0xff 0x25 (0x20 0x30 0x00 0x01)
  // 1. 0cff 0x25
  // 
  // iat->AddressOfData = 
  // IID + Name + entryNum * (IAT(4byte) + INT(Name of the function + 2) )
  //
  
  // you need to provide a virtual address()
  // Each Section should contain estimated virtual address of each symbol.
  
  // You must determinte where is going to be of each virtual address.
  // But it depends on how many section is existed before the section and the each size of
  // each section.
  // if you cannot determine virtual address, you cannot fill data.
  // It means you need to fill virtual address before creating image import directory entry.
  
}

void* add_section(char* name) {
  
  IMAGE_SECTION_HEADER* sec = malloc(sizeof(IMAGE_SECTION_HEADER));
  strcpy(sec->Name, name);
  sec->NumberOfRelocations = 0;
  sec->PointerToRelocations = 0;
  sec->PointerToRawData = 0;
  sec->SizeOfRawData = 0;
  // you cannot fill any data in this stage as the virtual address has not yet been decided.
  return alloc_section_chain(0, sec, 0);
}

void init_hashtable() {

  HashTable.nbucket = 100;
  HashTable.nchain = 0;
  HashTable.bucket = malloc(HashTable.nbucket*4);
  memset(HashTable.bucket,HashTable.nbucket*4,0);
  
  DLLHashTable.nbucket = 10;
  DLLHashTable.nchain = 0;
  DLLHashTable.bucket = malloc(DLLHashTable.nbucket*4);
  memset(HashTable.bucket,HashTable.nbucket*4,0);
  
  alloc_dynamic_symbol("ExitProcess","kernel32");  
}

void init() {  
  init_hashtable();  
}

void* read_cmdline(int argc, char** argv) {
  uint8_t file_num = argc;
  uint8_t i = 1;
  size_t* p = malloc(sizeof(void*) * (argc));
  void* _p = p;
  for (;i<argc;i++) {
    /* if (strcmp()) { */      
    /* } */    
    *p = argv[i];
    p++;
  }
  *p = 0;
  return _p;
}

int main(int argc, char** argv) {
  
  // 1.create_section_chain
  // 2.create_symbol_chain
  // 3.relocation finding(all of import should be resolved except -l)
  // 4.add_image_directory_entry_section(0) export..import..
  // 5.set_address_of_export
  // 6.resolve_import
  // 7.resolve_image_directory_entry_section
  // 
  // --- incremental linking
  // you can resolve some pairs of symbols without waiting arrival of all object files.

  init();
  size_t* p1 = read_cmdline(argc, argv);
  void* q;
  info_on_coff c;
  for (;*p1;p1++) {
    printf("%s\n",*p1);
    q = alloc_obj((void*)*p1);
    read_coff(q, &c);
  }
  void* edata;
  void* isection = add_section(".idata");
  if (ExportDirectoryLen)
    edata = add_section(".edata");
  set_virtual_address();
  do_reloc(&resolve_on_export_obj);  
  add_import(isection);
  if (ExportDirectoryLen)  
    add_export(edata);  
  /* gen();   */
  return 1;
}


