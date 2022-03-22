
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>

int open__(char* filename, int flags) {
  return open(filename, flags, 0777);
  // sys_open(filename, O_RDONLY|O_CLOEXEC);  
}

int __os__open(char* filename, int flags, int perm) {
  return open(filename, flags, 0777);
}

int get_file_size__(void* fd) {
  /* struct stat st; */
  /* stat(filename, &st); */
  lseek(fd, 0, SEEK_END); // seek to end of file
  int size = lseek(fd, 0, SEEK_CUR);
  // ftell(f); // get current file pointer
  lseek(fd, 0, SEEK_SET);
  return size;
}

void __os__read(void* fp, void* buf, int len) {
  read(fp, buf, len);
}

void __os__write(char* fp, void* buf, int len) {
  write(fp, buf, len);
}

int __os__seek(void* fd, int offset, uint8_t origin) {

  // origin == 0 (from beginning)
  // origin == 1 (from current file offset)
  // origin == 2 (from end)
  
  return lseek(fd, offset, origin);
}

