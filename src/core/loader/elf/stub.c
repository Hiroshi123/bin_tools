
#include <stdint.h>

int LOADER_SIZE = 1;
int EXE_SIZE = 1;

void end();

static void* __deob01(uint8_t* src, uint8_t* dst, int size) {

  int i = 0;
  for (;i<size;src++,dst++,i++) {
    *dst = *src;
  }
  return dst;
}

static void __deob02(uint8_t* src, uint8_t* dst, int size) {

  int i = 0;
  for (;i<size;src++,dst++,i++) {
    *dst = *src;
  }
}

void __start(void* rsp) {

  // __write(1, "s\n", 2);
  uint8_t* p = end + 5;
  
  // let the program header allocate enough amount of memory
  // deobfuscate and copy from bottom of this file which is beginning of loader to the end of loader
  // to the newly-allocated memory.

  int loader_size = LOADER_SIZE;  
  int exe_size = EXE_SIZE;
  uint8_t* exe_offset = p + loader_size;
  uint8_t* new_loader_offset = exe_offset + exe_size;
  
  p = __deob01(p, new_loader_offset, loader_size);
  
  __deob02(exe_offset, p, exe_size);

}

void end() {}
