
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    // show_usage();
    return 1;
  }

  const int fd = open(argv[2], O_RDONLY);
  if (fd == -1) {
    close(fd);
    exit(1);
  }
  struct stat stbuf;
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    exit(1);
  }
  char *p = (char *)mmap(NULL, map_size, PROT_READ /*|PROT_WRITE*/,
                         MAP_SHARED /* | MAP_FIXED*/, fd, 0);
}
