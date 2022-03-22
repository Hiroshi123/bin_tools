
/* #include <windows.h> */
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>

#include "alloc.h"
#include "link.h"
#include "objformat.h"
#include "os.h"
#include "pe.h"

// will be dead soon
static ObjectChain* InitialObject;
static uint32_t CurrentFileOffset = 0;
static uint32_t RelocationHead = 0;

static int __FD;

// ImageBase is DWORD(4byte) on 32bit/QWORD on 64bit.
uint64_t ImageBase = 0;
uint32_t TotalImageSize = 0;
uint8_t EmitType = 0;
uint32_t SymbolNum = 0;
uint32_t SymbolTablePointer = 0;

extern Config* Confp;

static int get_header_size() {
  if (Confp->header_size) return Confp->header_size;
  int header_size = sizeof(IMAGE_DOS_HEADER) + 4 + sizeof(IMAGE_FILE_HEADER) +
                    sizeof(IMAGE_OPTIONAL_HEADER64) +
                    sizeof(IMAGE_SECTION_HEADER) * Confp->shdr_num;
  if (Confp->file_format == PE32) header_size -= 0x10;
  header_size = (header_size + (FILE_ALIGNMENT - 1) & -FILE_ALIGNMENT);
  Confp->header_size = header_size;
  return header_size;
}

static int get_out_size() {
  SectionContainer* scon = Confp->initial_section;
  SectionChain* schain;
  IMAGE_SECTION_HEADER* ish;
  // grab a last virtual address and its size to compute ouput file size.
  int size = 0;
  for (; scon; scon = scon->next) {
    size += scon->size;
  }
  size += get_header_size();
  return size;
}

static void* write_a_section(int fd) {
  IMAGE_SECTION_HEADER sec = {};
  strcpy(&sec.Name[0], ".gik");
  sec.VirtualAddress = 0x1000;
  int cur;
  if (Confp->file_format == PE32) {
    cur = sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS32);
  } else {
    cur = sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS64);
  }
  if (__os__lseek(fd, 0, SEEK_CUR) != cur) {
    __os__write(1, "error\n", 6);
  }
  sec.PointerToRawData = Confp->header_size;
  int size = Confp->out_size - Confp->header_size;
  sec.Misc.VirtualSize = size;
  sec.SizeOfRawData = size;
  sec.PointerToRelocations = 0;
  sec.PointerToLinenumbers = 0;
  sec.NumberOfRelocations = 0;
  sec.NumberOfLinenumbers = 0;
  sec.Characteristics =
      IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
  __os__write(fd, &sec, sizeof(IMAGE_SECTION_HEADER));
  __os__lseek(fd, FILE_ALIGNMENT, SEEK_SET);
}

static void* write_section(SectionChain* sc, int fd) {
  IMAGE_SECTION_HEADER* sec = sc->p;
  SectionChain* s = sc;
  uint32_t size = 0;
  for (; s; s = s->this) {
    size += ((IMAGE_SECTION_HEADER*)s->p)->SizeOfRawData;
  }
  sec->Misc.VirtualSize = size;
  // VirtualSize == 0 wont work out.
  if (!sec->Misc.VirtualSize) sec->Misc.VirtualSize = 1;
  sec->PointerToRelocations = 0;
  sec->PointerToLinenumbers = 0;
  sec->NumberOfRelocations = 0;
  sec->NumberOfLinenumbers = 0;
  /* sec->Characteristics = 0xE0000080; */
  __os__write(fd, sec, sizeof(IMAGE_SECTION_HEADER));
  return sec;
}

static void write_section_for_obj(SectionChain* sc, HANDLE handle) {
  IMAGE_SECTION_HEADER* sec = sc->p;
  SectionChain* s = sc;
  uint32_t size = 0;
  uint32_t relocation_num = 0;
  for (; s; s = s->this) {
    size += ((IMAGE_SECTION_HEADER*)s->p)->SizeOfRawData;
    relocation_num += ((IMAGE_SECTION_HEADER*)s->p)->NumberOfRelocations;
    // printf("size up,%d,%d\n", s->p->SizeOfRawData,s->p->NumberOfRelocations);
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
  __os__write(handle, sec, sizeof(IMAGE_SECTION_HEADER));
  sec->SizeOfRawData = orig_sizeofrawdata;
  sec->NumberOfRelocations = orig_numofrel;
  sec->PointerToRelocations = orig_pointertorelo;
}

static void write_dos_header(HANDLE handle) {
  IMAGE_DOS_HEADER* dos_header = __malloc(sizeof(IMAGE_DOS_HEADER));
  dos_header->e_magic = 0x5a4d;
  dos_header->e_cblp = 0xff;
  dos_header->e_lfarlc = 0x60;
  dos_header->e_lfanew = 0x40;
  DWORD dwWriteSize;
  __os__write(handle, dos_header, sizeof(IMAGE_DOS_HEADER));
}

static void write_nt_header_signature(int fd) {
  int size = 4;
  DWORD dwWriteSize;
  DWORD nt_header_signature = 0x4550;
  __os__write(fd, &nt_header_signature, size);
}

static void write_image_file_header(HANDLE handle) {
  IMAGE_FILE_HEADER* file_header = __malloc(sizeof(IMAGE_FILE_HEADER));
  file_header->Machine = (Confp->file_format == PE32) ? 0x14c : 0x8664;
  file_header->NumberOfSections = 1;  // TotalSectionNum;
  file_header->TimeDateStamp = 0;
  if (Confp->outfile_type == PE_REL) {
    file_header->PointerToSymbolTable = SymbolTablePointer;
    file_header->NumberOfSymbols = SymbolNum;
    file_header->SizeOfOptionalHeader = 0;
    file_header->Characteristics = 0;
  }
  if (Confp->outfile_type == PE_EXEC || Confp->outfile_type == PE_DLL) {
    file_header->PointerToSymbolTable = 0;
    file_header->NumberOfSymbols = 0;
    file_header->Characteristics = (Confp->file_format == PE32) ? 0x32f : 0x22f;
    file_header->SizeOfOptionalHeader = (Confp->file_format == PE32)
                                            ? sizeof(IMAGE_OPTIONAL_HEADER32)
                                            : sizeof(IMAGE_OPTIONAL_HEADER64);
  }
  if (Confp->outfile_type == PE_DLL) {
    file_header->Characteristics |= 0x2000;
  }
  __os__write(handle, file_header, sizeof(IMAGE_FILE_HEADER));
}

static void set_image_directory_entry(IMAGE_DATA_DIRECTORY* d) {
  SectionContainer* s = Confp->initial_section;
  SectionChain* t;
  IMAGE_SECTION_HEADER* ish;
  for (; s; s = s->next) {
    for (t = s->this; t; t = t->this) {
      /* printf("t:%p,%p\n", s,t); */
      ish = t->p;
      if (!strcmp(ish->Name, ".edata")) {
        // printf("edata:%p\n",1);
        d->VirtualAddress = ish->VirtualAddress;
        if (ish->Misc.VirtualSize == 0)
          d->Size = ish->SizeOfRawData;
        else
          d->Size = ish->Misc.VirtualSize;
      }
      if (!strcmp(ish->Name, ".idata")) {
        /* printf("idata:%p,%p\n",d, t->p->SizeOfRawData); */
        (d + 1)->VirtualAddress = ish->VirtualAddress;
        if (ish->Misc.VirtualSize == 0)
          (d + 1)->Size = ish->SizeOfRawData;
        else
          (d + 1)->Size = ish->Misc.VirtualSize;
      }
      if (!strcmp(ish->Name, ".rdata")) {
      }
      if (!strcmp(ish->Name, ".tls")) {
        // tls
        //
      }
    }
  }
}

static void write_optional_header(int fd) {
  int size = sizeof(IMAGE_OPTIONAL_HEADER64);
  IMAGE_OPTIONAL_HEADER64* optional_header = __malloc(size);
  // 0x10b or 0x20b
  optional_header->Magic = (Confp->file_format == PE32) ? 0x10b : 0x20b;
  // values which could be anything...
  optional_header->MajorLinkerVersion = 0;
  optional_header->MinorLinkerVersion = 0;
  // these 3 needs to be correct.
  if (Confp->out_size == 0) {
    Confp->out_size = get_out_size();
  }
  optional_header->SizeOfCode = Confp->out_size;
  optional_header->SizeOfInitializedData = 0;
  optional_header->SizeOfUninitializedData = 0;
  // this is often set as 0x1000
  optional_header->BaseOfCode = 0x1000;
  if (Confp->file_format == PE32) {
    // BaseOfData
    DWORD* p = &optional_header->ImageBase;
    *p = 0x1000;
    optional_header->BaseOfCode = 0x1000;
    p++;
    *p = ImageBase;
  } else {
    optional_header->ImageBase = Confp->base_address;
    /* optional_header->ImageBase = 0x40000; */
  }
  if (Confp->entry_address) {
    optional_header->AddressOfEntryPoint = Confp->entry_address;
  } else {
    optional_header->AddressOfEntryPoint = 0x1000;
  }
  ///////////////////////////////////////////////////////

  optional_header->MajorOperatingSystemVersion = 6;
  optional_header->MinorOperatingSystemVersion = 0;
  optional_header->MajorImageVersion = 0;
  optional_header->MinorImageVersion = 0;
  // subsystem needs
  optional_header->MajorSubsystemVersion = 6;
  optional_header->MinorSubsystemVersion = 0;
  // assume just 1 section case
  optional_header->SizeOfImage =
      (Confp->out_size + (2 * SECTION_ALIGNMENT - 1)) & -SECTION_ALIGNMENT;
  optional_header->SizeOfHeaders = get_header_size();
  optional_header->SectionAlignment = SECTION_ALIGNMENT;
  optional_header->FileAlignment = FILE_ALIGNMENT;
  // DLL characterstics which is probably most important
  optional_header->DllCharacteristics = 0;  // 0x40 | 0x800 | 0x100;
  // IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
  optional_header->Subsystem = 3;

  // from 0x48 to (0x58)0x68 is the only difference of optional header between
  // 32bit and 64bit.
  uint8_t* p;  // = ((uint8_t*)&optional_header) + 0x48;
  if (Confp->file_format == PE32) {
    // NumberOfRvaAndSizes
    uint32_t* _p = ((uint8_t*)optional_header) + 0x5c;
    *_p = 0x10;
    p = _p + 1;
  } else {
    uint32_t* _p = ((uint8_t*)optional_header) + 0x6c;
    *_p = 0x10;
    p = _p + 1;
  }
  /* optional_header->SizeOfStackReserve; */
  /* optional_header->SizeOfStackCommit; */
  /* optional_header->SizeOfHeapReserve; */
  /* optional_header->SizeOfHeapCommit;   */
  /* optional_header->LoaderFlags = 0; */
  /* optional_header->NumberOfRvaAndSizes = 0x10; */
  // iterate_section_container();
  set_image_directory_entry(p /*&optional_header->DataDirectory*/);
  DWORD dwWriteSize;
  __os__write(fd, optional_header,
              (Confp->file_format == PE32) ? sizeof(IMAGE_OPTIONAL_HEADER32)
                                           : sizeof(IMAGE_OPTIONAL_HEADER64));
}

static void write_sections(HANDLE handle) {
  SectionContainer* scon = Confp->initial_section;
  SectionChain* schain;
  for (scon = scon->next; scon; scon = scon->next) {
    schain = scon->this;
    printf(":%p,%s\n", schain, scon->name);
    IMAGE_SECTION_HEADER* sec;
    for (; schain; schain = schain->next) {
      schain->p;
    }
    // printf("!!%p,%p,%s\n", schain, schain->virtual_address, scon->name);
    /* if (EmitType == EMIT_OBJ) { */
    /*   write_section_for_obj(s->this, handle); */
    /* } else { */
    /*   write_section(s->this, handle); */
    /* } */
  }
}

static void write_raw_data(void* arg1) {
  SectionContainer* scon = arg1;
  SectionChain* schain = scon->init;
  if (schain == 0) return;
  IMAGE_SECTION_HEADER* ish;
  for (; schain; schain = schain->this) {
    if (Confp->loglevel >= 0) {
      logger_emit("misc.log", scon->name);
    }
    ish = schain->p;
    /* printf("%s,%d\n", ish->Name, ish->SizeOfRawData); */
    if (ish->SizeOfRawData) {
      int fp = __os__lseek(__FD, 0, SEEK_CUR /*FILE_CURRENT*/);
      /* printf("%p\n", fp); */
      __os__write(__FD, schain->data, ish->SizeOfRawData);
    }
  }
}

static void write_raw_data_for_obj(HANDLE handle) {
  // Actual Data is going to be fed.
  SectionChain* s = Confp->initial_section;
  SectionChain* s1;
  IMAGE_SECTION_HEADER* sec;
  DWORD dwWriteSize;
  for (; s; s = s->next) {
    sec = s->this->p;
    printf("!!pointer to raw,%p,%p\n", sec->PointerToRawData,
           sec->SizeOfRawData);
    if (sec->SizeOfRawData) {
      // __os__lseek();
      DWORD fp = __os__lseek(handle, 0, 0 /*FILE_CURRENT*/);
      printf("%p,%p,%p,%s\n", fp, sec->PointerToRawData,
             sec->PointerToRawData - fp, sec->Name);
      __os__lseek(handle, sec->PointerToRawData - fp, 0 /*FILE_CURRENT*/);
      for (s1 = s->this; s1; s1 = s1->this) {
        // printf("sizeofraw:%d\n", s1->p->SizeOfRawData);
        __os__write(handle, s1->data, sec->SizeOfRawData);
      }
    }
    // if you have relocation record on a merging section which comes after the
    // former section, then Offset & SymbolIndex must be different.
    if (sec->NumberOfRelocations) {
      if (RelocationHead == 0)
        RelocationHead = __os__lseek(handle, 0, 0 /*FILE_CURRENT*/);
      for (s1 = s->this; s1; s1 = s1->this) {
        __os__lseek(handle, sec->NumberOfRelocations * sizeof(CoffReloc),
                    0 /*FILE_CURRENT*/);
        /* __os__write */
        /*   (handle, s1->p->PointerToRelocations, */
        /*    s1->p->NumberOfRelocations * 10 ,&dwWriteSize , NULL); */
      }
    }
  }
}

static void write_relocation_record(HANDLE handle) {
  SectionChain* s = Confp->initial_section;
  SectionChain* s1;
  IMAGE_SECTION_HEADER* sec;
  IMAGE_SECTION_HEADER* ish;
  DWORD dwWriteSize;
  DWORD fp = __os__lseek(handle, RelocationHead, 0 /*FILE_BEGIN*/);
  for (; s; s = s->next) {
    sec = s->this->p;
    if (sec->NumberOfRelocations) {
      uint32_t size_of_code = 0;
      // __os__lseek(handle, sec->PointerToRelocations - fp, NULL, FILE_);
      for (s1 = s->this; s1; s1 = s1->this) {
        printf("write relocation rec,%p\n", s1->obj);
        ObjectChain* oc = InitialObject;
        uint32_t symbol_num = 0;
        for (; oc; oc = oc->next) {
          if (oc == s1->obj) break;
          symbol_num += oc->symbol_num - 1;
          printf("symtable p:%p\n", oc->symbol_table_p);
          printf("sym num :%d\n", oc->symbol_num);
        }
        ish = s1->p;
        CoffReloc* r = ish->PointerToRelocations;
        CoffReloc* rend = r + ish->NumberOfRelocations;
        int i = 0;
        for (; r < rend; r++, i++) {
          /* printf("%p,%p\n", r->VirtualAddress, size_of_code); */
          /* printf("%p,%p\n", r->SymbolTableIndex,symbol_num); */
          /* printf("%d,%p,%p\n", i,r, rend); */
          if (r->VirtualAddress == -1 /*should be deleted**/) {
            printf("deleted entry\n");
            // r--;// this is because number of entry had been decremented
            // already.
            continue;
          }
          r->VirtualAddress += size_of_code;
          r->SymbolTableIndex += symbol_num;
          __os__write(handle, r, sizeof(CoffReloc));
        }
        size_of_code += ish->SizeOfRawData;
      }
    }
  }
  printf("o\n");
}

static void write_symbol_table(HANDLE handle) {
  //
  SymbolTablePointer = __os__lseek(handle, 0, SEEK_CUR /*FILE_CURRENT*/);
  ObjectChain* oc = InitialObject;
  DWORD dwWriteSize;
  uint8_t drop = 0;
  IMAGE_SYMBOL* is;
  IMAGE_SYMBOL* sym_end;
  uint8_t ever_absolute = 0;
  uint32_t str_table_length = 0;
  void* p;
  for (; oc; oc = oc->next) {
    printf("!symtable p:%p\n", oc->symbol_table_p);
    printf("!sym num :%d\n", oc->symbol_num);
    printf("str table len:%d\n", *(uint32_t*)oc->str_table_p);
    // external records which had been resolved should be skipped.
    // record for a merged section should be merged(one of them should be
    // skipped.) Note you need to combine relocation information together.
    // .absolute should not be redundunt.
    is = oc->symbol_table_p;
    sym_end = is + oc->symbol_num;
    for (; is != sym_end; is++) {
      if (*(uint32_t*)is == 0) {
        IMAGE_SYMBOL* q = is;  // + str_table_length;
        char* name = GET_NAME(q, oc->str_table_p);
        printf("kkk:%s,%p\n", name, *((uint32_t*)is + 1));
        // printf("kkk:%s,%p\n", name, *((uint32_t*)is+1));
      }
      UPDATE_STRTABLE_OFFSET(is, str_table_length);
      if (is->SectionNumber == 0 /*IMAGE_SYM_ABSOLUTE*/) {
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
      } else {
        __os__write(handle, is,
                    (is->NumberOfAuxSymbols + 1) * sizeof(IMAGE_SYMBOL));
      }
      for (p = is + is->NumberOfAuxSymbols; is < p; is++)
        ;
      // you should probably decrease the relocation count on aux symbols on
      // setion. In a meanwhile, it is as it was.
    }
    SymbolNum += oc->symbol_num;
    str_table_length += *(uint32_t*)oc->str_table_p - 4;
  }
  str_table_length += 4;
  // first 4byte is for string table length.
  // This cannot be determined until you compute whole length.
  __os__write(handle, &str_table_length, 4);
  for (oc = InitialObject; oc; oc = oc->next) {
    if (str_table_length = *(uint32_t*)oc->str_table_p - 4) {
      __os__write(handle, oc->str_table_p + 4, str_table_length);
    }
  }
}

static void emit_dos(int fd) {
  write_dos_header(fd);
  write_nt_header_signature(fd);
  write_image_file_header(fd);
  write_optional_header(fd);
  write_a_section(fd);
  iterate_section_container(write_raw_data);
  return;
}

static HANDLE emit_obj(HANDLE hFile) {
  // As you cannot know symbol table offset before you accumulate length of all
  // rest of headers, you just let it blank and fill it later on.
  DWORD fp =
      __os__lseek(hFile, sizeof(IMAGE_FILE_HEADER), SEEK_CUR /*FILE_CURRENT*/);
  write_sections(hFile);
  write_raw_data_for_obj(hFile);
  write_symbol_table(hFile);
  write_relocation_record(hFile);
  fp = __os__lseek(hFile, 0, SEEK_CUR);
  // fp should be 0.
  write_image_file_header(hFile);
  return hFile;
}

void __z__link__gen_pe(char* name) {
  int fd = __os__open(name, O_CREAT | O_WRONLY | O_TRUNC, 0777);
  __FD = fd;
  emit_dos(fd);
  __os__close(fd);
}
