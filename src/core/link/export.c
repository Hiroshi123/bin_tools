
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

extern ObjectChain* InitialObject;
extern void* OutputFileName;

uint32_t ExportFuncCount = 0;
uint32_t ExportDirectoryLen = 0;

void add_export(SectionChain* _sec) {
  IMAGE_SECTION_HEADER* sec = _sec->p;
  if (sec->SizeOfRawData == 0)
    sec->SizeOfRawData = ExportDirectoryLen;
  void* export_data_p = __malloc(ExportDirectoryLen);
  void* begin = export_data_p;
  _sec->data = export_data_p;
  IMAGE_EXPORT_DIRECTORY* ied = export_data_p;
  ied->Characteristics = 0;
  ied->TimeDateStamp = 0;
  ied->MajorVersion = 0;
  ied->MinorVersion = 0;
  ied->Name = sec->VirtualAddress;
  ied->Base = 0;
  ied->NumberOfFunctions = ExportFuncCount;
  ied->NumberOfNames = ExportFuncCount;
  // added later on along with actual allocation
  ied->AddressOfFunctions = sec->VirtualAddress;
  ied->AddressOfNames = sec->VirtualAddress;
  ied->AddressOfNameOrdinals = sec->VirtualAddress;
  
  ObjectChain* oc;
  SymbolChain* sc;
  IMAGE_SYMBOL* is;
  char* name;
  char* image_name = ied+1;
  strcpy(image_name, OutputFileName);
  ied->Name += (void*)image_name - begin;
  WORD* addressOfNameOrdinals = image_name + strlen(image_name) + 1;
  DWORD* addressOfFunctions = addressOfNameOrdinals + ExportFuncCount;
  DWORD* addressOfNames = addressOfFunctions + ExportFuncCount;
  ied->AddressOfFunctions += (void*)addressOfFunctions - begin;
  ied->AddressOfNames += (void*)addressOfNames - begin;
  ied->AddressOfNameOrdinals += (void*)addressOfNameOrdinals - begin;
  
  void* c = addressOfNames + ExportFuncCount;
  DWORD export_addr;
  uint32_t i = 0;
  if (export_data_p)
  for (oc=InitialObject;oc;oc=oc->next) {
    sc = oc->symbol_chain_head;
    printf("!aa,%p,%d,%p\n", ied,ExportDirectoryLen,sec);
    for (;sc;sc=sc->next) {
      is = sc->p;
      name = GET_NAME(sc->p, oc->str_table_p);
      *addressOfNames = sec->VirtualAddress + (c - begin);
      export_addr = get_export_virtual_address(is, oc);
      *addressOfFunctions = export_addr;
      *addressOfNameOrdinals = i;
      printf("%s,%p,%d\n",name, sc->p, sc->p->StorageClass);
      // should contain some virtual address
      // addressOfNameOrdinals =
      strcpy(c, name);
      c += strlen(name) + 1;
      addressOfFunctions++;
      addressOfNameOrdinals++;
      addressOfNames++;
      i++;      
    }
  }
}


