
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "memory.h"

// static char COUNT = 0;

/*static*/ heap *HEAP_HEADER_ADDR_HEAD;
heap *HEAP_HEADER_ADDR_P;
heap *HEAP_HEADER_ADDR_TAIL;
size_t PAGE_SIZE;

__attribute__((constructor)) void set_heap_header() {
  PAGE_SIZE = getpagesize();
  heap *tmp = (heap*)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (tmp == MAP_FAILED) {
    printf("error\n");
    exit(0);
  }
  HEAP_HEADER_ADDR_HEAD = tmp;
  HEAP_HEADER_ADDR_P = tmp;
  HEAP_HEADER_ADDR_TAIL = tmp + PAGE_SIZE;  
}

__attribute__((destructor)) void unset_heap_header() {
  printf("unset:%x\n");
  const heap *h = (heap *)HEAP_HEADER_ADDR_HEAD;
  for (; h != HEAP_HEADER_ADDR_P; h++) {
    munmap(h->begin, /*h->page_num * */PAGE_SIZE);
    if (h->file)
      close(h->file);
  }
}

heap *init_map_file(const char *const fname) {
  const int fd = open(fname, O_RDWR);
  if (fd == -1)
    return 0;
  struct stat stbuf;
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    return 0;
  }
  const size_t map_size = ((stbuf.st_size + 0x1000) & 0xfffff000);
  printf("map:%x,%x\n",stbuf.st_size,map_size);
  void *begin = mmap(NULL, map_size, PROT_READ|PROT_WRITE|PROT_EXEC,
                     MAP_PRIVATE, fd, 0);
  if (begin == MAP_FAILED) {
    printf("error:%u\n", errno);
    close(fd);
    return 0;
  }
  heap *h = (heap *)HEAP_HEADER_ADDR_P;
  h->begin = begin;
  h->page_num = (uint16_t)0x1000;
  h->page_num += stbuf.st_size;
  h->guest_addr = -1;
  // TODO.
  // PROT & MAP flags for mmap is scheduled to be condensed as 4byte.
  // Although it is originally 16byte(8+8)
  h->flags = 1;
  h->file = (uint16_t)fd;
  HEAP_HEADER_ADDR_P += 1;
  return h;
}

heap* get_page(uint8_t num) {
  
  int map_size = PAGE_SIZE*num;
  int fd = -1;
  void *begin = mmap(NULL,map_size , PROT_READ|PROT_WRITE|PROT_EXEC,
                     MAP_PRIVATE|MAP_ANONYMOUS, fd, 0);
  heap *h = (heap *)HEAP_HEADER_ADDR_P;
  h->begin = begin;
  h->page_num = map_size / PAGE_SIZE;
  h->flags = 1;
  h->file = (uint16_t)fd;
  h->guest_addr = -1;
  HEAP_HEADER_ADDR_P = h + 1;  
  return h;
}

uint8_t* get_page_head() {
  heap* h = (heap*)(HEAP_HEADER_ADDR_P-1);
  return h->begin;
}

heap* get_current_meta_addr() {
  return (heap*)HEAP_HEADER_ADDR_P;
}

heap* get_current_meta_head() {
  printf("meta:%x,%x\n",HEAP_HEADER_ADDR_HEAD,*(uint8_t*)HEAP_HEADER_ADDR_HEAD);
  return (heap*)HEAP_HEADER_ADDR_HEAD;
}

heap* guest_mmap(void* guest_addr, uint8_t page_num) {
  
  //
  int map_size = PAGE_SIZE*page_num;
  int fd = -1;
  void *begin = mmap(NULL,map_size , PROT_READ|PROT_WRITE|PROT_EXEC,
                     MAP_PRIVATE|MAP_ANONYMOUS, fd, 0);
  heap *h = (heap *)HEAP_HEADER_ADDR_P;
  printf("begin:%x,%x\n",begin, h);
  h->begin = begin;
  h->page_num = map_size / PAGE_SIZE;
  h->flags = 1;
  h->file = (uint16_t)fd;
  h->guest_addr = guest_addr;
  HEAP_HEADER_ADDR_P += 1;  
  return h;
}

void* get_diff_host_guest_addr(void* guest_addr) {

  printf("arg1:%x\n",guest_addr);
  
  heap* h = HEAP_HEADER_ADDR_HEAD;
  heap* h_end = HEAP_HEADER_ADDR_P;
  void* guest_begin = (uint64_t)guest_addr & 0xfffff000;
  for (;h!=h_end;h++) {
    printf("%x\n",h->begin);
    if (h->guest_addr != -1) {
      printf("g:%x,%x\n",h->guest_addr,guest_begin);
      if (guest_begin == h->guest_addr) {
	uint64_t diff = (uint64_t)h->begin - (uint64_t)guest_begin;
	printf("diff:%x,%x\n",h->begin,diff);
	return (void*)diff;
      }
      /* if (h->guest_addr != h->page_num) {	 */
      /* } */
    }
  }
  return NULL;
}

