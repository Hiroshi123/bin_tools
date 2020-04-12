
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
#include "os.h"

#define DEBUG 1

extern void* get_caller_address();
extern void* mmap__(uint32_t size);
extern int get_file_size__(void*);

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

static H HeapMeta;
Chunk* CUR_CHUNK;

void mem_init() {
  printf("mem init\n");
  // allocate a page for a Bin which contains multiple chunks
  Bin* c = mmap__(0x1000);
  if (c == 0) {
    printf("error,%p\n", c);
    return;
  }
  HeapMeta.bin_head = c;
  HeapMeta.bin_tail = c;
  c->bin[0] = 1;
  c->next = 0;
  uint8_t* m = mmap__(0x1000);
  c->page_addr = m;
}

int maxConsecutiveOnes(int x)
{
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

uint8_t slowest_find_consecutive_bin(uint8_t* p, uint8_t q) {
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

void set_bin(uint64_t* p, uint8_t s, uint8_t q) {
  uint8_t i = 0;
  uint64_t v = 1;
  uint8_t e = s + q - 1;
  // printf("e:%d\n",e);
  uint8_t up = 0;
  for (;i<=e;v<<=1,i++) {
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

void unset_bin(uint64_t* p, uint8_t s, uint8_t q) {
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
    // printf("%p\n",c->bin[0] & 0x1);
  }
}

void printb(unsigned int v) {
  
  unsigned int mask = (int)1 << (sizeof(v) * CHAR_BIT - 1);
  do {
    // putchar(mask & v ? '1' : '0');
    logger_emit(mask & v ? "1" : "0");
  }
  while (mask >>= 1);
}

void putb(unsigned int v) {
  logger_emit("0b");
  // putchar('0'), putchar('b'), printb(v), putchar('\n');
  printb(v);
  logger_emit("\n");
}

void printbin(Bin* c) {
  char log[60] = {};
  sprintf(log, "-----bitmap-----:%x(%x-%x)\n",
	  c, c->page_addr, c->page_addr + 0x1000 - 1);
  logger_emit(log);
  // printf("-----bitmap-----:%x(%x-%x)\n",
  //	 c, c->page_addr, c->page_addr + 0x1000 - 1);
  uint8_t i=0;
  for (;i<8;i++) {
    putb(c->bin[i]);
  }
  // printf("----------\n");
}

static void* expand_heap(int size) {
  
  // size / 0xFFFFF000;
  void* m = mmap__(size);
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
  printf("bb\n");
  return m;
}

void* __thalloc() {
  printf("a\n");
  return expand_heap(1024);
}

void* __malloc(int size) {
#ifdef DEBUG
  logger_emit("-----------------\n");
  // printf("-----------------\n");
  char log[40] = {};
  sprintf(log, "malloc:size:%d(0x%x),bin:%d\n",size+1,size+1,((size+1)/0x10)+1);
  logger_emit(log);
#endif
  if (size >= 0x1000) {
    return expand_heap(size);
  }
  Bin* c = HeapMeta.bin_tail;
  uint8_t* r;
  uint8_t a;
  Bin* pre = c;
  uint8_t bin_size = (size / 0x10) + 1;
  for (;c;c=c->next) {
    pre = c;
  b1:
    a = slowest_find_consecutive_bin(&c->bin[0], bin_size);
    if (a) {
      set_bin(&c->bin[0], a, bin_size);
      r = (Block*)c->page_addr + a;
      *(r - 1) = bin_size;
#ifdef DEBUG
      printbin(c);
      logger_emit("-----------------\n");
      sprintf(log, "index:%p,%d,%p\n", a, a, r);
      logger_emit(log);
#endif
      return r;
    }
  }
  c = pre+=1;
  if (c) {
    printf("error\n");
  }
  uint8_t* m = mmap__(0x1000);
  c->page_addr = m;
  c->next = 0;
  c->bin[0] = 1;
  HeapMeta.bin_tail->next = c;
  HeapMeta.bin_tail = c;
  goto b1;
  return r;
}

void __free(uint8_t* p) {
  uint8_t bin_size;
  printf("free:%p\n",p);
  if (((size_t)p & 0xFFF) == 0) {
    bin_size = 255;
    printf("mmmmmm\n");
  } else {
    bin_size = *(p-1);
  }
  Bin* c = HeapMeta.bin_head;
  void* q;
  uint8_t s;
  uint8_t* l = p + 0x10 * bin_size;
  // iterating bins will not be heavy in usual.
  for (;c;c=c->next) {
    q = p - c->page_addr;
    if (q < 0x1000) {
      s = (uint32_t)q / 0x10;
      unset_bin(&c->bin[0], s, bin_size);
      // real free will not clean up but i do.
      for (;p<l;*p=0,p++);
#ifdef DEBUG
      printbin(c);
#endif
      break;
    }
  }
}

#ifdef _WIN32

void* alloc_file(char* fname) {
  HANDLE hFile = CreateFile
    (
     fname, GENERIC_ALL/* | GENERIC_EXECUTE*/, 0, NULL,
     OPEN_EXISTING/*CREATE_NEW*/, 0/*FILE_SHARE_READ*/, NULL
     );
  if (hFile == -1) {
    printf("file not found\n");
    return 0;
  }
  DWORD wReadSize;
  DWORD size = GetFileSize(hFile , NULL);
  if (size == -1) {
    printf("cannot get file size\n");
    return 0;
  }
  void* p = __malloc(size);
  ReadFile(hFile, p, size, &wReadSize , NULL);
  CloseHandle(hFile);
  return p;
}
#endif

#ifdef linux
void* alloc_file(char* fname) {
  void* fp = open__(fname, O_RDONLY);
  printf("open ok,%p\n", fp);
  int size = get_file_size__(fp);
  printf("f1:%p,%d\n", fp, size);
  void* p = __malloc(size);
  __os__read(fp, p, size);
  printf("aaa,%p,%p\n", p,*(uint64_t*)p);  
  return p;
}
#endif

