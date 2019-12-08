
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

void add_import(SymbolChain* _sc) {
  IMAGE_SECTION_HEADER* import_section = _sc->p;
  void* import_data_begin = __malloc(ImportDirectoryLen);
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
  uint8_t* c = iid;
  printf("ok!\n");
  IMAGE_THUNK_DATA* iat = c;
  // 1st loop is for list of dll.
  for (e=InitialImport;e;e=e->next) {
    printf("ok!\n");
    // 2nd loop is for each funtion entry for IAT.
    for (s1=e->this;s1;s1=s1->next) {
      iat->u1.AddressOfData = import_section->VirtualAddress;
      iat++;
    }
  }
  printf("ok!\n");

  c = iat;
  iat = iid; // back to the begining of IAT
  iid = import_data_begin; // back to the beginning of IID
  for (e=InitialImport;e;e=e->next,iid++,iat++) {
    printf("DLL:%s\n",e->name);
    strcpy(c, e->name);    
    // DLL_NAME
    iid->Name += c - begin;
    c += strlen(c);
    // INT
    iid->OriginalFirstThunk += c - begin;    
    for (s1=e->this;s1;s1=s1->next) {
      printf("f:%s\n", s1->name);
      // IAT
      iat->u1.AddressOfData += c - begin;
      strcpy(c, s1->name);
      c += 2 + strlen(c);
    }
    iid->FirstThunk += (uint8_t*)iat - begin;
  }
}


