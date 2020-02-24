
// #include "pe.h"

typedef struct {
  // head of image dos header
  const IMAGE_DOS_HEADER* ids;
  // head of image nt header (retrieved from the last element of image dos header)
  const IMAGE_NT_HEADERS32* nt;
  // 
  const IMAGE_SECTION_HEADER* sec_begin;
  const IMAGE_SECTION_HEADER* sec_end;
  // head of text section
  const char* text_begin;
  // end of text section
  const char* text_end;  
  // heads of image import descriptor which are
  // given from rawdatapointer on idata section
  const IMAGE_IMPORT_DESCRIPTOR* idata_begin;
  const IMAGE_IMPORT_DESCRIPTOR* idata_end;
  uint32_t idata_vaddr;
  
  // symbol table begin & end can be retrieved from
  // image_file_header,
  const IMAGE_SYMBOL* sym_begin;
  const IMAGE_SYMBOL* sym_end;
  
  // begining of string table is recorded as information
  // on symbol table entry.
  // the first 4 byte of str_begin is length where lets
  // you estimate the end of it.
  const char* str_begin;
  const char* str_end;
  
  const char* text_reloc_begin;
  const char* text_reloc_end;
  
} info_on_pe, info_on_coff;

