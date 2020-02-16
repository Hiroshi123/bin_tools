
#ifdef windows
#include <windows.h>
#endif

#ifdef linux
#include <sys/mman.h>
#include <sys/stat.h>
#endif

#include <stdint.h>
#include <stdlib.h>

#ifdef windows

NTSYSAPI NTSTATUS  WINAPI NtCreateSection(HANDLE*,ACCESS_MASK,const OBJECT_ATTRIBUTES*,const LARGE_INTEGER*,ULONG,ULONG,HANDLE);
NTSYSAPI NTSTATUS  WINAPI NtMapViewOfSection(HANDLE,HANDLE,PVOID*,ULONG,SIZE_T,const LARGE_INTEGER*,SIZE_T*,void*,ULONG,ULONG);

void* mmap_win(uint32_t size) {
  LARGE_INTEGER MaximumSize;
  HANDLE hSection = 0;
  // *hSection = 0;
  MaximumSize.QuadPart = size;
  NTSTATUS status = NtCreateSection
    (&hSection,
     /* SECTION_MAP_READ, */
     /* SECTION_ALL_ACCESS, */
     SECTION_ALL_ACCESS,
     0,
     &MaximumSize,
     // PAGE_NOACCESS,
     PAGE_READWRITE,//|PAGE_NOCACHE,//|MEM_PRIVATE,
     // |MEM_PRIVATE
     SEC_COMMIT,//MEM_MAPPED,//|SEC_FILE,
     // 
     // MEM_MAPPED,
     // SEC_RESERVE | SEC_COMMIT,
     // SEC_FILE,
     // SEC_IMAGE,
     0// hFile
     );
  size_t* base = 0;//malloc(1000);
  size_t* _size = 0;// malloc(8);
  status = NtMapViewOfSection
    (hSection,
     //NtCurrentProcess(),
     ((HANDLE) -1),
     &base,
     0, 0, 0,
     &_size,
     2/*ViewUnmap*/,
     0,
     // PAGE_NOACCESS);
     PAGE_READWRITE);

  //logger_emit("------------------\n");
  char log[15] = {};
  // sprintf(log, "mmap__ %p\n", base);
  // logger_emit(log);
  return base;
}

#endif

#ifdef linux
static void* mmap_linux(uint32_t size) {
  
  void *tmp = mmap
    (NULL, size/*PAGE_SIZE*/, PROT_READ | PROT_WRITE,
     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  printf("tmp:%p\n", tmp);
  return tmp;
}
#endif

void* mmap__(uint32_t size) {
#ifdef windows
  return mmap_win(size);
#elif linux
  return mmap_linux(size);
#endif
}
