
/* #include <windows.h> */
#include <stdint.h>
#include <stdio.h>

#include "alloc.h"

/* #include "coff.h" */
#include "link.h"
#include "objformat.h"
#include "pe.h"

static SymbolChain3* InitialImport = 0;
static SymbolChain3* CurrentImport = &InitialImport;

extern Config* Confp;

static uint32_t swap_uint32(uint32_t val) {
  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
  return (val << 16) | (val >> 16);
}

void* add_dynamic_resolved_entry(char* name, char* dllname, void* addr) {
  SymbolChain3* e = InitialImport;
  SymbolChain3* s1;
  uint8_t onlyFuncEntry = 0;
  for (; e; e = e->next) {
    // if you add only entry on an existing dll, then you just need to add the
    // entry.
    if (!strcmp(e->name, dllname)) {
      for (s1 = e->this; s1->next; s1 = s1->next)
        ;
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
    // logger_emit("only func\n");
    s1->next = fentry;
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

static void* iterate_import(uint8_t* /*IMAGE_THUNK_DATA*/ iant,
                            uint32_t vaddr) {
  SymbolChain3* e = InitialImport;
  SymbolChain3* s1;
  for (e = InitialImport; e; e = e->next) {
    // 2nd loop is for each funtion entry for IAT.
    for (s1 = e->this; s1; s1 = s1->next) {
      // import_section->VirtualAddress
      if (Confp->file_format == COFF_I386) {
        *(uint32_t*)iant = vaddr;
        iant += 4;
      } else {
        *(uint64_t*)iant = vaddr;
        iant += 8;
      }
    }
    iant += (Confp->file_format == COFF_I386) ? 4 : 8;
  }
  return iant;  //_Win32 ? iant+4 : iant+8;
}

void add_import() {
  if (Confp->import_directory_len == 0) {
    return;
  }

  uint32_t plt_vaddr = Confp->virtual_address_offset;
  int plt_size = Confp->plt_offset - Confp->virtual_address_offset;
  Confp->virtual_address_offset += plt_size;
  uint32_t import_vaddr = Confp->virtual_address_offset;
  Confp->virtual_address_offset += Confp->import_directory_len;

  uint8_t* pltp = __malloc(plt_size);
  void* import_data_begin = __malloc(Confp->import_directory_len);

  SectionContainer* plt_sec_con =
      add_coff_section(".plt", plt_vaddr, plt_size, pltp);
  SectionContainer* idata_sec_con = add_coff_section(
      ".idata", import_vaddr, Confp->import_directory_len, import_data_begin);

  uint8_t* begin = import_data_begin;
  IMAGE_IMPORT_DESCRIPTOR* iid = import_data_begin;
  // this thunk will point on the head of an array named image thunk data
  // (where each of them has a pointer to image import by name)
  SymbolChain3* e = InitialImport;
  SymbolChain3* s1;
  for (; e; e = e->next) {
    // OriginalFirstThunk(INT),FirstThunk(IAT),Name(DLLNAME) should be
    // initialized as section virtual address
    iid->u.OriginalFirstThunk = import_vaddr;  // import_section->VirtualAddress;
    iid->TimeDateStamp = 0;
    iid->ForwarderChain = 0;
    iid->Name = import_vaddr;        // import_section->VirtualAddress;
    iid->FirstThunk = import_vaddr;  // import_section->VirtualAddress;
    iid++;
  }
  /*IMAGE_THUNK_DATA**/
  uint8_t* iat = iid + 1;
  iid = begin;
  /*IMAGE_THUNK_DATA**/
  uint8_t* _int = iterate_import(iat, import_vaddr);
  uint8_t* c = iterate_import(_int, import_vaddr);
  /* // 1st IAT */
  /* iid->FirstThunk += (uint8_t*)iat - begin; */
  /* // 1st INT */
  /* iid->OriginalFirstThunk += (uint8_t*)_int - begin; */

  for (e = InitialImport; e; e = e->next, iid++) {
    // 1st IAT
    iid->FirstThunk += (uint8_t*)iat - begin;
    // 1st INT
    iid->u.OriginalFirstThunk += (uint8_t*)_int - begin;

    // printf("DLL:%s\n",e->name);
    strcpy(c, e->name);
    // DLL_NAME
    iid->Name += c - begin;
    c += strlen(c);
    *c = 0;
    c++;
    // import_section->VirtualAddress + ;
    // 1 ImageImportDirectory can contain multiple pairs of
    // ImageThunkData/ImageImportByName.
    /* iid->FirstThunk += c - begin; */
    uint32_t v;
    // printf("iat:%p,int:%p\n",iat, _int);
    for (s1 = e->this; s1; s1 = s1->next) {
      v = import_vaddr + ((uint8_t*)iat - begin);
      *(uint16_t*)pltp = 0x25ff;
      pltp += 2;
      // pltp->code = 0x25ff;
      // pltp->data = swap_uint32(v - plt_vaddr);
      // ImageThunkData
      // printf("iat:%p,int:%p\n",iat, _int);
      if (Confp->file_format == COFF_I386) {
        // On x32, it should be absolute address.
        v += Confp->base_address;
        *(uint32_t*)pltp = v;  // - plt_vaddr;
        // size of image_thunk_data defers.
        *(uint32_t*)iat += c - begin;
        *(uint32_t*)_int += c - begin;
        iat += 4;
        _int += 4;
      } else {
        // on 64bit, relative address should be set.
        plt_vaddr += 6;
        *(uint32_t*)pltp = v - plt_vaddr;
        *(uint64_t*)iat += c - begin;
        *(uint64_t*)_int += c - begin;
        iat += 8;
        _int += 8;
      }
      pltp += 4;
      // iat->u1.AddressOfData += c - begin;
      // _int->u1.AddressOfData += c - begin;
      // ImageImportByName(Hint(2byte)+function name)
      *(uint16_t*)c = 0;
      c += 2;
      strcpy(c, s1->name);
      c += strlen(c);
      *c = 0;
      c++;
    }
    iat += (Confp->file_format == COFF_I386) ? 4 : 8;
    _int += (Confp->file_format == COFF_I386) ? 4 : 8;
  }
}
