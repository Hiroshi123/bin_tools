
#include "pe.h"
#include "memory.h"
#include "objformat.h"
#include <stdlib.h>

// this must be set as 32bit as always guest address is represented as 32bit.
static uint32_t CURRENT_MODULE_TAIL;

const char check_pe(const uint16_t* p) {
  
  return *p == 0x5a4d;
}

uint64_t map_pe32(IMAGE_DOS_HEADER* p_dos_header, void* image_base) {
  
  IMAGE_NT_HEADERS32* nt_header32 = (IMAGE_NT_HEADERS32*)((uint8_t*)p_dos_header + p_dos_header->e_lfanew);
  printf("!%x,%d\n",&nt_header32->OptionalHeader.Magic,sizeof(nt_header32));
  return 0;
}

uint64_t map_pe64(IMAGE_DOS_HEADER* p_dos_header, void* image_base) {
  
  IMAGE_NT_HEADERS64* nt_header = (IMAGE_NT_HEADERS64*)((uint8_t*)p_dos_header + p_dos_header->e_lfanew);
  if (!image_base) {
    image_base = nt_header->OptionalHeader.ImageBase;
  }
#ifndef DEBUG
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
  /* uint64_t* a = ((uint8_t*)(p_dos_header)) /\*+ nt_header->OptionalHeader.ImageBase*\/ */
  /*   + nt_header->OptionalHeader.BaseOfCode;  */
  
  IMAGE_SECTION_HEADER* s = (IMAGE_SECTION_HEADER*)(nt_header + 1);
  
  /* IMAGE_SECTION_HEADER* s = e->sec_begin; */
  heap* h;  
  // mapping image header(dos header + file feader + optional header) itself
  
  uint32_t map_size = ((0 + 0x1000) & 0xfffff000);  
  h = guest_mmap(image_base,map_size);
  memcpy
    (h->begin,
     p_dos_header,
     (uint64_t)s - (uint64_t)p_dos_header
     );
#ifndef DEBUG
  printf("---------------image section-------------------\n");
  printf("characteristics:%x\n", s->Characteristics);
  printf("vaddr(+image base):%lx\n", image_base);
  printf("page addr:%lx\n",h->begin);
  printf("page num:%d\n", h->page_num);
  printf("-----------------------------------------------\n");
  
#endif  
  // mmap section header.
  int sec_num = nt_header->FileHeader.NumberOfSections;  
  IMAGE_SECTION_HEADER* end = s + sec_num;
  for (;s<end;s++) {
    map_size = ((s->SizeOfRawData + 0x1000) & 0xfffff000);
    // guest map also have to have some flags...
    h = guest_mmap(image_base + s->VirtualAddress, map_size);
    void* dst = memcpy
      (h->begin,
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
    printf("page addr:%lx\n",h->begin);
    printf("page num:%d\n", h->page_num);
    printf("-----------------------------------------------\n");

#endif
    
    if (!strcmp(s->Name,".rdata")) {
      printf("rdata\n");
      printf("%x\n",h->begin);      
    }    
  }  
  return (uint64_t) image_base;
}

void* map_pe_for_check_export
(
 void* head,
 void** virtual_address_offset,
 void** image_base
 ) {
  
  IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)head;
  uint16_t lfanew = dos_header->e_lfanew;
  printf("%x,%x\n",dos_header->e_magic,dos_header->e_lfanew);
  uint16_t* v = (uint16_t*)((uint8_t*)dos_header + dos_header->e_lfanew + 0x18);
  printf("%x,%x\n",v,*v);
  uint64_t guest_image_base;
  void** vv;
  nt_header v_;
  void* guest_export_addr;
  uint64_t next_map = ((CURRENT_MODULE_TAIL + 0x1000) & 0xfffff000);    
  printf("guest addr::%x,%x\n", CURRENT_MODULE_TAIL, next_map);
  
  if (*v == 0x10b) {
    guest_image_base = map_pe32(dos_header, next_map);
    get_diff_host_guest_addr_(guest_image_base, &vv);
    v_.nt_header32 = (IMAGE_NT_HEADERS32*)((uint8_t*)v - 0x18);
    guest_export_addr =
      /*v_.nt_header32->OptionalHeader.ImageBase*/
      guest_image_base
      + v_.nt_header32->OptionalHeader.DataDirectory[0].VirtualAddress;
    *virtual_address_offset = v_.nt_header64->OptionalHeader.DataDirectory[0].VirtualAddress;
    *image_base = guest_image_base;

  } else if (*v == 0x20b) {
    guest_image_base = map_pe64(dos_header, next_map);
    get_diff_host_guest_addr_(guest_image_base, &vv);
    v_.nt_header64 = (IMAGE_NT_HEADERS64*)((uint8_t*)vv + lfanew);
    guest_export_addr =
      /*v_.nt_header64->OptionalHeader.ImageBase*/
      guest_image_base
      + v_.nt_header64->OptionalHeader.DataDirectory[0].VirtualAddress;
    /* printf("%lx,%x\n",vv,v_.nt_header64->OptionalHeader.DataDirectory[0].VirtualAddress); */
    *virtual_address_offset = v_.nt_header64->OptionalHeader.DataDirectory[0].VirtualAddress;
    *image_base = guest_image_base;
  } else {
    printf("error\n");
    return 0;
  }
  void** host_edata_addr;
  get_diff_host_guest_addr_(guest_export_addr, &host_edata_addr);  
  /* printf("oo:%x,%x\n",host_edata_addr,*((uint8_t*)host_edata_addr + 5)); */
  /* printf("%x,%x\n",host_edata_addr - edata_distance_from_image_base,edata_distance_from_image_base);   */
  return host_edata_addr;
}


void* find_f_from_export_directory
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
      get_diff_host_guest_addr_
	(image_base + *(addr_function + *(addr_ordinal + count)), &vv);
      
      printf("match:%d,%d,%x,%s\n",
	     count,
	     *(addr_ordinal + count),
	     *(addr_function + *(addr_ordinal + count)),
	     /* (uint64_t)module_base_for_code_section & 0xffffffff, */
	     (uint64_t*)vv
	     //module_base_for_code_section + *(addr_function + *(addr_ordinal + count))
	     );
      get_host_head(image_base + *(addr_function + *(addr_ordinal + count)) ,&v1);
      get_host_head_from_host(&ied->Characteristics ,&v2);      
      printf("%x,%x\n",v1,v2);
      if (v1 == v2) {
	*forward = 1;
      } else {
	*forward = 0;
      }
      return vv;
    }
  }
}

// before mapping idata section or rdata which are going to substitute where idata sits,
// 
void iterate_import_directory(uint64_t module_base_for_idata_section, void* iid_addr) {
  
  IMAGE_IMPORT_DESCRIPTOR* iid = (IMAGE_IMPORT_DESCRIPTOR*)iid_addr;
  
#ifndef DEBUG
  printf("------------image import directory--------------\n");
  printf("image import descriptor head addr:%lx\n",&iid->u.OriginalFirstThunk);
  printf("int head addr:%x\n",iid->u.OriginalFirstThunk);
  printf("time date stamp:%x\n",iid->TimeDateStamp);
  printf("name:%s\n",module_base_for_idata_section + iid->Name);
  printf("iat head addr:%x\n",iid->FirstThunk);
  printf("-------------------------------------------------\n");
#endif
  
  char* filename = module_base_for_idata_section + iid->Name;
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
    (h->begin,
     &edata_virtual_address_offset,
     &image_base);
  
  char first = 1;
  for (;*int_entry;int_entry++, iat_entry++) {
    
    printf("%x,%s\n",int_entry,module_base_for_idata_section + *int_entry + 2);
    char forward;
    // you need to add HINT which is 2byte on image import by name struct
    void* query = module_base_for_idata_section + *int_entry + 2;
    if (first) {
      void* r = find_f_from_export_directory
	(
	 edata_virtual_address_offset,
	 image_base,
	 edata_addr,
	 query,
	 &forward);
      
      // void* base_addr = (uint64_t)edata_addr - (uint32_t)edata_virtual_address_offset;
      
      void** iat_subject_addr;// = (uint8_t*)base_addr + *iat_entry;      
      get_diff_host_guest_addr_(0x80000000 + *iat_entry, &iat_subject_addr);
      // whatever it is set on base + iat, all you just do is to just fill the value
      // on it.[NOTE this is .rdata section..]
      // or just simply iat entry itself which was set on import address table???      
      printf("forward:%x,%x,%x,%x,%lx,%x\n",
	     forward,
	     *iat_entry,
	     iat_entry,
	     0x80000000 + *iat_entry,
	     iat_subject_addr,
	     *iat_subject_addr);
      
      first = 0;
      
      break;
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

uint64_t load_pe(void* head) {
  
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
  if (*nt_optional_header_head == 0x10b) {
    guest_image_base = map_pe32(dos_header, NULL);
    get_diff_host_guest_addr_(guest_image_base, &tmp);
    host_nt_header.nt_header32 = (IMAGE_NT_HEADERS32*)((uint8_t*)tmp + lfanew);
    import_addr =
      host_nt_header.nt_header32->OptionalHeader.ImageBase
      + host_nt_header.nt_header32->OptionalHeader.DataDirectory[1].VirtualAddress;
    distance_from_image_base =
      host_nt_header.nt_header32->OptionalHeader.DataDirectory[1].VirtualAddress;
  } else if (*nt_optional_header_head == 0x20b) {
    guest_image_base = map_pe64(dos_header, NULL);    
    get_diff_host_guest_addr_(guest_image_base, &tmp);
    host_nt_header.nt_header64 = (IMAGE_NT_HEADERS64*)((uint8_t*)tmp + lfanew);
    import_addr =
      host_nt_header.nt_header64->OptionalHeader.ImageBase
      + host_nt_header.nt_header64->OptionalHeader.DataDirectory[1].VirtualAddress;
    distance_from_image_base =
      host_nt_header.nt_header64->OptionalHeader.DataDirectory[1].VirtualAddress;
  } else {
    printf("error\n");
    return 0;
  }
  /* uint64_t idata_section_head_addr = 0; */
  uint64_t* idata_section_head;// = idata_section_head_addr;
  /* get_diff_host_guest_addr_(import_addr, &idata_section_head); */
  /* iterate_import_directory(distance_from_image_base, idata_section_head); */
  /* idata_section_head += sizeof(IMAGE_IMPORT_DESCRIPTOR); */
  get_diff_host_guest_addr_((uint8_t*)import_addr, &idata_section_head);
  uint64_t module_base_for_idata_section =
    (uint8_t*)idata_section_head - distance_from_image_base;  
  iterate_import_directory
    (module_base_for_idata_section,
     (uint8_t*)idata_section_head + 0x14);
  
  printf("%x\n",sizeof(IMAGE_IMPORT_DESCRIPTOR));
  
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


