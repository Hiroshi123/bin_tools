
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "macro.h"
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

heap* map_file(const int fd, uint32_t size, p_guest guest_addr) {

  const size_t map_size = ((size + 0x1000) & 0xfffff000);
  void *begin = mmap(NULL, map_size, PROT_READ/*|PROT_WRITE*/|PROT_EXEC,
                     MAP_PRIVATE, fd, 0);
  if (begin == MAP_FAILED) {
    printf("error:%u\n", errno);
    close(fd);
    return 0;
  }
  heap *h = (heap *)HEAP_HEADER_ADDR_P;
  h->begin = begin;
  h->page_num = (uint16_t)0x1000;
  h->page_num += size;
  h->guest_addr = guest_addr;
  // TODO.
  // PROT & MAP flags for mmap is scheduled to be condensed as 4byte.
  // Although it is originally 16byte(8+8)
  /* if (guest_addr == -1) { */
  /*   h->flags = 1; */
  /* } else { */
  /*   h->flags = 0; */
  /* } */
  h->flags = 1;
  h->file = (uint16_t)fd;
  HEAP_HEADER_ADDR_P += 1;
  return h;
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
  return map_file(fd, stbuf.st_size, -1);
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
  HEAP_HEADER_ADDR_P += 1;  
  return h;
}

uint8_t* get_page_head() {
  heap* h = (heap*)(HEAP_HEADER_ADDR_P-1);
  return h->begin;
}

heap* get_current_meta_addr() {
  return (heap*)HEAP_HEADER_ADDR_P-1;
}

heap* get_current_meta_head() {
  printf("meta:%x,%x\n",HEAP_HEADER_ADDR_HEAD,*(uint8_t*)HEAP_HEADER_ADDR_HEAD);
  return (heap*)HEAP_HEADER_ADDR_HEAD;
}

heap* guest_mmap(void* guest_addr, uint32_t map_size, uint32_t flags, uint64_t name_or_parent_addr) {
  
  int fd = -1;
  void *begin = mmap(NULL,map_size , PROT_READ|PROT_WRITE|PROT_EXEC,
                     MAP_PRIVATE|MAP_ANONYMOUS, fd, 0);
  heap *h = (heap *)HEAP_HEADER_ADDR_P;
  h->begin = begin;
  h->page_num = map_size / PAGE_SIZE;
  printf("page:%x,%x\n",map_size, h->page_num);
  h->flags = flags;
  h->file = (uint16_t)fd;
  // higher bits are going to be set as 0.
  h->guest_addr = (uint64_t)guest_addr & 0xffffffff;
  if (h->flags == 1) {
    h->name_addr = name_or_parent_addr;    
  } else {
    h->parent_addr = name_or_parent_addr;
  }
  //
  HEAP_HEADER_ADDR_P += 1;
  return h;
  
}

void* EXPORT(get_diff_host_guest_addr)
  (void* guest_addr) {

  heap* h = HEAP_HEADER_ADDR_HEAD;
  heap* h_end = HEAP_HEADER_ADDR_P;
  void* guest_begin = (uint64_t)guest_addr & 0xfffff000;
  for (;h!=h_end;h++) {
    printf("%x\n",h->begin);
    if (h->guest_addr != -1) {
      printf("gg:%lx,%lx\n",h->guest_addr,guest_begin);
      uint32_t page = 0;
      for (;page < h->page_num;page++) {
	if (guest_begin == h->guest_addr + page * PAGE_SIZE) {
	  printf("a\n");
	  uint64_t diff = (uint64_t)h->begin - ((uint64_t)guest_begin - (uint64_t) page * PAGE_SIZE);
	  return (void*)diff;
	}
      }
    }
  }
  return NULL;
}

void get_diff_host_guest_addr_(void* guest_addr, void** host_addr) {
  
  heap* h = HEAP_HEADER_ADDR_HEAD;
  heap* h_end = HEAP_HEADER_ADDR_P;
  void* guest_begin = (uint64_t)guest_addr & 0xfffff000;
  
  /* printf("%x,%x\n",guest_addr,guest_begin); */
  
  uint32_t page = 0;
  for (;h!=h_end;h++) {
    /* printf("%x\n",h->begin); */
    if (h->guest_addr != -1) {
      /* printf("g:%lx,%lx\n",h->guest_addr,guest_begin); */
      
      page = 0;
      for (;page < h->page_num;page++) {
	if (guest_begin == h->guest_addr + page * PAGE_SIZE) {
	  uint64_t diff = (uint64_t)h->begin - ((uint64_t)guest_begin - (uint64_t) page * PAGE_SIZE);
	  uint32_t dd = page * PAGE_SIZE;
	  uint16_t guest_in_page_offset = guest_addr - guest_begin;
	  /* printf("diff:%lx,%x,%x\n",h->begin,page,dd); */
	  *host_addr = h->begin + dd + guest_in_page_offset;
	  return;
	}
      }
    }
  }
}

// [Caution] :: you need to make sure that you included header proerply when you return
// 64bit pointer, otherwise it will be truncated.
p_host get_host_addr(p_guest guest_addr) {
  
  heap* h = HEAP_HEADER_ADDR_HEAD;
  heap* h_end = HEAP_HEADER_ADDR_P;
  uint32_t guest_begin = guest_addr & 0xfffff000;
  uint32_t page = 0;

  for (;h!=h_end;h++) {
    if (h->guest_addr != -1) {
      page = 0;
      for (;page < h->page_num;page++) {
	if (guest_begin == h->guest_addr + page * PAGE_SIZE) {
	  /* uint64_t diff = (uint64_t)h->begin - ((uint64_t)guest_begin - (uint64_t) page * PAGE_SIZE); */
	  uint32_t dd = page * PAGE_SIZE;
	  uint16_t guest_in_page_offset = guest_addr - guest_begin;
	  return h->begin + dd + guest_in_page_offset;
	}
      }
    }
  }
  return 0;
}

p_host get_host_head(p_guest guest_addr/*, void** host_head_addr*/) {
  
  heap* h = HEAP_HEADER_ADDR_HEAD;
  heap* h_end = HEAP_HEADER_ADDR_P;
  uint32_t guest_begin = guest_addr & 0xfffff000;  
  uint32_t page = 0;
  for (;h!=h_end;h++) {
    if (h->guest_addr != -1) {
      page = 0;
      for (;page < h->page_num;page++) {
	if (guest_begin == h->guest_addr + page * PAGE_SIZE) {
	  // *host_head_addr = h->begin;
	  return h->begin;
	}
      }
    }
  }
  return 0;
}

void get_host_head_from_host(void* host_addr, void** host_head_addr) {
  heap* h = HEAP_HEADER_ADDR_HEAD;
  heap* h_end = HEAP_HEADER_ADDR_P;
  uint32_t page = 0;
  void* host_a_page_head = (uint64_t)host_addr & 0xfffffffffffff000;  
  for (;h!=h_end;h++) {
    if (h->guest_addr != -1) {
      page = 0;
      for (;page < h->page_num;page++) {
	if (host_a_page_head == h->begin + page * PAGE_SIZE) {
	  *host_head_addr = h->begin;
	  return;
	}
      }
    }
  }
}

void* get_diff_host_addr(void* host_addr) {
  
  /* printf("arg1:%x\n",guest_addr); */ 
  heap* h = HEAP_HEADER_ADDR_HEAD;
  heap* h_end = HEAP_HEADER_ADDR_P;
  void* host_begin = (uint64_t)host_addr & 0xfffff000;
  for (;h!=h_end;h++) {
    /* printf("%x\n",h->begin); */
    if (h->guest_addr != -1) {
      /* printf("g:%x,%x\n",h->guest_addr,guest_begin); */
      if (host_begin == h->begin) {
	uint64_t diff = (uint64_t)h->guest_addr - (uint64_t)host_begin;
	/* printf("diff:%x,%x\n",h->begin,diff); */
	return (void*)diff;
      }
      /* if (h->guest_addr != h->page_num) {	 */
      /* } */
    }
  }
  return NULL;
}

/*heap**/void* get_parent_heap_from_guest(p_guest p) {
  heap* h = HEAP_HEADER_ADDR_HEAD;
  heap* h_end = HEAP_HEADER_ADDR_P;
  uint32_t guest_begin = p & 0xfffff000;  
  uint32_t page = 0;
  for (;h!=h_end;h++) {
    if (h->guest_addr != -1) {
      page = 0;
      for (;page < h->page_num;page++) {
	if (guest_begin == h->guest_addr + page * PAGE_SIZE) {	  
	  return h->parent_addr;
	}
      }
    }
  }
  return NULL;
}

heap* search_page_by_name(char* query) {
  heap* h = HEAP_HEADER_ADDR_HEAD;
  heap* h_end = HEAP_HEADER_ADDR_P;
  for (;h!=h_end;h++) {
    /* printf("%x\n",h->begin); */
    if (h->guest_addr != -1) {
      if (h->flags && h->name_addr && h->name_addr == query) {
	return h;
      }
    }
  }
  return NULL;
}

