
/* #include <windows.h> */
#include <stdio.h>
#include <stdint.h>
#include "alloc.h"

#include "pe.h"
#include "link.h"

extern Config* Confp;

void add_export(/*SectionChain* _sec*/) {
  
  SectionContainer* scon = match_section(".edata");
  if (scon == 0) return;
  SectionChain* _sec = scon->this;
  IMAGE_SECTION_HEADER* sec = _sec->p;
  if (sec->SizeOfRawData == 0)
    sec->SizeOfRawData = Confp->export_directory_len;
  void* export_data_p = __malloc(Confp->export_directory_len);
  // Confp->export_data_p = export_data_p;
  void* begin = export_data_p;
  _sec->data = export_data_p;
  IMAGE_EXPORT_DIRECTORY* ied = export_data_p;
  Confp->dynsym_head = ied;
  ied->Characteristics = 0;
  ied->TimeDateStamp = 0;
  ied->MajorVersion = 0;
  ied->MinorVersion = 0;
  ied->Name = sec->VirtualAddress;
  ied->Base = 0;
  ied->NumberOfFunctions = Confp->export_func_count;
  ied->NumberOfNames = Confp->export_func_count;
  // added later on along with actual allocation
  ied->AddressOfFunctions = sec->VirtualAddress;
  ied->AddressOfNames = sec->VirtualAddress;
  ied->AddressOfNameOrdinals = sec->VirtualAddress;
  logger_emit_p(sec->VirtualAddress);
  ObjectChain* oc;
  SymbolChain* sc;
  IMAGE_SYMBOL* is;
  char* name;
  char* image_name = ied+1;
  strcpy(image_name, Confp->outfile_name);  
  ied->Name += (void*)image_name - begin;

  // this is local(in this process) pointer
  // do not confuse with virtual address
  WORD* addressOfNameOrdinals = image_name + strlen(image_name) + 1;
  DWORD* addressOfFunctions = addressOfNameOrdinals + Confp->export_func_count;
  DWORD* addressOfNames = addressOfFunctions + Confp->export_func_count;
  
  // pe_export_data export_data = {};
  
  ied->AddressOfNameOrdinals += (void*)addressOfNameOrdinals - begin;
  ied->AddressOfFunctions += (void*)addressOfFunctions - begin;
  ied->AddressOfNames += (void*)addressOfNames - begin;

  // this is also local pointer to string table
  void* c = addressOfNames + Confp->export_func_count;

  Confp->export_data.ied_p = ied;
  Confp->export_data.address_of_name_ordinals = addressOfNameOrdinals;
  Confp->export_data.address_of_functions = addressOfFunctions;
  Confp->export_data.address_of_names = addressOfNames;
  Confp->export_data.str_p = c;
  Confp->export_data.vaddr_p = sec->VirtualAddress;
  
  DWORD export_addr;
  uint32_t i = 0;
  
  if (export_data_p)
  for (oc=Confp->initial_object;oc;oc=oc->next) {
    sc = oc->symbol_chain_head;
    // printf("!aa,%p,%d,%p\n", ied,ExportDirectoryLen,sec);
    for (;sc;sc=sc->next) {
      is = sc->p;
      name = GET_NAME(is, oc->str_table_p);
      *addressOfNames = sec->VirtualAddress + (c - begin);
      // export_addr = get_export_virtual_address(is, oc);
      export_addr = sc->schain->virtual_address + is->Value;
      *addressOfFunctions = export_addr;
      *addressOfNameOrdinals = i;
      if (Confp->use_dt_hash) {
	add_dt_hash_entry(Confp->hash_table_p, sc->name, i);
	// printf("!%s,%p,%d,%p\n",name, export_addr, i, sc->schain);
      }
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


