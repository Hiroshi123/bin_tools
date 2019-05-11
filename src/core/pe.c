
#include "pe.h"
#include "memory.h"
#include "objformat.h"
#include <stdlib.h>

// this must be set as 32bit as always guest address is represented as 32bit.
static uint32_t CURRENT_MODULE_TAIL;
/* static uint32_t CURRENT_MODULE_BASE; */
/* static uint32_t CURRENT_IMAGE_DIRECTORY_HEAD; */

const char check_pe(const uint16_t* p) {
  
  return *p == 0x5a4d;
}

uint64_t map_pe32(IMAGE_DOS_HEADER* p_dos_header, void* image_base, uint64_t dll_name_addr) {
  
  IMAGE_NT_HEADERS32* nt_header32 = (IMAGE_NT_HEADERS32*)((uint8_t*)p_dos_header + p_dos_header->e_lfanew);
  printf("!%x,%d\n",&nt_header32->OptionalHeader.Magic,sizeof(nt_header32));
  return 0;
}

uint64_t map_pe64(IMAGE_DOS_HEADER* p_dos_header, void* image_base, uint64_t dll_name_addr) {
  
  IMAGE_NT_HEADERS64* nt_header = (IMAGE_NT_HEADERS64*)((uint8_t*)p_dos_header + p_dos_header->e_lfanew);
  if (!image_base) {
    image_base = nt_header->OptionalHeader.ImageBase;
  }
#ifndef DEBUG
  printf("--------------------------------------------------------\n");
  printf("number of sections: %d\n", nt_header->FileHeader.NumberOfSections);
  printf("number of symbols: %d\n", nt_header->FileHeader.NumberOfSymbols);  
  printf("base of code: 0x%x\n", nt_header->OptionalHeader.BaseOfCode);
  printf("size of code: 0x%x\n", nt_header->OptionalHeader.SizeOfCode);
  printf("address of entry point: %x\n" ,nt_header->OptionalHeader.AddressOfEntryPoint);
  printf("size of initialized data:%d\n", nt_header->OptionalHeader.SizeOfUninitializedData);
  printf("size of image:%d\n", nt_header->OptionalHeader.SizeOfImage);
  uint8_t data_directory_num = 0;  
  for (;data_directory_num<16;data_directory_num++) {
    printf("---------------Optional Header-------------------\n");
    printf("%d,VirtualAddress:%x\tSize:%d\n",
	   data_directory_num,
	   nt_header->OptionalHeader.DataDirectory[data_directory_num].VirtualAddress,
	   nt_header->OptionalHeader.DataDirectory[data_directory_num].Size);
  }
#endif
  
  IMAGE_SECTION_HEADER* s = (IMAGE_SECTION_HEADER*)(nt_header + 1);
  heap* h1;
  uint32_t map_size = ((0 + 0x1000) & 0xfffff000);
  h1 = guest_mmap(image_base,map_size,1,dll_name_addr);
  memcpy
    (h1->begin,
     p_dos_header,
     (uint64_t)s - (uint64_t)p_dos_header
     );
#ifndef DEBUG
  printf("---------------image section-------------------\n");
  printf("characteristics:%x\n", s->Characteristics);
  printf("vaddr(+image base):%lx\n", image_base);
  printf("page addr:%lx\n",h1->begin);
  printf("page num:%d\n", h1->page_num);
  printf("-----------------------------------------------\n");
  
#endif
  // mmap section header.
  int sec_num = nt_header->FileHeader.NumberOfSections;
  IMAGE_SECTION_HEADER* end = s + sec_num;
  heap* h2;
  for (;s<end;s++) {
    map_size = ((s->SizeOfRawData + 0x1000) & 0xfffff000);
    // guest map also have to have some flags...
    h2 = guest_mmap(image_base + s->VirtualAddress, map_size ,0 ,h1 );
    void* dst = memcpy
      (h2->begin,
       (uint8_t*)p_dos_header + s->PointerToRawData,
       s->SizeOfRawData);
    if (image_base + s->VirtualAddress + s->SizeOfRawData
	> CURRENT_MODULE_TAIL) {
      CURRENT_MODULE_TAIL = image_base + s->VirtualAddress +
	s->SizeOfRawData;
    }
#ifndef DEBUG
    printf("---------------section info-----------------------\n");
    printf("name:%s\n", s->Name);
    printf("reloc:%x\n", s->NumberOfRelocations);
    printf("reloc:%x\n", s->PointerToRelocations);
    printf("ptr:%x\n", s->PointerToRawData);
    printf("size:%x\n", s->SizeOfRawData);
    printf("vaddr:%x\n", s->VirtualAddress);
    printf("vaddr(+image base):%lx\n", image_base + s->VirtualAddress);
    printf("characteristics:%x\n", s->Characteristics);
    printf("page addr:%lx\n",h2->begin);
    printf("page num:%d\n", h2->page_num);
    printf("-----------------------------------------------\n");

#endif
    if (!strcmp(s->Name,".rdata")) {
      printf("rdata\n");
      printf("%x\n",h2->begin);      
    }
  }
  return (uint64_t) image_base;
}

// look for edata section from subject image base.
// [image base] -> guest(or host) of edata section, VirtualAddress

// this function will assume 
void* map_pe_for_check_export
(
 uint64_t dll_name_addr,
 void* head,
 void** virtual_address_offset,
 void** image_base
 ) {
  
  IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)head;
  uint16_t lfanew = dos_header->e_lfanew;
  uint16_t* v = (uint16_t*)((uint8_t*)dos_header + dos_header->e_lfanew + 0x18);
  uint64_t guest_image_base;
  void** vv;
  nt_header v_;
  void* guest_export_addr;
  uint64_t next_map = ((CURRENT_MODULE_TAIL + 0x1000) & 0xfffff000);    
  if (*v == 0x10b) {
    guest_image_base = map_pe32(dos_header, next_map, dll_name_addr);
    get_diff_host_guest_addr_(guest_image_base, &vv);
    v_.nt_header32 = (IMAGE_NT_HEADERS32*)((uint8_t*)v - 0x18);
    guest_export_addr =
      guest_image_base
      + v_.nt_header32->OptionalHeader.DataDirectory[0].VirtualAddress;
    *virtual_address_offset = v_.nt_header64->OptionalHeader.DataDirectory[0].VirtualAddress;
    *image_base = guest_image_base;

  } else if (*v == 0x20b) {
    guest_image_base = map_pe64(dos_header, next_map, dll_name_addr);
    get_diff_host_guest_addr_(guest_image_base, &vv);
    v_.nt_header64 = (IMAGE_NT_HEADERS64*)((uint8_t*)vv + lfanew);
    guest_export_addr =
      guest_image_base
      + v_.nt_header64->OptionalHeader.DataDirectory[0].VirtualAddress;
    *virtual_address_offset = v_.nt_header64->OptionalHeader.DataDirectory[0].VirtualAddress;
    *image_base = guest_image_base;
  } else {
    printf("error\n");
    return 0;
  }
  void** host_edata_addr;
  get_diff_host_guest_addr_(guest_export_addr, &host_edata_addr);  
  return host_edata_addr;
}

// when you need to resolve where the function is on a dll,
// you should use this function.
// Provide following arugments.
// 1. virtualAddress of export directory
// 2. image base
// 3. edata host address
// 4. query sequence of characters

// if export table directory contains entry which will be forwarded,
// return address will refer the string which contans characters of DLL and function having . in between the two.
// if not forwarded, it will refer the head of address of the function.

// you can set the value on import address table of caller dll or exe.
// it should also be provided as guest address as it is wierd if every rest of address is represented as guest address
// but if the only IAT holds host address.

p_guest find_f_from_export_directory
(
 uint32_t edata_virtual_address_offset,
 uint64_t image_base,
 void* edata_addr,
 char* query,
 char* forward
 ) {

  IMAGE_EXPORT_DIRECTORY* ied = (IMAGE_EXPORT_DIRECTORY*)edata_addr;  
  // [CAUTION:: higher 32 bit needs to be correct for accessing memory!!!!!]
  uint64_t module_base_for_edata_section =
    (uint64_t)&ied->Characteristics - edata_virtual_address_offset;
  
  uint32_t* addr_name =
    module_base_for_edata_section + ied->AddressOfNames;
  uint32_t* addr_name_end = addr_name + ied->NumberOfNames;
  uint32_t* addr_function =
    module_base_for_edata_section + ied->AddressOfFunctions;
  uint32_t* addr_function_end = addr_function + ied->NumberOfFunctions;
  uint16_t* addr_ordinal =
    module_base_for_edata_section + ied->AddressOfNameOrdinals;
  uint32_t count = 0;
  void** vv;
  void** v1;
  void** v2;
  for (;addr_name!=addr_name_end;addr_name++,count++) {
    /* printf("!%s\n",query); */
    /* printf("!!%lx,%lx,%lx\n",edata_virtual_address_offset, */
    /* 	   module_base_for_edata_section,module_base_for_edata_section + *addr_name); */
    if (!strcmp(query, module_base_for_edata_section + *addr_name)) {
      
      p_guest r = image_base + *(addr_function + *(addr_ordinal + count));      
      /* printf("match:%d,%d,%x,%s\n", */
      /* 	     count, */
      /* 	     *(addr_ordinal + count), */
      /* 	     *(addr_function + *(addr_ordinal + count)), */
      /* 	     /\* (uint64_t)module_base_for_code_section & 0xffffffff, *\/ */
      /* 	     (uint64_t*)vv */
      /* 	     //module_base_for_code_section + *(addr_function + *(addr_ordinal + count)) */
      /* 	     ); */
      get_host_head(r ,&v1);
      get_host_head_from_host(&ied->Characteristics ,&v2);      
      *forward = (v1 == v2) ? 1 : 0;
      return r;
    }
  }
}

char* tolowers(char* t) {
  char* t1 = t;
  for (;*t1!=0;t1++) *t1 = tolower(*t1);   
  return t1;
}
// before mapping idata section or rdata which are going to substitute where idata sits,
// 
void iterate_import_directory(uint64_t module_base_for_idata_section, void* iid_addr) {
  
  IMAGE_IMPORT_DESCRIPTOR* iid = (IMAGE_IMPORT_DESCRIPTOR*)iid_addr;
  char* filename = module_base_for_idata_section + iid->Name;
  // for case insensitivity.
  tolowers(filename);
  
#ifndef DEBUG
  printf("------------image import directory--------------\n");
  printf("image import descriptor head addr:%lx\n",&iid->u.OriginalFirstThunk);
  printf("int head addr:%x\n",iid->u.OriginalFirstThunk);
  printf("time date stamp:%x\n",iid->TimeDateStamp);
  printf("name:%s\n",filename);
  printf("iat head addr:%x\n",iid->FirstThunk);
  printf("-------------------------------------------------\n");
#endif
  
  // image thunk data is 8byte and used for describing
  // struct of an entry of import name table & import address table.
  uint64_t* int_entry = module_base_for_idata_section + iid->u.OriginalFirstThunk;
  uint64_t* iat_entry = module_base_for_idata_section + iid->FirstThunk;
  heap * h = init_map_file(filename);
  uint8_t res = detect_format((uint8_t*)h->begin);
  if (res != PE) {
    printf("format error\n");
    return;
  }
  void** edata_virtual_address_offset;
  void** image_base;
  void* edata_addr =
    map_pe_for_check_export
    (filename,
     h->begin,
     &edata_virtual_address_offset,
     &image_base);
  
  char first = 1;
  for (;*int_entry;int_entry++, iat_entry++) {
    printf("%x,%s\n",int_entry,module_base_for_idata_section + *int_entry + 2);
    char forward;
    // you need to add HINT which is 2byte on image import by name struct
    void* query = module_base_for_idata_section + *int_entry + 2;
    if (first) {
      p_guest r = find_f_from_export_directory
	(
	 edata_virtual_address_offset,
	 image_base,
	 edata_addr,
	 query,
	 &forward);
      p_host* vv;
      get_diff_host_guest_addr_(image_base + r, &vv);
      if (forward == 1) {
	printf("forward::%lx,%s\n",vv,vv);
      } else {
	printf("found::%lx\n",vv);
      }
      // void* base_addr = (uint64_t)edata_addr - (uint32_t)edata_virtual_address_offset;
      
      /* void** iat_subject_addr;// = (uint8_t*)base_addr + *iat_entry;       */
      /* get_diff_host_guest_addr_(0x80000000 + *iat_entry, &iat_subject_addr); */
      /* // whatever it is set on base + iat, all you just do is to just fill the value */
      /* // on it.[NOTE this is .rdata section..] */
      /* // or just simply iat entry itself which was set on import address table???       */
      /* printf("forward:%x,%x,%x,%x,%lx,%x\n", */
      /* 	     forward, */
      /* 	     *iat_entry, */
      /* 	     iat_entry, */
      /* 	     0x80000000 + *iat_entry, */
      /* 	     iat_subject_addr, */
      /* 	     *iat_subject_addr); */
      
      //first = 0;
      
      // break;
    }
    
    // idata section(virtual address of export directory)
    // module_base_for_edata (considering host/get mapping gap)    
    // find_f_from_export_directory();  
  }
  
  /* int_entry++; */
  /*   for (;*int_entry;int_entry++) {     */
  /*   printf("%x,%s\n",int_entry,pp + *int_entry + 2); */
  /* } */
  
  // 1.get name of dll
  // 2.search a name of function given the dll
  // 3. check if the dll had already been mapped.
  // 4.open
  // 5.map section (at least image section & export entry)
  // 6.search export entry
  // 7.fill the retrived value from export table in import address table accordingly.
  
  // if everything had been done, import resolution had been done.  
  
}

// guest entry point is going to be returned..
uint32_t load_pe(void* head) {
  
  IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)head;
  uint16_t lfanew = dos_header->e_lfanew;
  printf("%x,%x\n",dos_header->e_magic,dos_header->e_lfanew);
  uint16_t* nt_optional_header_head =
    (uint16_t*)((uint8_t*)dos_header + lfanew + 0x18);
  nt_header host_nt_header;
  void* guest_image_base;
  void** tmp;
  void* import_addr;
  uint32_t distance_from_image_base;
  // guest memory should be always represented as 32bit for descrimination from
  // pointer of host address.
  uint32_t start_addr;
  if (*nt_optional_header_head == 0x10b) {
    guest_image_base = map_pe32(dos_header, NULL, 0);
    get_diff_host_guest_addr_(guest_image_base, &tmp);
    host_nt_header.nt_header32 = (IMAGE_NT_HEADERS32*)((uint8_t*)tmp + lfanew);
    import_addr =
      host_nt_header.nt_header32->OptionalHeader.ImageBase
      + host_nt_header.nt_header32->OptionalHeader.DataDirectory[1].VirtualAddress;
    distance_from_image_base =
      host_nt_header.nt_header32->OptionalHeader.DataDirectory[1].VirtualAddress;
    start_addr =
      host_nt_header.nt_header32->OptionalHeader.ImageBase + 
      host_nt_header.nt_header32->OptionalHeader.AddressOfEntryPoint;
    /* CURRENT_IMAGE_DIRECTORY_HEAD = 0xffffffff & */
    /*   host_nt_header.nt_header32->OptionalHeader.DataDirectory[0].VirtualAddress;     */
  } else if (*nt_optional_header_head == 0x20b) {
    guest_image_base = map_pe64(dos_header, NULL, 0);
    get_diff_host_guest_addr_(guest_image_base, &tmp);
    host_nt_header.nt_header64 = (IMAGE_NT_HEADERS64*)((uint8_t*)tmp + lfanew);
    import_addr =
      host_nt_header.nt_header64->OptionalHeader.ImageBase
      + host_nt_header.nt_header64->OptionalHeader.DataDirectory[1].VirtualAddress;
    distance_from_image_base =
      host_nt_header.nt_header64->OptionalHeader.DataDirectory[1].VirtualAddress;
    start_addr =
      host_nt_header.nt_header64->OptionalHeader.ImageBase + 
      host_nt_header.nt_header64->OptionalHeader.AddressOfEntryPoint;
    
    printf("addr!!!!!%x\n",&host_nt_header.nt_header64->OptionalHeader.DataDirectory[1].VirtualAddress);
    /* CURRENT_IMAGE_DIRECTORY_HEAD = 0xffffffff & */
    /*   host_nt_header.nt_header64->OptionalHeader.DataDirectory[0].VirtualAddress; */
  } else {
    printf("error\n");
    return 0;
  }
  /* uint64_t idata_section_head_addr = 0; */
  uint64_t* idata_section_head;// = idata_section_head_addr;
  /* get_diff_host_guest_addr_(import_addr, &idata_section_head); */
  /* idata_section_head += sizeof(IMAGE_IMPORT_DESCRIPTOR); */
  
  get_diff_host_guest_addr_((uint8_t*)import_addr, &idata_section_head);
  uint64_t module_base_for_idata_section =
    (uint8_t*)idata_section_head - distance_from_image_base;
  
  iterate_import_directory
    (module_base_for_idata_section,
     (uint8_t*)idata_section_head);
  
  /* iterate_import_directory */
  /*   (module_base_for_idata_section, */
  /*    (uint8_t*)idata_section_head + 0x14); */

  /* CURRENT_MODULE_BASE = guest_image_base; */
  
  printf("import:::%x\n",import_addr);

  return start_addr;  
  
  // IMAGE_IMPORT_DESCRIPTOR* iid = (IMAGE_IMPORT_DESCRIPTOR*)idata_section_head;
  
  /* printf("ddd:%lx,%lx,%lx,%lx\n", */
  /* 	 idata_section_head, */
  /* 	 &idata_section_head, */
  /* 	 iid, */
  /* 	 &iid); */
  
  /* iid++; */
  /* printf("ddd:%lx,%lx,%lx,%lx\n", */
  /* 	 idata_section_head, */
  /* 	 &idata_section_head, */
  /* 	 iid, */
  /* 	 &iid); */

  /* /\* *iid++; *\/ */
  /* iterate_import_directory(distance_from_image_base, iid); */
  
  //uint8_t* module_base_for_idata_section = (uint8_t*)idata_section_head - distance_from_image_base;
  
}

p_host get_image_directory_head(p_host head) {
  IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)head;
  uint16_t lfanew = dos_header->e_lfanew;
  printf("!%x,%x\n",dos_header->e_magic,dos_header->e_lfanew);
  uint16_t* nt_optional_header_head =
    (uint16_t*)((uint8_t*)dos_header + lfanew + 0x18);
  p_host idh_addr;
  if (*nt_optional_header_head == 0x10b) {
    idh_addr = (uint8_t*)nt_optional_header_head + 0x60;
  } else if (*nt_optional_header_head == 0x20b) {
    idh_addr = (uint8_t*)nt_optional_header_head + 0x70;
  }
  printf("%x,%x,%x\n",dos_header,nt_optional_header_head,idh_addr);
  return idh_addr;
}

// you need to provide a guest address which tells you if it is within current IAT.
char _check_on_iat(p_guest rip, p_guest query) {
  
  // get actual value of virtual address of import section on host address
  p_host* v_addr;
  heap* h = get_parent_heap_from_guest(rip);  
  p_host ide = get_image_directory_head(h->begin);
  // leap 0x08 as first one is export directory
  uint32_t iid_v_addr = *(uint32_t*)(ide + 0x08);
  uint32_t size = *(uint32_t*)(ide + 0x08 + 0x04);
  uint32_t iat_head = h->guest_addr + iid_v_addr;
  uint32_t iat_tail = iat_head + size;
  return (iat_head <= query && query <= iat_tail) ? 1 : 0;
}

// from given 

p_host get_dll_name(p_guest v_addr, p_host idata_section, p_host f_name) {

  p_host module_base_for_idata_section = idata_section - v_addr;
  printf("aa:%lx,%s,%d\n",f_name,f_name,strlen(f_name));
  /* for case insensitivity. */
  /* tolowers(filename); */
  
  IMAGE_IMPORT_DESCRIPTOR* iid = (IMAGE_IMPORT_DESCRIPTOR*)idata_section;
  
  for (;iid->u.OriginalFirstThunk;iid++) {
    p_host dll_name = (module_base_for_idata_section + iid->Name);
    uint64_t* int_entry = module_base_for_idata_section + iid->u.OriginalFirstThunk;
    uint64_t* iat_entry = module_base_for_idata_section + iid->FirstThunk;  
    for (;*int_entry;int_entry++, iat_entry++) {
      p_host n = module_base_for_idata_section + *int_entry + 2;
      if (!memcmp(f_name,n,strlen(n)-1)) {
	return dll_name;
      }
      printf("%x,%s,%d\n",int_entry,module_base_for_idata_section + *int_entry + 2,
	     strlen(module_base_for_idata_section + *int_entry + 2)
	     );
    }
    printf("%s\n",dll_name);
  }  
  return NULL;//f_name;
}

p_host _find_f_addr(p_guest rip, p_guest query) {

  p_host* v_addr;
  heap* h1 = get_parent_heap_from_guest(rip);  
  p_host ide = get_image_directory_head(h1->begin);
  uint32_t iid_v_addr = *(uint32_t*)(ide + 0x08);
  
  p_host* idata_section;
  get_diff_host_guest_addr_(h1->guest_addr + iid_v_addr, &idata_section);
  
  // if the region specifies INT, you get the function name.
  p_host* tmp;
  p_host* f_name;
  get_diff_host_guest_addr_(query, &tmp);
  get_diff_host_guest_addr_(h1->guest_addr + *(uint64_t*)tmp + 2, &f_name);
  
  printf("%x,%x,%x,%s\n",tmp, *(uint64_t*)tmp, 0x400000 + *(uint64_t*)tmp,(uint8_t*)f_name);
  
  // you also need to reach the dll name.
  p_host dll_name = get_dll_name(iid_v_addr, idata_section, f_name);
  
  // After reaching dll, you should search the corresponding page among ever mapped page given the dll name
  
  heap* h2 = search_page_by_name(dll_name);
  if (!h2) {
    printf("error\n");
  }
  printf("%x\n",h2->begin);
  uint32_t* ied = (uint32_t*)get_image_directory_head(h2->begin);

  /* IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)h->guest_addr; */
  /* uint16_t* nt_optional_header_head = */
  /*   (uint16_t*)((uint8_t*)dos_header + dos_header->e_lfanew + 0x18);   */
  /* uint32_t v_addr; */
  /* if (*nt_optional_header_head == 0x10) { */
  /*   IMAGE_NT_HEADERS32* tmp = (IMAGE_NT_HEADERS32*)nt_optional_header_head; */
  /*   v_addr = tmp->OptionalHeader.DataDirectory[0].VirtualAddress; */
  /* } else if (*nt_optional_header_head == 0x20) { */
  /*   IMAGE_NT_HEADERS64* tmp = (IMAGE_NT_HEADERS64*)nt_optional_header_head; */
  /*   v_addr = tmp->OptionalHeader.DataDirectory[0].VirtualAddress; */
  /* } */
  
  /* p_host get_image_directory_head(p_host head); */
  
  p_host* edata_addr;
  get_diff_host_guest_addr_((uint8_t*)h2->guest_addr + *ied, &edata_addr);
  
  char* forward;
  p_host r = find_f_from_export_directory(*ied, h2->guest_addr, edata_addr, f_name, &forward);
  printf("r:%x\n",r);
  return r;
}


