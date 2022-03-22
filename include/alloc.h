
#include <stdint.h>

void mem_init();
void* __malloc(uint32_t s);
void* __thalloc();
void __free(uint8_t* p);

void* __z__mem__alloc_file(char* fname);
int __z__mem__get_file_size(void* fd);
void* __z__mem__alloc_file_with_size(char* fname, int* size_p);
