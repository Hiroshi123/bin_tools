

int open__(char*, int);

// the one which has 4th argument will be rcx=>r10, and the rest of register is shared
// with normal calling convention.

void __os__exit(int);
int __os__read(int fd, void* buf, int len);
int __os__write(int fd, void* buf, int len);
void __os__close(int fd);
int __os__lseek(int fd, int buf, int len);
uint64_t  __os__open(char* fname, int flags, int permission);

void* __os__mmap(void* addr, int size, int protection, int map_attr, int, int);

int __os__fork();
int __os__execve(void*,void*,void*);

int __z__os__fcntl(int fd, void* buf, int len);
