
#include <stdint.h>
#include "types.h"

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

#define IMAGE_REL_AMD64_ADDR64 1
#define IMAGE_REL_AMD64_ADDR32 2
#define IMAGE_REL_AMD64_ADDR32NB 3
#define IMAGE_REL_AMD64_REL32 4
#define IMAGE_REL_AMD64_SSPAN32 0x10

// own output type
#define PE_NONE		0
#define PE_REL		1
#define PE_EXEC		2
#define PE_DLL		3

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef uint64_t ULONGLONG;
typedef void* HANDLE;

typedef short SHORT;

typedef struct _IMAGE_DOS_HEADER {
    WORD  e_magic;      /* 00: MZ Header signature */
    WORD  e_cblp;       /* 02: Bytes on last page of file */
    WORD  e_cp;         /* 04: Pages in file */
    WORD  e_crlc;       /* 06: Relocations */
    WORD  e_cparhdr;    /* 08: Size of header in paragraphs */
    WORD  e_minalloc;   /* 0a: Minimum extra paragraphs needed */
    WORD  e_maxalloc;   /* 0c: Maximum extra paragraphs needed */
    WORD  e_ss;         /* 0e: Initial (relative) SS value */
    WORD  e_sp;         /* 10: Initial SP value */
    WORD  e_csum;       /* 12: Checksum */
    WORD  e_ip;         /* 14: Initial IP value */
    WORD  e_cs;         /* 16: Initial (relative) CS value */
    WORD  e_lfarlc;     /* 18: File address of relocation table */
    WORD  e_ovno;       /* 1a: Overlay number */
    WORD  e_res[4];     /* 1c: Reserved words */
    WORD  e_oemid;      /* 24: OEM identifier (for e_oeminfo) */
    WORD  e_oeminfo;    /* 26: OEM information; e_oemid specific */
    WORD  e_res2[10];   /* 28: Reserved words */
    DWORD e_lfanew;     /* 3c: Offset to extended header */
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
  WORD  Machine;
  WORD  NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD  SizeOfOptionalHeader;
  WORD  Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
  DWORD VirtualAddress;
  DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct  _IMAGE_OPTIONAL_HEADER64 {
  WORD  Magic; /* 0x20b */
  BYTE MajorLinkerVersion;
  BYTE MinorLinkerVersion;
  DWORD SizeOfCode;
  DWORD SizeOfInitializedData;
  DWORD SizeOfUninitializedData;
  DWORD AddressOfEntryPoint;
  DWORD BaseOfCode;
  ULONGLONG ImageBase;
  DWORD SectionAlignment;
  DWORD FileAlignment;
  WORD MajorOperatingSystemVersion;
  WORD MinorOperatingSystemVersion;
  WORD MajorImageVersion;
  WORD MinorImageVersion;
  WORD MajorSubsystemVersion;
  WORD MinorSubsystemVersion;
  DWORD Win32VersionValue;
  DWORD SizeOfImage;
  DWORD SizeOfHeaders;
  DWORD CheckSum;
  WORD Subsystem;
  WORD DllCharacteristics;
  ULONGLONG SizeOfStackReserve;
  ULONGLONG SizeOfStackCommit;
  ULONGLONG SizeOfHeapReserve;
  ULONGLONG SizeOfHeapCommit;
  DWORD LoaderFlags;
  DWORD NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_NT_HEADERS64 {
  DWORD Signature;
  IMAGE_FILE_HEADER FileHeader;
  IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef struct _IMAGE_OPTIONAL_HEADER {

  /* Standard fields */

  WORD  Magic; /* 0x10b or 0x107 */	/* 0x00 */
  BYTE  MajorLinkerVersion;
  BYTE  MinorLinkerVersion;
  DWORD SizeOfCode;
  DWORD SizeOfInitializedData;
  DWORD SizeOfUninitializedData;
  DWORD AddressOfEntryPoint;		/* 0x10 */
  DWORD BaseOfCode;
  DWORD BaseOfData;

  /* NT additional fields */

  DWORD ImageBase;
  DWORD SectionAlignment;		/* 0x20 */
  DWORD FileAlignment;
  WORD  MajorOperatingSystemVersion;
  WORD  MinorOperatingSystemVersion;
  WORD  MajorImageVersion;
  WORD  MinorImageVersion;
  WORD  MajorSubsystemVersion;		/* 0x30 */
  WORD  MinorSubsystemVersion;
  DWORD Win32VersionValue;
  DWORD SizeOfImage;
  DWORD SizeOfHeaders;
  DWORD CheckSum;			/* 0x40 */
  WORD  Subsystem;
  WORD  DllCharacteristics;
  DWORD SizeOfStackReserve;
  DWORD SizeOfStackCommit;
  DWORD SizeOfHeapReserve;		/* 0x50 */
  DWORD SizeOfHeapCommit;
  DWORD LoaderFlags;
  DWORD NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES]; /* 0x60 */
  /* 0xE0 */
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct _IMAGE_NT_HEADERS {
  DWORD Signature; /* "PE"\0\0 */	/* 0x00 */
  IMAGE_FILE_HEADER FileHeader;		/* 0x04 */
  IMAGE_OPTIONAL_HEADER32 OptionalHeader;	/* 0x18 */
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;


#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER {
  BYTE  Name[IMAGE_SIZEOF_SHORT_NAME];
  union {
    DWORD PhysicalAddress;
    DWORD VirtualSize;
  } Misc;
  DWORD VirtualAddress;
  DWORD SizeOfRawData;
  DWORD PointerToRawData;
  DWORD PointerToRelocations;
  DWORD PointerToLinenumbers;
  WORD  NumberOfRelocations;
  WORD  NumberOfLinenumbers;
  DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef struct __attribute__((packed))  _IMAGE_SYMBOL {
    union {
        BYTE    ShortName[8];
        struct {
            DWORD   Short;
            DWORD   Long;
        } Name;
        DWORD   LongName[2];
    } N;
    DWORD   Value;
    SHORT   SectionNumber;
    WORD    Type;
    BYTE    StorageClass;
    BYTE    NumberOfAuxSymbols;
} IMAGE_SYMBOL;


typedef struct __attribute__((packed)) _IMAGE_IMPORT_DESCRIPTOR {
	union {
		DWORD	Characteristics; /* 0 for terminating null import descriptor  */
		DWORD	OriginalFirstThunk;	/* RVA to original unbound IAT */
	} u;
	DWORD	TimeDateStamp;	/* 0 if not bound,
				 * -1 if bound, and real date\time stamp
				 *    in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT
				 * (new BIND)
				 * otherwise date/time stamp of DLL bound to
				 * (Old BIND)
				 */
	DWORD	ForwarderChain;	/* -1 if no forwarders */
	DWORD	Name;
	/* RVA to IAT (if bound this IAT has actual addresses) */
	DWORD	FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR,*PIMAGE_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_THUNK_DATA32 {
	union {
		DWORD ForwarderString;
		DWORD Function;
		DWORD Ordinal;
		DWORD AddressOfData;
	} u1;
} IMAGE_THUNK_DATA32,*PIMAGE_THUNK_DATA32;

typedef struct _IMAGE_IMPORT_BY_NAME {
	WORD	Hint;
	BYTE	Name[1];
} IMAGE_IMPORT_BY_NAME,*PIMAGE_IMPORT_BY_NAME;


/* These defines are for the Characteristics bitfield. */
/* #define IMAGE_SCN_TYPE_REG0x00000000 - Reserved */
/* #define IMAGE_SCN_TYPE_DSECT0x00000001 - Reserved */
/* #define IMAGE_SCN_TYPE_NOLOAD0x00000002 - Reserved */
/* #define IMAGE_SCN_TYPE_GROUP0x00000004 - Reserved */
#define IMAGE_SCN_TYPE_NO_PAD 0x00000008 /* Reserved */
/* #define IMAGE_SCN_TYPE_COPY0x00000010 - Reserved */

#define IMAGE_SCN_CNT_CODE 0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080

#define IMAGE_SCN_LNK_OTHER 0x00000100
#define IMAGE_SCN_LNK_INFO 0x00000200
/* #defineIMAGE_SCN_TYPE_OVER0x00000400 - Reserved */
#define IMAGE_SCN_LNK_REMOVE 0x00000800
#define IMAGE_SCN_LNK_COMDAT 0x00001000

/* 0x00002000 - Reserved */
/* #define IMAGE_SCN_MEM_PROTECTED 0x00004000 - Obsolete */
#define IMAGE_SCN_MEM_FARDATA 0x00008000

/* #define IMAGE_SCN_MEM_SYSHEAP0x00010000 - Obsolete */
#define IMAGE_SCN_MEM_PURGEABLE 0x00020000
#define IMAGE_SCN_MEM_16BIT 0x00020000
#define IMAGE_SCN_MEM_LOCKED 0x00040000
#define IMAGE_SCN_MEM_PRELOAD 0x00080000

#define IMAGE_SCN_ALIGN_1BYTES 0x00100000
#define IMAGE_SCN_ALIGN_2BYTES 0x00200000
#define IMAGE_SCN_ALIGN_4BYTES 0x00300000
#define IMAGE_SCN_ALIGN_8BYTES 0x00400000
#define IMAGE_SCN_ALIGN_16BYTES 0x00500000  /* Default */
#define IMAGE_SCN_ALIGN_32BYTES 0x00600000
#define IMAGE_SCN_ALIGN_64BYTES 0x00700000
#define IMAGE_SCN_ALIGN_128BYTES 0x00800000
#define IMAGE_SCN_ALIGN_256BYTES 0x00900000
#define IMAGE_SCN_ALIGN_512BYTES 0x00A00000
#define IMAGE_SCN_ALIGN_1024BYTES 0x00B00000
#define IMAGE_SCN_ALIGN_2048BYTES 0x00C00000
#define IMAGE_SCN_ALIGN_4096BYTES 0x00D00000
#define IMAGE_SCN_ALIGN_8192BYTES 0x00E00000
/* 0x00F00000 - Unused */
#define IMAGE_SCN_ALIGN_MASK 0x00F00000

#define IMAGE_SCN_LNK_NRELOC_OVFL 0x01000000


#define IMAGE_SCN_MEM_DISCARDABLE 0x02000000
#define IMAGE_SCN_MEM_NOT_CACHED 0x04000000
#define IMAGE_SCN_MEM_NOT_PAGED 0x08000000
#define IMAGE_SCN_MEM_SHARED 0x10000000
#define IMAGE_SCN_MEM_EXECUTE 0x20000000
#define IMAGE_SCN_MEM_READ 0x40000000
#define IMAGE_SCN_MEM_WRITE 0x80000000


typedef struct __attribute__((packed)) {
  // offset to the relocation entry
  uint32_t rel_addr;
  // index of symbol table entry
  uint32_t sym_index;
  // number of byte sequence
  uint16_t byte_num;
} relocation_entry;

typedef struct {
  
  // head of image dos header
  const IMAGE_DOS_HEADER* ids;
  // head of image nt header (retrieved from the last element of image dos header)
  const IMAGE_NT_HEADERS32* nt;
  // 
  const IMAGE_SECTION_HEADER* sec_begin;
  const IMAGE_SECTION_HEADER* sec_end;
  // head of text section
  union {
    const char* text_begin;
    uint32_t base_of_code;
  };// code_begin;
  union {
    const char* text_end;
    uint32_t size_of_code;
  };// code_size;
  // end of text section
  /* const char* text_end;   */

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
  // 
  const char* image_base;
  void* this_image_base;
  int fd;
} info_on_pe, info_on_coff;

typedef struct _IMAGE_EXPORT_DIRECTORY {
  DWORD Characteristics;
  DWORD TimeDateStamp;
  WORD MajorVersion;
  WORD MinorVersion;
  DWORD Name;
  DWORD Base;
  DWORD NumberOfFunctions;
  DWORD NumberOfNames;
  DWORD AddressOfFunctions;
  DWORD AddressOfNames;
  DWORD AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY,*PIMAGE_EXPORT_DIRECTORY;

typedef union {
  IMAGE_NT_HEADERS32* nt_header32;
  IMAGE_NT_HEADERS64* nt_header64;
} nt_header;

char _check_on_iat(p_guest rip, p_guest query);
/* p_host get_dll_name(p_host idata_section, p_host f_name); */
p_host _find_f_addr(p_guest rip, p_guest query);
/*heap**/void* get_parent_heap_from_guest(p_guest p);
char* __z__obj__load_pe(char* name, void* out);

/* void iterate_import_directory(uint8_t* pp, void* _p); */
