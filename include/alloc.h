
void mem_init();
void* __malloc(uint32_t s);
void* __thalloc();
void __free(uint8_t* p);

void* __z__mem__alloc_file(char* fname);