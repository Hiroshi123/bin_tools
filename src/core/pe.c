
#include "pe.h"
#include "memory.h"


const char check_pe(const uint16_t* p) {
  
  return *p == 0x5a4d;
}

char load_pe(const char *const begin,void* stack_addr
	     /*info_on_pe *e*/) {
  
  IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)begin;
  printf("%x,%x\n",dos_header->e_magic,dos_header->e_lfanew);
  uint16_t* v = (uint16_t*)((uint8_t*)dos_header + dos_header->e_lfanew + 0x18);
  printf("%x,%x\n",v,*v);
  if (*v == 0x10b) {
    load_pe32(dos_header, stack_addr);
  } else if (*v == 0x20b) {
    load_pe64(dos_header, stack_addr);
  }
}

void load_pe32(IMAGE_DOS_HEADER* p_dos_header, void* stack_addr) {
  
  IMAGE_NT_HEADERS32* nt_header32 = (IMAGE_NT_HEADERS32*)((uint8_t*)p_dos_header + p_dos_header->e_lfanew);
  printf("!%x,%d\n",&nt_header32->OptionalHeader.Magic,sizeof(nt_header32));
  
}

void load_pe64(IMAGE_DOS_HEADER* p_dos_header, void* stack_addr) {
  
  IMAGE_NT_HEADERS64* nt_header = (IMAGE_NT_HEADERS64*)((uint8_t*)p_dos_header + p_dos_header->e_lfanew);
  printf("!!%x,%x\n",nt_header->OptionalHeader.BaseOfCode ,nt_header->OptionalHeader.AddressOfEntryPoint);
  
  printf("num sec %d\n", nt_header->FileHeader.NumberOfSections);
  printf("num sec %d\n", nt_header->FileHeader.NumberOfSymbols);
  
  printf("%x\n", nt_header->OptionalHeader.BaseOfCode);
  
  printf("%x\n", nt_header->OptionalHeader.SizeOfCode);
  printf("%d\n", nt_header->OptionalHeader.SizeOfUninitializedData);
  printf("%d\n", nt_header->OptionalHeader.SizeOfImage);
  
  printf("0,%d\n", nt_header->OptionalHeader.DataDirectory[0].Size);
  printf("1,%d\n", nt_header->OptionalHeader.DataDirectory[1].Size);
  printf("2,%d\n", nt_header->OptionalHeader.DataDirectory[2].Size);
  printf("3,%d\n", nt_header->OptionalHeader.DataDirectory[3].Size);
  printf("4,%d\n", nt_header->OptionalHeader.DataDirectory[4].Size);
  printf("5,%d\n", nt_header->OptionalHeader.DataDirectory[5].Size);
  printf("6,%d\n", nt_header->OptionalHeader.DataDirectory[6].Size);
  printf("7,%d\n", nt_header->OptionalHeader.DataDirectory[7].Size);
  printf("8,%d\n", nt_header->OptionalHeader.DataDirectory[8].Size);
  printf("9,%d\n", nt_header->OptionalHeader.DataDirectory[9].Size);
  printf("10,%d\n", nt_header->OptionalHeader.DataDirectory[10].Size);
  printf("11,%d\n", nt_header->OptionalHeader.DataDirectory[11].Size);
  printf("12,%d\n", nt_header->OptionalHeader.DataDirectory[12].Size);
  printf("13,%d\n", nt_header->OptionalHeader.DataDirectory[13].Size);
  printf("14,%d\n", nt_header->OptionalHeader.DataDirectory[14].Size);
  
  /* uint64_t* a = ((uint8_t*)(p_dos_header)) /\*+ nt_header->OptionalHeader.ImageBase*\/ */
  /*   + nt_header->OptionalHeader.BaseOfCode;  */
  
  IMAGE_SECTION_HEADER* s = (IMAGE_SECTION_HEADER*)(nt_header + 1);
  
  /* IMAGE_SECTION_HEADER* s = e->sec_begin; */
  heap* h;
  
  // mapping image header(dos header + file feader + optional header) itself
  uint32_t map_size = ((0 + 0x1000) & 0xfffff000);  
  h = guest_mmap(nt_header->OptionalHeader.ImageBase,map_size);
  memcpy
    (h->begin,
     p_dos_header,
     (uint64_t)s - (uint64_t)p_dos_header
     );
  // mmap section header.
  int sec_num = nt_header->FileHeader.NumberOfSections;  
  IMAGE_SECTION_HEADER* end = s + sec_num;
  for (;s!=end;s++) {
    map_size = ((s->VirtualAddress + 0x1000) & 0xfffff000);
    // guest map also have to have some flags...
    h = guest_mmap(nt_header->OptionalHeader.ImageBase + s->VirtualAddress, map_size);
    printf("p:%x,%x\n",h->begin, h->page_num);
    void* dst = memcpy
      (h->begin,
       (uint8_t*)p_dos_header + s->PointerToRawData,
       s->SizeOfRawData);
    printf("dst:%x\n",dst);
    /* printf("%x,%x\n",h,h->begin); */    
    // memcpy(s->VirtualAddress, );
    // s->PointerToRawData    
    printf("!%x\n",s);
    printf("%s\n",s->Name);
    printf("%x\n", s->NumberOfRelocations);
    printf("%x\n", s->PointerToRelocations);
    printf("%x\n", s->PointerToRawData);
    printf("%x\n", s->SizeOfRawData);
    printf("%x\n", s->VirtualAddress);
    printf("%x\n", s->Characteristics);
    printf("---\n");
  }
  
  // stack Reserved
  void* stack_head = (uint64_t)stack_addr & 0xfffff000;
  map_size = ((0 + 0x1000) & 0xfffff000);  
  h = guest_mmap(stack_head, map_size); 
  // heap Reserve
  void* start_addr = nt_header->OptionalHeader.ImageBase + nt_header->OptionalHeader.AddressOfEntryPoint;
  return start_addr;
}


// prepare host/get address translation mapping.
// guest (page1) 0x40000000 -> host current address();
// guest (page2) 0x

// offset calculation
// 0x40000000
// 0x40001000
// 

