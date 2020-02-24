
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "alloc.h"

#include "coff.h"
#include "link.h"

extern uint8_t TotalSectionNum;
extern void* OutputFileName;
extern SectionChain* InitialSection;

extern SectionChain* EntrySectionChain;
extern uint32_t EntrySectionOffset;
extern uint8_t _Win32;
extern ObjectChain* InitialObject;
extern uint32_t TotalHeaderSize;

static uint32_t CurrentFileOffset = 0;
static uint32_t RelocationHead = 0;

// ImageBase is DWORD(4byte) on 32bit/QWORD on 64bit.
uint64_t ImageBase = 0;
uint32_t TotalImageSize = 0;
uint8_t EmitType = 0;
uint32_t SymbolNum = 0;
uint32_t SymbolTablePointer = 0;

void* write_section(SectionChain* sc, HANDLE handle) {
  IMAGE_SECTION_HEADER* sec = sc->p;
  printf("!!!name:%s\n", sec->Name);
  SectionChain* s = sc;
  uint32_t size = 0;
  for (;s;s=s->this) {
    size += s->p->SizeOfRawData;
  }
  sec->Misc.VirtualSize = size;
  // VirtualSize == 0 wont work out.
  if (!sec->Misc.VirtualSize)
    sec->Misc.VirtualSize = 1;
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

void write_section_for_obj(SectionChain* sc, HANDLE handle) {
  IMAGE_SECTION_HEADER* sec = sc->p;  
  SectionChain* s = sc;
  uint32_t size = 0;
  uint32_t relocation_num = 0;
  for (;s;s=s->this) {
    size += s->p->SizeOfRawData;
    relocation_num += s->p->NumberOfRelocations;
    printf("size up,%d,%d\n", s->p->SizeOfRawData,s->p->NumberOfRelocations);
  }
  uint32_t orig_sizeofrawdata = sec->SizeOfRawData;
  uint32_t orig_numofrel = sec->NumberOfRelocations;
  uint32_t orig_pointertorelo = sec->PointerToRelocations;
  // SizeOfRawData and NumberOfRelocation should be temporarily as they are used
  // as original value when writing raw data.
  sec->SizeOfRawData = size;
  sec->NumberOfRelocations = relocation_num;
  sec->PointerToRawData = CurrentFileOffset;
  sec->PointerToRelocations = CurrentFileOffset + size;
  DWORD dwWriteSize;
  WriteFile(handle ,sec ,sizeof(IMAGE_SECTION_HEADER) ,&dwWriteSize , NULL);
  sec->SizeOfRawData = orig_sizeofrawdata;
  sec->NumberOfRelocations = orig_numofrel;
  sec->PointerToRelocations = orig_pointertorelo;
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
  file_header->Machine = _Win32 ? 0x14c : 0x8664;
  file_header->NumberOfSections = TotalSectionNum;
  file_header->TimeDateStamp = 0;
  if (EmitType == EMIT_OBJ) {
    file_header->PointerToSymbolTable = SymbolTablePointer;
    file_header->NumberOfSymbols = SymbolNum;
    file_header->SizeOfOptionalHeader = 0;
    file_header->Characteristics = 0;
  } else {
    file_header->PointerToSymbolTable = 0;
    file_header->NumberOfSymbols = 0;  
    file_header->Characteristics = _Win32 ? 0x32f : 0x22f;
    file_header->SizeOfOptionalHeader =
      _Win32 ? sizeof(IMAGE_OPTIONAL_HEADER64) - 0x10 : sizeof(IMAGE_OPTIONAL_HEADER64);  
  }
  if (EmitType == EMIT_DLL) {
    file_header->Characteristics |= 0x2000;
  }
  // IMAGE_FILE_32BIT_MACHINE
  DWORD dwWriteSize;
  WriteFile(handle , file_header, sizeof(IMAGE_FILE_HEADER), &dwWriteSize, NULL);
}

void set_image_directory_entry(IMAGE_DATA_DIRECTORY* d) {
  
  SectionChain* s = InitialSection;
  SectionChain* t;
  for (;s;s=s->next) {
    for (t=s->this;t;t=t->this) {
      /* printf("t:%p,%p\n", s,t); */
      if (!strcmp(t->p->Name, ".edata")) {
	/* printf("edata:%p\n",d); */
	d->VirtualAddress = t->p->VirtualAddress;
	if (t->p->Misc.VirtualSize == 0)
	  d->Size = t->p->SizeOfRawData;
	else
	  d->Size = t->p->Misc.VirtualSize;
      }
      if (!strcmp(t->p->Name, ".idata")) {
	/* printf("idata:%p,%p\n",d, t->p->SizeOfRawData); */
	(d+1)->VirtualAddress = t->p->VirtualAddress;
	if (t->p->Misc.VirtualSize == 0)
	  (d+1)->Size = t->p->SizeOfRawData;
	else
	  (d+1)->Size = t->p->Misc.VirtualSize;
      }
      if (!strcmp(t->p->Name, ".rdata")) {
	
      }
      if (!strcmp(t->p->Name, ".tls")) {
	// tls
	// 
      }
    }
  }
}

void write_optional_header(HANDLE handle) {

  int size = sizeof(IMAGE_OPTIONAL_HEADER64);
  IMAGE_OPTIONAL_HEADER64* optional_header = __malloc(size);
  // 0x10b or 0x20b
  optional_header->Magic = _Win32 ? 0x10b : 0x20b;
  // values which could be anything...
  optional_header->MajorLinkerVersion = 0;
  optional_header->MinorLinkerVersion = 0;
  // these 3 needs to be correct.
  optional_header->SizeOfCode = 0;
  optional_header->SizeOfInitializedData = 0;
  optional_header->SizeOfUninitializedData = 0;
  // this is often set as 0x1000
  optional_header->BaseOfCode = 0x1000;
  if (_Win32) {
    // BaseOfData
    DWORD* p = &optional_header->ImageBase;
    *p = 0x1000;
    optional_header->BaseOfCode = 0x1000;
    p++;
    // ImageBase
    *p = ImageBase;
  } else {
    // 
    optional_header->ImageBase = ImageBase;//0x400000;
  }
  /* if (EmitType == EMIT_DLL) { */
  /*   // optional_header->AddressOfEntryPoint = 0;     */
  /* } else */

  if (EntrySectionChain) {
    printf("not come\n");
    optional_header->AddressOfEntryPoint =
      EntrySectionChain->p->VirtualAddress + EntrySectionOffset;
  } else {
    optional_header->AddressOfEntryPoint = 0x1000;
  }
  ///////////////////////////////////////////////////////
  
  optional_header->MajorOperatingSystemVersion = 0;
  optional_header->MinorOperatingSystemVersion = 0;
  optional_header->MajorImageVersion = 0;
  optional_header->MinorImageVersion = 0;
  // subsystem needs 
  optional_header->MajorSubsystemVersion = 5;
  optional_header->MinorSubsystemVersion = 2;
  // 
  /* TotalHeaderSize = sizeof(IMAGE_DOS_HEADER) + 4 + sizeof(IMAGE_FILE_HEADER) */
  /*   + sizeof(IMAGE_OPTIONAL_HEADER64) + sizeof(IMAGE_SECTION_HEADER) * TotalSectionNum; */
  /* if (_Win32) */
  /*   TotalHeaderSize -= 0x10; */
  optional_header->SizeOfImage = TotalImageSize;
  optional_header->SizeOfHeaders = TotalHeaderSize;
  // 
  optional_header->SectionAlignment = SECTION_ALIGNMENT;
  optional_header->FileAlignment = FILE_ALIGNMENT;
  // DLL characterstics which is probably most important
  optional_header->DllCharacteristics = 0;
  optional_header->Subsystem = 3;

  // from 0x48 to (0x58)0x68 is the only difference of optional header between
  // 32bit and 64bit.
  uint8_t* p;// = ((uint8_t*)&optional_header) + 0x48;
  if (_Win32) {
    // NumberOfRvaAndSizes
    uint32_t* _p = ((uint8_t*)optional_header) + 0x5c;
    *_p = 0x10;
    p = _p+1;
  } else {
    uint32_t* _p = ((uint8_t*)optional_header) + 0x6c;
    *_p = 0x10;
    p = _p+1;
  }
  /* optional_header->SizeOfStackReserve; */
  /* optional_header->SizeOfStackCommit; */
  /* optional_header->SizeOfHeapReserve; */
  /* optional_header->SizeOfHeapCommit;   */
  /* optional_header->LoaderFlags = 0; */
  /* optional_header->NumberOfRvaAndSizes = 0x10; */
  set_image_directory_entry(p/*&optional_header->DataDirectory*/);
  DWORD dwWriteSize;
  WriteFile
    (handle, optional_header,
     _Win32 ? sizeof(IMAGE_OPTIONAL_HEADER) - 0x10 : sizeof(IMAGE_OPTIONAL_HEADER),
     &dwWriteSize, NULL);
}

void write_sections(HANDLE handle) {
  CurrentFileOffset = SetFilePointer(handle, 0, NULL, FILE_CURRENT);
  CurrentFileOffset += TotalSectionNum * sizeof(IMAGE_SECTION_HEADER);
  printf("curp:%p,%p,%p\n", CurrentFileOffset,CurrentFileOffset + FILE_ALIGNMENT - 1,
	 FILE_ALIGNMENT);
  if (EmitType != EMIT_OBJ)
    CurrentFileOffset = (CurrentFileOffset + FILE_ALIGNMENT - 1) & (0 - FILE_ALIGNMENT);
  printf("curp:%p\n", CurrentFileOffset);
  SectionChain* s = InitialSection;
  for (;s;s = s->next) {
    // sec = s->this->p;
    // printf("!!%p,%p,%s\n", s, s->this->p, sec->Name);
    if (EmitType == EMIT_OBJ) {
      write_section_for_obj(s->this, handle);
    } else {
      write_section(s->this, handle);
    }
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

void write_raw_data_for_obj(HANDLE handle) {
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
	printf("sizeofraw:%d\n", s1->p->SizeOfRawData);
	WriteFile(handle ,s1->data ,s1->p->SizeOfRawData ,&dwWriteSize , NULL);
      }
    }
    // if you have relocation record on a merging section which comes after the former section,
    // then Offset & SymbolIndex must be different.
    if (sec->NumberOfRelocations) {
      if (RelocationHead == 0)
	RelocationHead = SetFilePointer(handle, 0, NULL, FILE_CURRENT);
      for (s1=s->this;s1;s1=s1->this) {
	SetFilePointer(handle, s1->p->NumberOfRelocations * sizeof(CoffReloc), NULL, FILE_CURRENT);
	/* WriteFile */
	/*   (handle, s1->p->PointerToRelocations, */
	/*    s1->p->NumberOfRelocations * 10 ,&dwWriteSize , NULL); */
      }
    }
  }
}

void write_relocation_record(HANDLE handle) {
  SectionChain* s = InitialSection;
  SectionChain* s1;
  IMAGE_SECTION_HEADER* sec;
  DWORD dwWriteSize;
  DWORD fp = SetFilePointer(handle, RelocationHead, NULL, FILE_BEGIN);
  for (;s;s = s->next) {
    sec = s->this->p;
    if (sec->NumberOfRelocations) {
      uint32_t size_of_code = 0;
      // SetFilePointer(handle, sec->PointerToRelocations - fp, NULL, FILE_);      
      for (s1=s->this;s1;s1=s1->this) {
	printf("write relocation rec,%p\n", s1->obj);
	ObjectChain* oc = InitialObject;
	uint32_t symbol_num = 0;
	for (;oc;oc=oc->next) {
	  if (oc == s1->obj) break;
	  symbol_num += oc->symbol_num - 1;
	  printf("symtable p:%p\n", oc->symbol_table_p);
	  printf("sym num :%d\n", oc->symbol_num);	  
	}
	CoffReloc* r = s1->p->PointerToRelocations;
	CoffReloc* rend = r + s1->p->NumberOfRelocations;
	int i = 0;
	for (;r<rend;r++,i++) {
	  /* printf("%p,%p\n", r->VirtualAddress, size_of_code); */
	  /* printf("%p,%p\n", r->SymbolTableIndex,symbol_num); */
	  /* printf("%d,%p,%p\n", i,r, rend); */
	  if (r->VirtualAddress == -1/*should be deleted**/) {
	    printf("deleted entry\n");
	    // r--;// this is because number of entry had been decremented already.
	    continue;
	  }
	  r->VirtualAddress += size_of_code;
	  r->SymbolTableIndex += symbol_num;	
	  WriteFile
	    (handle, r, sizeof(CoffReloc), &dwWriteSize, NULL);
	}
	size_of_code += s1->p->SizeOfRawData;	
      }
    }
  }
  printf("o\n");
}

void write_symbol_table(HANDLE handle) {
  // 
  SymbolTablePointer = SetFilePointer(handle, 0, NULL, FILE_CURRENT);    
  ObjectChain* oc = InitialObject;
  DWORD dwWriteSize;
  uint8_t drop = 0;
  IMAGE_SYMBOL* is;
  IMAGE_SYMBOL* sym_end;
  uint8_t ever_absolute = 0;
  uint32_t str_table_length = 0;
  void* p;
  for (;oc;oc=oc->next) {
    printf("!symtable p:%p\n", oc->symbol_table_p);
    printf("!sym num :%d\n", oc->symbol_num);
    printf("str table len:%d\n", *(uint32_t*)oc->str_table_p);
    // external records which had been resolved should be skipped.
    // record for a merged section should be merged(one of them should be skipped.)
    // Note you need to combine relocation information together.
    // .absolute should not be redundunt.
    is = oc->symbol_table_p;
    sym_end = is + oc->symbol_num;
    for (;is!=sym_end;is++) {
      if (*(uint32_t*)is == 0) {
	IMAGE_SYMBOL* q = is;// + str_table_length;
	char* name = GET_NAME(q, oc->str_table_p);
	printf("kkk:%s,%p\n", name, *((uint32_t*)is+1));
	// printf("kkk:%s,%p\n", name, *((uint32_t*)is+1));
      }
      UPDATE_STRTABLE_OFFSET(is, str_table_length);
      if (is->SectionNumber == IMAGE_SYM_ABSOLUTE) {
	if (ever_absolute == 0) {
	  ever_absolute = 1;
	} else {
	  drop = 1;
	}
      }
      if (is->Value == -1) {	
	printf("skip\n");
	drop = 1;
      }
      if (drop) {
	oc->symbol_num -= (is->NumberOfAuxSymbols + 1);
	drop = 0;
      }
      else {
	WriteFile(handle, is,
		  (is->NumberOfAuxSymbols + 1) * sizeof(IMAGE_SYMBOL), &dwWriteSize, NULL);
      }
      for (p=is+is->NumberOfAuxSymbols;is<p;is++);
      // you should probably decrease the relocation count on aux symbols on setion.
      // In a meanwhile, it is as it was.
    }
    SymbolNum += oc->symbol_num;
    str_table_length += *(uint32_t*)oc->str_table_p - 4;    
  }
  str_table_length += 4;
  // first 4byte is for string table length.
  // This cannot be determined until you compute whole length.
  WriteFile
    (handle, &str_table_length,
     4, &dwWriteSize, NULL);
  for (oc = InitialObject;oc;oc=oc->next) {
    if (str_table_length = *(uint32_t*)oc->str_table_p - 4) {
      WriteFile
	(handle, oc->str_table_p + 4,
	 str_table_length, &dwWriteSize, NULL);
    }
  }
}

HANDLE emit_dos(HANDLE hFile) {
  write_dos_header(hFile);
  write_nt_header_signature(hFile);
  write_image_file_header(hFile);
  write_optional_header(hFile);
  write_sections(hFile);
  write_raw_data(hFile);
  return hFile;
}

HANDLE emit_obj(HANDLE hFile) {

  // As you cannot know symbol table offset before you accumulate length of all rest of headers,
  // you just let it blank and fill it later on.
  DWORD fp = SetFilePointer(hFile, sizeof(IMAGE_FILE_HEADER), NULL, FILE_CURRENT);
  write_sections(hFile);
  write_raw_data_for_obj(hFile);
  write_symbol_table(hFile);
  write_relocation_record(hFile);  
  fp = SetFilePointer(hFile, 0, NULL, FILE_BEGIN/*0*/);
  // fp should be 0.
  write_image_file_header(hFile);
  return hFile;
}

void gen() {

  HANDLE hFile = CreateFile
    (
     OutputFileName, GENERIC_ALL/* | GENERIC_EXECUTE*/, 0, NULL,
     CREATE_ALWAYS, 0/*FILE_SHARE_READ*/, NULL
     );
  if (EmitType != EMIT_OBJ) {
    emit_dos(hFile);
  } else {
    emit_obj(hFile);
  }
  CloseHandle(hFile);
}

