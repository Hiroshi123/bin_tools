
#include "pe.h"
#include "macro.h"
#include "memory.h"
#include "os.h"
#include "alloc.h"

#include "objformat.h"
#include <fcntl.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
  char* base;
  int fd;
} dso;


static void* alloc_vaddress(void* imagebase, int size_of_image);

// this must be set as 32bit as always guest address is represented as 32bit.
/* static uint32_t CURRENT_MODULE_TAIL; */
extern p_guest EXPORT(current_page_tail);
/* static uint32_t CURRENT_MODULE_BASE; */
/* static uint32_t CURRENT_IMAGE_DIRECTORY_HEAD; */

uint64_t map_pe
(
 p_host p_dos_header, p_host section_head, uint32_t section_num,
 p_guest image_base, uint64_t dll_name_addr) {
  IMAGE_SECTION_HEADER* s = (IMAGE_SECTION_HEADER*)section_head;
  heap* h1;
  uint32_t map_size = ((0 + 0x1000) & 0xfffff000);
  h1 = guest_mmap(image_base,map_size,1,dll_name_addr,-1);
  uint32_t len = (uint32_t)s - (uint32_t)p_dos_header;
  memcpy
    (h1->begin,
     p_dos_header,
     len
     );
#ifndef DEBUG
  printf("---------------image section-------------------\n");
  printf("characteristics:%x\n", s->Characteristics);
  printf("vaddr(+image base):%lx\n", image_base);
  printf("page addr:%lx\n",h1->begin);
  printf("page num:%d\n", h1->page_num);
  printf("-----------------------------------------------\n");
  
#endif
  IMAGE_SECTION_HEADER* section_end = s + section_num;
  heap* h2;
  for (;s<section_end;s++) {
    map_size = ((s->SizeOfRawData + 0x1000) & 0xfffff000);
    // guest map also have to have some flags...
    h2 = guest_mmap(image_base + s->VirtualAddress, map_size ,0 ,h1, -1);
    void* dst = memcpy
      (h2->begin,
       (uint8_t*)p_dos_header + s->PointerToRawData,
       s->SizeOfRawData);
    if (image_base + s->VirtualAddress + s->SizeOfRawData
	> EXPORT(current_page_tail)) {
      EXPORT(current_page_tail) = image_base + s->VirtualAddress +
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

p_host retrive_nt_header_32(IMAGE_DOS_HEADER* p_dos_header, p_guest* image_base, uint32_t* sec_num) {

  IMAGE_NT_HEADERS32* nt_header = (IMAGE_NT_HEADERS32*)((uint8_t*)p_dos_header + p_dos_header->e_lfanew);
  if (!image_base) {
    image_base = nt_header->OptionalHeader.ImageBase;
  }
  // mmap section header.
  *sec_num = nt_header->FileHeader.NumberOfSections;
  
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
  // should return section head of guest address.
  return nt_header+1;
}

p_host retrive_nt_header_64(IMAGE_DOS_HEADER* p_dos_header, p_guest* image_base, uint32_t* sec_num) {
  
  IMAGE_NT_HEADERS64* nt_header = (IMAGE_NT_HEADERS64*)((uint8_t*)p_dos_header + p_dos_header->e_lfanew);
  if (image_base) {
    *image_base = nt_header->OptionalHeader.ImageBase;
  }
  
  *sec_num = nt_header->FileHeader.NumberOfSections;
  printf("aaa\n");
  /* *section_head = (nt_header+1); */
  printf("aaa\n");
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
  printf("!%lx\n",nt_header+1);
  return nt_header+1;//nt_header->OptionalHeader.ImageBase;
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
  
  void** host_image_base;
  nt_header v_;
  void* guest_export_addr;
  p_host section_head = 0;
  uint32_t sec_num;
  p_guest guest_image_base = ((EXPORT(current_page_tail) + 0x1000) & 0xfffff000);    

  if (*v == 0x10b) {
    section_head = retrive_nt_header_32(dos_header, NULL, &sec_num);
    map_pe(dos_header, section_head, sec_num, guest_image_base, dll_name_addr);
    get_diff_host_guest_addr_(guest_image_base, &host_image_base);
    v_.nt_header32 = (IMAGE_NT_HEADERS32*)((uint8_t*)v - 0x18);
    guest_export_addr =
      guest_image_base
      + v_.nt_header32->OptionalHeader.DataDirectory[0].VirtualAddress;
    *virtual_address_offset = v_.nt_header64->OptionalHeader.DataDirectory[0].VirtualAddress;
    *image_base = guest_image_base;

  } else if (*v == 0x20b) {
    section_head = retrive_nt_header_64(dos_header ,NULL ,&sec_num);
    map_pe(dos_header, section_head, sec_num, guest_image_base, dll_name_addr);
    get_diff_host_guest_addr_(guest_image_base, &host_image_base);
    v_.nt_header64 = (IMAGE_NT_HEADERS64*)((uint8_t*)host_image_base + lfanew);
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
  void* v1;
  void** v2;
  for (;addr_name!=addr_name_end;addr_name++,count++) {
    /* printf("!%s\n",query); */
    /* printf("!!%lx,%lx,%lx\n",edata_virtual_address_offset, */
    /* 	   module_base_for_edata_section,module_base_for_edata_section + *addr_name); */
    if (!strcmp(query, module_base_for_edata_section + *addr_name)) {
      
      p_guest r = *(addr_function + *(addr_ordinal + count));      
      /* printf("match:%d,%d,%x,%s\n", */
      /* 	     count, */
      /* 	     *(addr_ordinal + count), */
      /* 	     *(addr_function + *(addr_ordinal + count)), */
      /* 	     /\* (uint64_t)module_base_for_code_section & 0xffffffff, *\/ */
      /* 	     (uint64_t*)vv */
      /* 	     //module_base_for_code_section + *(addr_function + *(addr_ordinal + count)) */
      /* 	     ); */
      v1 = get_host_head(r);
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
  int fd = open(filename, O_RDONLY);
  uint32_t h_size;
  enum OBJECT_FORMAT res = detect_format(fd, &h_size);
  struct stat stbuf;
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    return;
  }
  heap * h = map_file(fd, stbuf.st_size, -1);
  if (res != PE64) {
    printf("format error\n");
    return;
  }
  void** edata_virtual_address_offset;
  p_host image_base;
  void* edata_addr =
    map_pe_for_check_export
    (filename,
     h->begin,
     &edata_virtual_address_offset,
     &image_base);
  printf("base:%lx,%lx,%lx\n",image_base, edata_virtual_address_offset, edata_addr);
  char first = 1;
  for (;*int_entry;int_entry++, iat_entry++) {
    printf("!!!%x,%s\n",int_entry,module_base_for_idata_section + *int_entry + 2);
    char forward;
    // you need to add HINT which is 2byte on image import by name struct
    void* query = module_base_for_idata_section + *int_entry + 2;
    if (first) {
      p_guest r = find_f_from_export_directory
	(
	 edata_virtual_address_offset,
	 edata_addr,
	 query,
	 &forward);
      p_host* vv;
      printf("r:%x\n",r);
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

void* __z__obj__read_nt_header(char* nt_optional_header_head) {
  // IMAGE_NT_HEADERS32*
  IMAGE_OPTIONAL_HEADER32* opt = nt_optional_header_head;
  IMAGE_OPTIONAL_HEADER32* opt2;
  char* ret;
  if (opt->Magic == 0x10b) {
    opt2 = (char*)opt;
  }
  else if (opt->Magic == 0x20b) {
    opt2 = (char*)opt + 0x10;
  }
  void* v = alloc_vaddress(opt->ImageBase, opt->SizeOfImage);
  logger_emit_p(v);  
  logger_emit_p(opt2->DataDirectory[1].VirtualAddress);
  logger_emit_p(opt2->DataDirectory[1].Size);
  return v;
}

static int strlen(char* p) {
  int i;
  for (i=0;*p;p++,i++);
  return i;
}

static void alloc_each_section(void* arg1, void* arg2, void* arg3) {

  /* IMAGE_FILE_HEADER* ifh = base; */
  /* char* str = (char*)base + ifh->PointerToSymbolTable; */
  /* str += ifh->NumberOfSymbols * sizeof(IMAGE_SYMBOL);   */
  
  IMAGE_SECTION_HEADER* isec = arg1;  
  char* sh_name = isec->Name;
  logger_emit("misc.log","----\n");
  logger_emit("misc.log", sh_name);
  logger_emit("misc.log", "\n");
  logger_emit_p(isec->PointerToRawData);
  logger_emit_p(isec->SizeOfRawData);
  
  dso* _dso = arg3;
  logger_emit_p(_dso->fd);
  logger_emit_p(_dso->base + isec->VirtualAddress);
  logger_emit_p(isec->SizeOfRawData);
  
  int size = isec->SizeOfRawData;
  if (size) {
    void* v = 0;
    int fd = _dso->fd;
    __os__lseek(fd, isec->PointerToRawData, SEEK_SET);
    int r = __os__read(fd, _dso->base + isec->VirtualAddress, size);
    if (r != size) {
      __os__write(1, "error\n", 6);
    }
    logger_emit_p(r);    
    // logger_emit_p(v);
  }
}

static void* alloc_vaddress(void* imagebase, int size_of_image) {
  
  void* v = __os__mmap
    (imagebase, size_of_image
     /*PAGE_SIZE*/, PROT_READ | PROT_WRITE | PROT_EXEC,
     MAP_SHARED | MAP_ANON, -1, 0);
  if (v == -1) {
    __os__write(1, "error\n", 6);
  }
  return v;
}

static void do_image_directory_export(void* arg1, void* arg2) {

  IMAGE_DATA_DIRECTORY* id = arg1;
  dso* _dso = arg2;
  IMAGE_EXPORT_DIRECTORY* ied = (IMAGE_EXPORT_DIRECTORY*)(_dso->base + id->VirtualAddress);
  
  logger_emit("misc.log", _dso->base + ied->Name);
  logger_emit_p(ied->NumberOfFunctions);
  
  // [CAUTION:: higher 32 bit needs to be correct for accessing memory!!!!!]
  
  /* uint64_t module_base_for_edata_section = */
  /*   (uint64_t)&ied->Characteristics - edata_virtual_address_offset; */
  
  uint32_t* addr_name =
    _dso->base + ied->AddressOfNames;
  uint32_t* addr_name_end = addr_name + ied->NumberOfNames;  
  uint32_t* addr_function =
   _dso->base + ied->AddressOfFunctions;
  uint32_t* addr_function_end =
    addr_function + ied->NumberOfFunctions;  
  uint16_t* addr_ordinal =
    _dso->base + ied->AddressOfNameOrdinals;
  
  uint32_t count = 0;
  /* void** vv; */
  /* void* v1; */
  /* void** v2; */
  /* for (;addr_name!=addr_name_end;addr_name++,count++) { */
  for (;addr_name!=addr_name_end;addr_name++,count++) {
    char* s = _dso->base + *addr_name;
    __os__write(1, s, strlen(s));
  }
}

static void do_image_directory_import(void* arg1, void* arg2) {
  IMAGE_DATA_DIRECTORY* id = arg1;
  dso* _dso = arg2;
  logger_emit("misc.log", "---\n");
  logger_emit_p(id->VirtualAddress);
  logger_emit_p(_dso->base + id->VirtualAddress);
  logger_emit_p(id->Size);
  
  IMAGE_IMPORT_DESCRIPTOR* iid = (IMAGE_IMPORT_DESCRIPTOR*)(_dso->base + id->VirtualAddress);
  char* filename = _dso->base + iid->Name;
  // for case insensitivity.
  logger_emit("misc.log", filename);
  logger_emit("misc.log", "\n");
  // image thunk data is 8byte and used for describing
  // struct of an entry of import name table & import address table.
  uint64_t* int_entry = _dso->base + iid->u.OriginalFirstThunk;
  uint64_t* iat_entry = _dso->base + iid->FirstThunk;
  // tolowers(filename);
  
  int fd = __os__open("./win/ntdll.dll", O_RDONLY, 0777);
  enum OBJECT_FORMAT format = __z__obj__detect_format(fd, 0);
  logger_emit_p(fd);
  logger_emit_p(format);
  __os__close(fd);

  // logger_emit_p("misc.log", fd);
  
/* #ifndef DEBUG */
/*   printf("------------image import directory--------------\n"); */
/*   printf("image import descriptor head addr:%lx\n",&iid->u.OriginalFirstThunk); */
/*   printf("int head addr:%x\n",iid->u.OriginalFirstThunk); */
/*   printf("time date stamp:%x\n",iid->TimeDateStamp); */
/*   printf("name:%s\n",filename); */
/*   printf("iat head addr:%x\n",iid->FirstThunk); */
/*   printf("-------------------------------------------------\n"); */
/* #endif */
  
}

// guest entry point is going to be returned..
char* __z__obj__load_pe(char* name, void* out) {

  info_on_pe* info = out;
  int fd = __os__open(name, O_RDONLY, 0777);
  if (fd == -1) {
    char str[] = "loadlibrary error\n";
    __os__write(1, str, sizeof(str));
  }
  enum OBJECT_FORMAT format = __z__obj__detect_format(fd, 0);
  if (format != PE64) {
    __os__write(1, "error\n", 6);
    return 0;
  }
  void* head = __os__mmap
    (NULL, 0x1000
     /*PAGE_SIZE*/, PROT_READ | PROT_WRITE | PROT_EXEC,
     MAP_PRIVATE, fd, 0);
  
  IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)head;
  uint16_t lfanew = dos_header->e_lfanew;
  // printf("%x,%x\n",dos_header->e_magic,dos_header->e_lfanew);
  uint32_t* signature = ((uint8_t*)dos_header) + lfanew;
  if (*signature != 0x4550) {    
    __os__write(1, "error\n", 6);
  }
  IMAGE_FILE_HEADER* ifh = signature + 1;
  uint16_t* nt_optional_header_head =
    (uint16_t*)(ifh + 0x1);
  IMAGE_OPTIONAL_HEADER32* opt = nt_optional_header_head;
  IMAGE_OPTIONAL_HEADER32* opt2 = opt;

  // meanwhile, consider only x86.
  if (1) {
    info->image_base = *(uint64_t*)&opt->BaseOfData;
    logger_emit_p(info->image_base);
  } else {
    info->image_base = opt->ImageBase;
  }
  if (*nt_optional_header_head == 0x10b) {    
  } else if (*nt_optional_header_head == 0x20b) {
    opt2 = (uint8_t*)opt2 + 0x10;
  }
  logger_emit_p(opt->BaseOfData);
  logger_emit_p(opt->ImageBase);
  logger_emit_p(opt->SectionAlignment);
  
  nt_header host_nt_header;
  p_guest guest_image_base;
  void** tmp;
  void* import_addr;
  uint32_t distance_from_image_base;
  // guest memory should be always represented as 32bit for descrimination from
  // pointer of host address.
  uint32_t sec_num;
  p_host section_head;
  __os__write(1, "ok\n", 3);
  logger_emit("misc.log", "ok\n");

  void* v = __z__obj__read_nt_header(nt_optional_header_head);
  dso* _dso = __malloc(sizeof(dso));
  _dso->base = v;
  _dso->fd = fd;
  info->this_image_base = _dso->base;
  info->fd = fd;  
  __z__obj__run_through_coff_shdr3(ifh, &alloc_each_section, _dso);
  
  do_image_directory_export((void*)&opt2->DataDirectory[0], _dso);
  // do_image_directory_import((void*)&opt2->DataDirectory[1], _dso);
  
  __os__close(fd);
  char* start_addr = _dso->base + opt->AddressOfEntryPoint;
  return start_addr;
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
char EXPORT(check_on_iat(p_guest rip, p_guest query)) {
  
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

p_host EXPORT(find_f_addr(p_guest rip, p_guest query)) {

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
  uint32_t* ied = (uint32_t*)get_image_directory_head(h2->begin);
  p_host* edata_addr;
  get_diff_host_guest_addr_((uint8_t*)h2->guest_addr + *ied, &edata_addr);  
  char* forward;
  p_host r = find_f_from_export_directory(*ied, edata_addr, f_name, &forward);
  printf("r:%x\n",r);
  return r;
}


