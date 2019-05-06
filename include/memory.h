
#include <stdint.h>

typedef struct __attribute__((__packed__)) {
  // first argument of mmap
  void *begin;
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
  void* guest_addr;
  
} heap;

heap* init_map_file(const char *const fname);
heap* guest_mmap(void* guest_addr, uint32_t page_num);
void* get_diff_host_guest_addr(void* guest_addr);
void get_diff_host_guest_addr_(void* guest_addr, void** host_addr);
void get_host_head(void* guest_addr, void** host_addr);
void get_host_head_from_host(void* host_addr, void** host_head_addr);
heap* get_current_meta_addr();
