
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

extern ObjectChain* InitialObject;

uint32_t ExportFuncCount = 0;
uint32_t ExportDirectoryLen = 0;

void add_export(SectionChain* _sec) {
  IMAGE_SECTION_HEADER* sec = _sec->p;
  // char* image_name = "a01.exe";
  void* export_data_p = __malloc
    (ExportDirectoryLen + sizeof(IMAGE_EXPORT_DIRECTORY) + strlen("a01.exe"));
  void* export_data_begin = export_data_p;
  IMAGE_EXPORT_DIRECTORY* ied = export_data_p;
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
  for (oc=InitialObject;oc;oc=oc->next) {
    sc = oc->symbol_chain_head;
    printf("!aa,%p,%d,%p\n", ied,ExportDirectoryLen,sec);
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


