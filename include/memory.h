
#include <stdint.h>
#include "types.h"
#include "macro.h"

typedef struct __attribute__((__packed__)) {
  // first argument of mmap
  p_host begin;
  // second argument of mmap
  // size_t size;
  
  // last 3 byte can be used as size of mapped file.
  // and top 1 bit used as number of pages.
  // which means you can concatenate maximum 16 pages in a row,
  uint16_t page_num;
  
  // 3rd & 4th argument of mmap
  uint32_t flags; // prot & map flags
  // 5th & 6th arugment
  uint16_t file;
  // subsequent mapping
  // struct __heap* next;
  p_guest guest_addr;
  uint16_t index1;
  uint16_t index2;  
  // pointer to name of the mapping if it is none, set 0.
  union {
    uint64_t name_addr;
    uint64_t parent_addr;
  };
} heap;

heap* get_page(uint8_t num);
heap* init_map_file(const char *const fname);
heap* map_file(const int fd, uint32_t size, uint32_t guest_addr);
heap* guest_mmap(void* guest_addr, uint32_t map_size, uint32_t flags, uint64_t name_or_parent_addr, int fd);
void* EXPORT(get_diff_host_guest_addr(void* guest_addr));
void  EXPORT(get_diff_host_guest_addr_(void* guest_addr, void** host_addr));
/* void get_host_head(void* guest_addr, void** host_addr); */
p_host get_host_head(p_guest guest_addr);

void get_host_head_from_host(void* host_addr, void** host_head_addr);
p_host get_host_addr(p_guest guest_addr);
heap* get_current_meta_addr();
heap* search_page_by_name(char* query);
heap* out_map_file(const int fd);

// All of pages which were mapped on the process will be
// mapped on a file which allows you to debug value of memory much easier.
// PAGE_PATH will be used for naming these files.
// These naming will be indexed as two stages.
// First index is 1 indexed-number of dybamic library.
// index 0 by the way is execution file itself.
// Second index indicates index of unit of page.
// Unit of page means some piece of loadable block which are mapped aligned together.
// First appears unit on a object format file will get smaller index.
// PAGE_PATH represents both index (dll index, map index of unit of mapping).
// path1 is index dll, path2 is page index
uint8_t get_page_path1();
void set_page_path1(uint8_t p);
uint8_t get_page_path2();
void set_page_path2(uint8_t p);
int open_page_map();



