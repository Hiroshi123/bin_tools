
/* #include <windows.h> */
#include <stdint.h>
#include <stdio.h>

#include "alloc.h"
#include "link.h"
#include "pe.h"

extern callback_arg3(void* arg1, void* arg2, void* f);

extern Config* Confp;

static uint8_t wrapper_f(void* arg1, void* arg2, void* f) {
  return callback_arg3(arg1, arg2, f);
}

static void fill_address(uint32_t* addr, uint16_t type, uint32_t dst_vaddr,
                         uint32_t src_vaddr) {
  switch (type) {
    case IMAGE_REL_AMD64_ADDR64 /*1*/:
      *(uint64_t*)addr += Confp->base_address /*ImageBase*/ + dst_vaddr;
      break;
    case IMAGE_REL_AMD64_ADDR32 /*2*/:
      *addr += Confp->base_address /*ImageBase*/ + dst_vaddr;
      break;
    case IMAGE_REL_AMD64_ADDR32NB /*3*/:
      *addr += dst_vaddr;
      break;
    case IMAGE_REL_AMD64_REL32 /*4*/:
      *addr += dst_vaddr - (src_vaddr + 4);
      break;
    case IMAGE_REL_AMD64_REL32 | IMAGE_REL_AMD64_SSPAN32:
      *addr += dst_vaddr - (src_vaddr + 4);
      break;
    default:
      printf("not supported type\n");
      break;
      // case 5 to 9 is REL32_1 to REL32_5
      // from A to F, they are somehow special.
      // IMAGE_REL_AMD64_SECTION
      // (0x10)IMAGE_REL_AMD64_SSPAN32
  }
}

uint8_t resolve_only_in_a_section(CallbackArgIn* _in, uint32_t* addr) {
  char* name = _in->name;
  uint16_t type = _in->type;
  if (type == IMAGE_REL_AMD64_REL32) {
    ObjectChain* oc = 0;
    IMAGE_SYMBOL* is = lookup_symbol(name, &oc);
    if (is) {
      char* export_section_name = get_section_name(is, oc);
      if (!strcmp(export_section_name, _in->src_section_name)) {
        printf("within a section\n");
        logger_emit("relocation:resolved on another file\n");
        size_t* export_address = get_export_virtual_address(is, oc);
        fill_address(addr, type, export_address, _in->src_vaddr);
        return 1;
      }
    }
  }
  return 0;
}

static int update_import_directory_len(void* new, char* name, char* dllname) {
  int offset = 0;
  if (new) {
    if (Confp->import_directory_len == 0) {
      // if it is 1st ever insertion of IID, you need to set an empty one
      // at the end of it.
      offset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
    }
    offset += sizeof(IMAGE_IMPORT_DESCRIPTOR) + 1 + strlen(dllname);
    // needs empty image_thunk_data at the end of it for indicating its end.
    offset += 2 * sizeof(void* /*IMAGE_THUNK_DATA*/);
    // Confp->import_directory_len += 2 + strlen(name) + 1;
  }
  /* // 1entry = (IAT) + (INT) + HINT + strlen + NULL */
  offset += 2 * sizeof(void* /*IMAGE_THUNK_DATA*/) + 2 + strlen(name) + 1;
  return offset;
}

void __coff_resolve(CallbackArgIn* _in) {
  size_t* addr = _in->filled_addr;
  char* name = _in->name;
  uint16_t type = _in->type;
  uint32_t storage_class = _in->storage_class;
  logger_emit_p(storage_class);
  // 1st, you should check symbols on the same object.
  // If storage class is static, it is likely that ADDR64/ADDR32 which
  // requires to fill virtual address + ImageBase on it directly.
  if (storage_class == 3 /*Static*/) {
    ObjectChain* src_oc = _in->src_oc;
    IMAGE_SECTION_HEADER* ish =
        (IMAGE_SECTION_HEADER*)src_oc->section_head + (_in->shndx - 1);
    if (ish == 0) {
      __os__write(1, "error\n", 6);
      return;
    }
    // __os__write(1, ish->Name, strlen(ish->Name));
    // __os__write(1, "\n", 1);
    logger_emit_p(ish->VirtualAddress);
    logger_emit_p(*_in->filled_addr);
    // *_in->filled_addr = 0;
    // get_section_chain_from_name(name);
    size_t* dst_vaddr = _in->dst_vaddr + ish->VirtualAddress;
    fill_address(_in->filled_addr, type, dst_vaddr, _in->src_vaddr);
    logger_emit("misc.log",
                "relocation:resolved on a same file(StorageClass==3)\n");
    return;
  }
  if (storage_class != 2 /*External*/) {
    printf("not external\n");
    return;
  }
  // printf("?:%s\n", name);
  int dst_vaddr = sysv_lookup_for_coff(name);
  // printf("r:%p\n", dst_vaddr);
  if (dst_vaddr) {
    fill_address(_in->filled_addr, type, dst_vaddr, _in->src_vaddr);
    logger_emit("misc.log", "relocation resolved externally\n");
    return;
  }
  uint32_t ever = 0;
  char* dllname = lookup_dynamic_symbol(name, 1, &ever);
  if (!dllname) {
    fprintf(stdout, "could not resolved..%s \n", name);
    return;
  }
  // printf("dll:%s\n", dllname);

  logger_emit("misc.log", "relocation:resolved on an entry of a dyanmic dll\n");
  // printf("ever:%p\n", ever);
  // PltOffset(0xff 0x25 0x00 0x00 0x00 0x00) - (addr + 1)(just after call)
  if (ever) {
    fill_address(addr, type, ever, _in->src_vaddr);
    // printf("do not need to add new import entry.\n");
    return;
  }
  fill_address(addr, type, Confp->plt_offset, _in->src_vaddr);
  Confp->plt_offset += 6;
  void* new = add_dynamic_resolved_entry(name, dllname, addr);
  Confp->import_directory_len +=
      update_import_directory_len(new, name, dllname);
  /* printf("%s was resolved on %s\n", name, dllname); */
}
