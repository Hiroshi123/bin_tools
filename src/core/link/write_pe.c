
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

extern uint8_t TotalSectionNum;
extern void* OutputFileName;
extern SectionChain* InitialSection;

static uint32_t CurrentFileOffset = 0;
static void* VirtualAddressOffset = 0;

uint32_t TotalImageSize = 0;
uint32_t TotalHeaderSize = 0;

void* write_section(SectionChain* sc, HANDLE handle) {
  IMAGE_SECTION_HEADER* sec = sc->p;
  SectionChain* s = sc;
  uint32_t size = 0;
  for (;s;s=s->this) {
    size += s->p->SizeOfRawData;
  }
  sec->Misc.VirtualSize = size;
  // sec->VirtualAddress = VirtualAddressOffset;
  // VirtualAddressOffset += (sec->Misc.VirtualSize + SECTION_ALIGNMENT) & 0xFFFFF000;
  // VirtualSize == 0 wont work out.
  if (!sec->Misc.VirtualSize)
    sec->Misc.VirtualSize = 1;
  // int original = sec->PointerToRawData;
  // CurrentFileOffset = (CurrentFileOffset + FILE_ALIGNMENT - 1) & CurrentFileOffset;
  /* sec->PointerToRawData = CurrentFileOffset; */
  /* CurrentFileOffset += (sec->SizeOfRawData + FILE_ALIGNMENT - 1) & (0 - FILE_ALIGNMENT); */
  sec->PointerToRelocations = 0;
  sec->PointerToLinenumbers = 0;
  sec->NumberOfRelocations = 0;
  sec->NumberOfLinenumbers = 0;
  /* sec->Characteristics = 0xE0000080; */
  DWORD dwWriteSize;
  WriteFile(handle ,sec ,sizeof(IMAGE_SECTION_HEADER) ,&dwWriteSize , NULL);
  // sec->PointerToRawData = original;
  return sec;
}

void write_dos_header(HANDLE handle) {
  
  IMAGE_DOS_HEADER* dos_header = __malloc(sizeof(IMAGE_DOS_HEADER));
  dos_header->e_magic = 0x5a4d;
  dos_header->e_cblp = 0xff;
  dos_header->e_lfarlc = 0x60;
  dos_header->e_lfanew = 0x40;
  DWORD dwWriteSize;
  WriteFile(handle , dos_header, sizeof(IMAGE_DOS_HEADER), &dwWriteSize, NULL);
}

void write_nt_header_signature(HANDLE handle) {
  int size = 4;
  DWORD dwWriteSize;
  DWORD nt_header_signature = 0x4550;
  WriteFile(handle , &nt_header_signature , size , &dwWriteSize , NULL); 
}

void write_image_file_header(HANDLE handle) {

  IMAGE_FILE_HEADER* file_header = __malloc(sizeof(IMAGE_FILE_HEADER));
  /* memset(file_header, 0, size); */
  file_header->Machine = 0x8664;
  file_header->NumberOfSections = TotalSectionNum;
  file_header->TimeDateStamp = 0;
  file_header->PointerToSymbolTable = 0;
  file_header->NumberOfSymbols = 0;
  file_header->SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
  file_header->Characteristics = 0x22f;
  DWORD dwWriteSize;
  WriteFile(handle , file_header, sizeof(IMAGE_FILE_HEADER), &dwWriteSize, NULL);
}

void set_image_directory_entry(IMAGE_DATA_DIRECTORY* d) {

  SectionChain* s = InitialSection;
  SectionChain* t;
  for (;s;s=s->next) {
    for (t=s->this;t;t=t->this) {
      printf("t:%p,%p\n", s,t);
      if (!strcmp(t->p->Name, ".edata")) {
	printf("edata:%p\n",d);
	d->VirtualAddress = t->p->VirtualAddress;
	if (t->p->Misc.VirtualSize == 0)
	  d->Size = t->p->SizeOfRawData;
	else
	  d->Size = t->p->Misc.VirtualSize;
      }
      if (!strcmp(t->p->Name, ".idata")) {
	printf("idata:%p\n",d);
	(d+1)->VirtualAddress = t->p->VirtualAddress;
	if (t->p->Misc.VirtualSize == 0)
	  (d+1)->Size = t->p->SizeOfRawData;
	else
	  (d+1)->Size = t->p->Misc.VirtualSize;	  
      }
      printf("!!!!!!!!!!!!%s,%p\n", t->p->Name, t->p->VirtualAddress);
    }
    printf("out\n");
  }
}

void write_optional_header(HANDLE handle) {

  int size = sizeof(IMAGE_OPTIONAL_HEADER64);
  IMAGE_OPTIONAL_HEADER64* optional_header = __malloc(size);
  memset(optional_header, 0, size);
  // 0c10b or 0x20b
  optional_header->Magic = 0x20b;
  // values which could be anything...
  optional_header->MajorLinkerVersion = 0;
  optional_header->MinorLinkerVersion = 0;
  // these 3 needs to be correct.
  optional_header->SizeOfCode = 0;
  optional_header->SizeOfInitializedData = 0;
  optional_header->SizeOfUninitializedData = 0;
  // this is often set as 0x1000  
  optional_header->BaseOfCode = 0x1000;
  // 
  optional_header->AddressOfEntryPoint = 0x1000;
  
  ///////////////////////////////////////////////////////
  
  optional_header->MajorOperatingSystemVersion = 0;
  optional_header->MinorOperatingSystemVersion = 0;
  optional_header->MajorImageVersion = 0;
  optional_header->MinorImageVersion = 0;
  // subsystem needs 
  optional_header->MajorSubsystemVersion = 5;
  optional_header->MinorSubsystemVersion = 2;
  // 
  optional_header->ImageBase = 0x400000;// 0x000;
  // 
  TotalHeaderSize = sizeof(IMAGE_DOS_HEADER) + 4 + sizeof(IMAGE_FILE_HEADER)
    + sizeof(IMAGE_OPTIONAL_HEADER64) + sizeof(IMAGE_SECTION_HEADER) * TotalSectionNum;
  VirtualAddressOffset = ((TotalHeaderSize + SECTION_ALIGNMENT) & 0xFFFFF000);
  TotalImageSize += VirtualAddressOffset;
  optional_header->SizeOfImage = TotalImageSize;
  optional_header->SizeOfHeaders = TotalHeaderSize;
  // 
  optional_header->SectionAlignment = SECTION_ALIGNMENT;
  optional_header->FileAlignment = FILE_ALIGNMENT;
  // DLL characterstics which is probably most important
  optional_header->DllCharacteristics = 0;
  optional_header->Subsystem = 3;
  
  optional_header->SizeOfStackReserve;
  optional_header->SizeOfStackCommit;
  optional_header->SizeOfHeapReserve;
  optional_header->SizeOfHeapCommit;
  
  optional_header->LoaderFlags = 0;
  // optional_header->BaseOfData = 0;
  optional_header->NumberOfRvaAndSizes = 0x10;
  set_image_directory_entry(&optional_header->DataDirectory);
  DWORD dwWriteSize;
  WriteFile(handle, optional_header, sizeof(IMAGE_OPTIONAL_HEADER), &dwWriteSize, NULL);
}

void write_sections(HANDLE handle) {
  CurrentFileOffset = SetFilePointer(handle, 0, NULL, FILE_CURRENT);
  CurrentFileOffset += TotalSectionNum * sizeof(IMAGE_SECTION_HEADER);
  printf("curp:%p,%p,%p\n", CurrentFileOffset,CurrentFileOffset + FILE_ALIGNMENT - 1,
	 FILE_ALIGNMENT);
  CurrentFileOffset = (CurrentFileOffset + FILE_ALIGNMENT - 1) & (0 - FILE_ALIGNMENT);
  printf("curp:%p\n", CurrentFileOffset);
  SectionChain* s = InitialSection;
  for (;s;s = s->next) {
    // sec = s->this->p;
    // printf("!!%p,%p,%s\n", s, s->this->p, sec->Name);
    write_section(s->this, handle);
  }
}

void write_raw_data(HANDLE handle) {
  // Actual Data is going to be fed.
  SectionChain* s = InitialSection;
  SectionChain* s1;
  IMAGE_SECTION_HEADER* sec;
  DWORD dwWriteSize;
  for (;s;s = s->next) {
    sec = s->this->p;
    printf("!!pointer to raw,%p,%p\n", sec->PointerToRawData, sec->SizeOfRawData);
    if (sec->SizeOfRawData) {
      DWORD fp = SetFilePointer(handle, 0, NULL, FILE_CURRENT);
      printf("%p,%p,%p,%s\n",fp, sec->PointerToRawData,sec->PointerToRawData - fp, sec->Name);
      SetFilePointer(handle, sec->PointerToRawData - fp, NULL, FILE_CURRENT);
      for (s1=s->this;s1;s1=s1->this) {
	WriteFile(handle ,s1->data ,s1->p->SizeOfRawData ,&dwWriteSize , NULL);
      }
    }
  }
}

void gen() {

  char* dummy_name = OutputFileName;
  HANDLE hFile = CreateFile
    (
     dummy_name , GENERIC_ALL/* | GENERIC_EXECUTE*/, 0, NULL,
     CREATE_ALWAYS, 0/*FILE_SHARE_READ*/, NULL
     );
  write_dos_header(hFile);
  write_nt_header_signature(hFile);
  write_image_file_header(hFile);
  write_optional_header(hFile);
  write_sections(hFile);
  write_raw_data(hFile);
  CloseHandle(hFile);
  // read_section_list();
}


