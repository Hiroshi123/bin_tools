
#ifdef _WIN32
#include <windows.h>
#include <winternl.h>
#endif

#ifdef linux
#include <limits.h>
#include <fcntl.h>

#endif
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "os.h"
#include "logger.h"

#define DEBUG 1
#define __logger_emit(X) logger_emit("memory.log", X)

extern void* get_caller_address();

// chunk (0,1,2,3,4,5)
// -----> chain of chunk
// bin ()
// head tail

// mal(bins)

typedef struct _Chunk {
  uint32_t size;
  uint32_t rel_addr;
} Chunk;

typedef struct _Bin Bin;

struct _Bin {
  uint32_t bin[8];
  Bin* next;
  size_t page_addr;
};

typedef struct _H {
  Bin* bin_head;
  Bin* bin_tail;
} H;

typedef struct {
  uint8_t d[0x10];
} Block;

static H HeapMeta = {-1, -1};
// Chunk* CUR_CHUNK;
static int INIT_DONE = 0;

void mem_init() {

  if (INIT_DONE == 1) return;
  logger_init("memory.log");
  // allocate a page for a Bin which contains multiple chunks
  Bin* c = __os__mmap
      (NULL, 0x1000/*PAGE_SIZE*/, PROT_READ | PROT_WRITE,
     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (c <= 0) {
    __logger_emit("map error\n");
    return;
  }
  HeapMeta.bin_head = c;
  HeapMeta.bin_tail = c;
  c->bin[0] = 1;
  c->next = 0;
  uint8_t* m = __os__mmap
      (NULL, 0x1000/*PAGE_SIZE*/, PROT_READ | PROT_WRITE,
     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  c->page_addr = m;
  INIT_DONE = 1;
}

static int maxConsecutiveOnes(int x) {
  // Initialize result
  int count = 0;
  // Count the number of iterations to
  // reach x = 0.
  while (x!=0) {
    // This operation reduces length
    // of every sequence of 1s by one.
    x = (x & (x << 1));
    count++;
  }
  return count;
}

static uint8_t slowest_find_consecutive_bin(uint8_t* p, uint8_t q) {
  uint8_t v = 1;
  uint8_t i = 0;
  uint8_t j = 0;
  uint8_t c = 0;
  int BIN_N = 256;
  uint8_t s = sizeof(uint8_t) * 8;
  uint8_t t = BIN_N / s;
  for (;j<t;v <<= 1) {
    if (!v) v = 1;
    if ((*p & v) == 0) {
      c += 1;
      if (c == q) {
	return i + 1 + j * 8 - q;
      }
    } else {
      c = 0;
    }
    if (i % s == s-1) {
      p++;
      j++;
      i = 0;
    } else {
      i++;
    }
  }
  return 0;
}

static void set_bin(uint64_t* p, uint8_t s, uint8_t q) {
  uint8_t i = 0;
  uint64_t v = 1;
  uint8_t e = s + q - 1;
  uint8_t up = 0;
  /* logger_emit_p(e); */
  for (;i<=e;v<<=1,i++) {
    /* logger_emit("misc.log", "a\n"); */
    if (!v) v = 1;
    if (up) {
      p++;
      up = 0;
    }
    if (i % 64 == 63) {
      up = 1;
    }
    if (i<s) continue;
    *p |= v;
    if (i == 255) break;
  }
}

static void unset_bin(uint64_t* p, uint8_t s, uint8_t q) {
  uint64_t v = 1;
  uint8_t i = 0;
  uint8_t e = s + q;
  for (;i<=e;v<<=1,i++) {
    if (!v) v = 1;
    if (i<s) continue;
    *p &= ~v;
    if (i % 64 == 63) {
      p++;
    }
  }
}

static void printb(unsigned int v) {

  unsigned int mask = (int)1 << (sizeof(v) * CHAR_BIT - 1);
  do {
    // putchar(mask & v ? '1' : '0');
    __logger_emit(mask & v ? "1" : "0");
  }
  while (mask >>= 1);
}

static void putb(unsigned int v) {
  __logger_emit("0b");
  // putchar('0'), putchar('b'), printb(v), putchar('\n');
  printb(v);
  __logger_emit("\n");
}

static void printbin(Bin* c) {

  /* char log[60] = {}; */
  /* sprintf(log, "-----bitmap-----:%x(%x-%x)\n", */
  /* 	  c, c->page_addr, c->page_addr + 0x1000 - 1); */
  /* __logger_emit(log); */
  __logger_emit("----Bitmap----\n");
  uint8_t i=0;
  for (;i<8;i++) {
    putb(c->bin[i]);
  }
}

static void* expand_heap(int size) {

  // size / 0xFFFFF000;
  void* m = __os__mmap
    (NULL, size/*PAGE_SIZE*/, PROT_READ | PROT_WRITE,
     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  char log[40] = {};
  __logger_emit("-----------------\n");
  /* sprintf(log, "expand heap:%p,%d\n", m, size); */
  /* __logger_emit(log); */
  Bin* c = HeapMeta.bin_head;
  for (;c->next;c=c->next);
  Bin* pre = c;
  c++;
  pre->next = c;
  c->page_addr = m;
  c->next = 0;
  uint8_t i=0;
  for (;i<8;i++) {
    c->bin[i] = -1;
    putb(c->bin[i]);
  }
  return m;
}

void* __thalloc() {
  return expand_heap(1024);
}

void* __malloc(int size) {
  __logger_emit("-----------------\n");
#ifdef DEBUG
  /* char log[40] = {}; */
  /* sprintf(log, "malloc:size:%d(0x%x),bin:%d\n",size+1,size+1,((size+1)/0x10)+1); */
  /* __logger_emit(log); */
#endif
  if (size >= 0x1000 - 0x10) {
    return expand_heap(size);
  }
  Bin* c = HeapMeta.bin_tail;
  uint8_t* r;
  uint8_t a;
  Bin* pre = c;
  uint8_t bins_size = (size / 0x10) + 1;
  for (;c;c=c->next) {
    pre = c;
  b1:
    a = slowest_find_consecutive_bin(&c->bin[0], bins_size);
    if (a) {
      set_bin(&c->bin[0], a, bins_size);
      r = (Block*)c->page_addr + a;
      *(r - 1) = bins_size;
#ifdef DEBUG
      printbin(c);
      __logger_emit("-----------------\n");
      /* sprintf(log, "index:%p,%d,%p\n", a, a, r); */
      /* __logger_emit(log); */
#endif
      return r;
    }
  }
  if (c) {
    __logger_emit("error\n");
    return 0;
  }
  c = pre + 1;
  uint8_t* m = __os__mmap
    (NULL, 0x1000/*PAGE_SIZE*/, PROT_READ | PROT_WRITE,
     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  c->page_addr = m;
  c->next = 0;
  c->bin[0] = 1;
  HeapMeta.bin_tail->next = c;
  HeapMeta.bin_tail = c;
  goto b1;
  return r;
}

void __free(uint8_t* p) {
  uint8_t bins_size;
  if (((size_t)p & 0xFFF) == 0) {
    bins_size = 255;
  } else {
    bins_size = *(p-1);
  }
  Bin* c = HeapMeta.bin_head;
  void* q;
  uint8_t s;
  uint8_t* l = p + 0x10 * bins_size;
  // iterating bins will not be heavy in usual.
  for (;c;c=c->next) {
    q = p - c->page_addr;
    if (q < 0x1000) {
      s = (uint32_t)q / 0x10;
      unset_bin(&c->bin[0], s, bins_size);
      // real free will not clean up but i do.
      for (;p<l;*p=0,p++);
#ifdef DEBUG
      printbin(c);
#endif
      break;
    }
  }
}

static int get_file_size(void* fd) {

  __os__lseek(fd, 0, SEEK_END); // seek to end of file
  int size = __os__lseek(fd, 0, SEEK_CUR);
  // ftell(f); // get current file pointer
  __os__lseek(fd, 0, SEEK_SET);
  return size;
}

void* __z__mem__alloc_file_with_malloc(char* fname) {
  void* fp = __os__open(fname, O_RDONLY, 0777);
  int size = get_file_size(fp);
  void* p = __malloc(size);
  __os__read(fp, p, size);
  return p;
}

void* __z__mem__alloc_file(char* fname) {
  int fd = __os__open(fname, O_RDONLY, 0666);
  // TODO ::
  if (fd < 0) return 0;
  int size = get_file_size(fd);
  const size_t map_size = ((size + 0xfff) & 0xfffff000);
  void* p = __os__mmap
    (NULL, map_size/*PAGE_SIZE*/, PROT_READ|PROT_EXEC | PROT_WRITE,
     MAP_PRIVATE/* | MAP_ANONYMOUS*/, fd, 0);
  return p;
}

