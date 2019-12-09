
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

static SymbolChain3* InitialImport = 0;
static SymbolChain3* CurrentImport = &InitialImport;
uint32_t ImportDirectoryLen;

void* add_dynamic_resolved_entry(char* name, char* dllname, void* addr) {
  SymbolChain3* e = InitialImport;
  SymbolChain3* s1;
  uint8_t onlyFuncEntry = 0;
  for (;e;e=e->next) {
    // if you add only entry on an existing dll, then you just need to add the entry.
    if (!strcmp(e->name,dllname)) {
      for (s1=e->this;s1;s1=s1->next);
      // assumes s1 == NULL at this point
      onlyFuncEntry = 1;
    }
  }
  SymbolChain3* fentry;
  // add entry for the function.
  fentry = __malloc(sizeof(SymbolChain3));
  // this will be used to store address of the import function.
  fentry->next = 0;
  fentry->this = addr;
  fentry->name = name;
  if (onlyFuncEntry) {
    s1 = fentry;
    return 0;
  }
  // if nothing is resolved yet, then add the new function with the new dllname
  SymbolChain3* dllentry;
  dllentry = __malloc(sizeof(SymbolChain3));
  dllentry->next = 0;
  dllentry->this = fentry;
  dllentry->name = dllname;
  if (InitialImport == 0) {
    InitialImport = dllentry;
  } else {
    CurrentImport->next = dllentry;
  }
  CurrentImport = dllentry;
  return 1;
}

void* iterate_import(IMAGE_THUNK_DATA* iant, uint32_t vaddr) {
  SymbolChain3* e = InitialImport;
  SymbolChain3* s1;
  for (e=InitialImport;e;e=e->next) {
    // 2nd loop is for each funtion entry for IAT.
    for (s1=e->this;s1;s1=s1->next) {
      // import_section->VirtualAddress
      iant->u1.AddressOfData = vaddr;
      iant++;
      printf("ok!!\n");
    }
  }
  return iant+1;
}

void add_import(SectionChain* _sc) {
  IMAGE_SECTION_HEADER* import_section = _sc->p;
  void* import_data_begin = __malloc(ImportDirectoryLen);
  if (import_section->SizeOfRawData == 0)
    import_section->SizeOfRawData = ImportDirectoryLen;
  _sc->data = import_data_begin;
  uint8_t* begin = import_data_begin;
  IMAGE_IMPORT_DESCRIPTOR* iid = import_data_begin;
  // this thunk will point on the head of an array named image thunk data
  // (where each of them has a pointer to image import by name)
  SymbolChain3* e = InitialImport;
  SymbolChain3* s1;
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
  // uint8_t* c = iid+1;
  IMAGE_THUNK_DATA* iat = iid+1;
  iid = begin;
  IMAGE_THUNK_DATA* _int = iterate_import(iat, import_section->VirtualAddress);
  uint8_t* c = iterate_import(_int, import_section->VirtualAddress);  
  // 1st IAT
  iid->OriginalFirstThunk += (uint8_t*)iat - begin;
  // 1st INT
  iid->FirstThunk += (uint8_t*)_int - begin;
  for (e=InitialImport;e;e=e->next,iid++) {
    printf("DLL:%s\n",e->name);
    strcpy(c, e->name);
    // DLL_NAME
    iid->Name += c - begin;
    c += strlen(c);
    *c = 0;
    c++;
    // 1 ImageImportDirectory can contain multiple pairs of
    // ImageThunkData/ImageImportByName.
    /* iid->FirstThunk += c - begin; */
    for (s1=e->this;s1;s1=s1->next,iat++,_int++) {
      printf("f:%s\n", s1->name);
      // ImageThunkData
      iat->u1.AddressOfData += c - begin;
      _int->u1.AddressOfData += c - begin;
      // ImageImportByName(Hint(2byte)+function name)
      *(uint16_t*)c = 0;
      c+=2;
      strcpy(c, s1->name);
      c += strlen(c);
      *c = 0;
      c++;
    }
  }
}



