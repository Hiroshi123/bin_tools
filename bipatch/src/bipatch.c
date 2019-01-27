
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
  uint8_t cmd;
  uint64_t addr;
  uint16_t length;
  uint8_t *elem;
} patch_format;

typedef struct {
  patch_format *begin;
  patch_format *end;
  int size_updown;
} patch_info;

typedef struct {
  char *map_begin;
  char *map_end;
} map_info;

static uint64_t ascii_to_bin(const char *begin, const char *end) {
  char *x = (char *)begin;
  uint64_t r = 0;
  char s = 0;
  if (*x == '0' && *(x + 1) == 'x') {
    for (; x != end; x++)
      ;
    for (; x != begin; x--, s += 4) {
      if ('0' <= *x && *x <= '9') {
        r += (*x - '0') << s;
      } else if ('a' <= *x && *x <= 'f') {
        r += (*x - ('a' + 10)) << s;
      } else {
        printf("parse error\n");
        return -1;
      }
    }
    return r;
  }
  return -1;
}

void read_patch(const char *fname, patch_info *info) {

  // input mapping
  const int fd = open(fname, O_RDONLY);
  if (fd == -1) {
    close(fd);
    exit(1);
  }
  struct stat stbuf;
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    exit(1);
  }
  size_t map_size = (stbuf.st_size + 0x1000) & 0xfffff000;
  char *p = (char *)mmap(NULL, map_size, PROT_READ /*|PROT_WRITE*/,
                         MAP_SHARED /* | MAP_FIXED*/, fd, 0);
  const char *end = (char *)((size_t)p + stbuf.st_size);
  map_size = 4096;
  // output mapping
  patch_format *q = (patch_format *)mmap(NULL, map_size, PROT_READ | PROT_WRITE,
                                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  uint8_t *data = (uint8_t *)mmap(NULL, map_size, PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  info->begin = q;
  info->size_updown = 0;
  uint64_t d;
  uint64_t addr;
  for (; p != end; p++, q++) {
    for (; *p != '\n'; p++) {
      if (*p == '1') {
        q->cmd = 1;
      } else if (*p == '0') {
        q->cmd = 0;
      } else {
        printf("parse error");
      }
      for (p++; *p != ' '; p++)
        ;
      addr = ascii_to_bin(p, p + 10);
      if (addr == (uint64_t)-1) {
        printf("parse error");
      }
      q->addr = addr;
      // next
      for (p++; *p != ' '; p++)
        ;
      if (q->cmd == 0) {
        d = ascii_to_bin(p, p + 4);
        if (d == (uint64_t)-1) {
          printf("parse error");
        }
        q->length = d;
        q->elem = 0;
        info->size_updown -= d;
      } else {
        q->elem = (uint8_t *)data;
        for (; *p != '\n'; p++, data++, q->length++) {
          d = ascii_to_bin(p, p + 4);
          if (d == (uint64_t)-1) {
            printf("parse error");
          }
          *data = d;
          for (p++; *p != ' '; p++)
            ;
        }
        info->size_updown += q->length;
      }
    }
  }
  munmap(q, map_size);
  info->end = (patch_format *)p;
}

void map_input_file(const char *fname, map_info *_p) {

  const int fd = open(fname, O_RDONLY);
  if (fd == -1) {
    exit(0);
  }
  struct stat stbuf;
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    exit(1);
  }
  size_t map_size = (stbuf.st_size + 0x1000) & 0xfffff000;
  char *p = (char *)mmap(NULL, map_size, PROT_READ /*|PROT_WRITE*/,
                         MAP_SHARED /* | MAP_FIXED*/, fd, 0);
  if (p == MAP_FAILED) {
    printf("err:%d\n", errno);
  }
  _p->map_begin = p;
  _p->map_end = (char *)((size_t)p + map_size);
}

void map_output_file(const char *fname, uint64_t map_size, map_info *_out) {

  map_size = (map_size + 0x1000) & 0xfffff000;
  int fd = open(fname, O_RDWR | O_CREAT | O_TRUNC);
  if (fd == -1) {
    printf("error");
    exit(0);
  }
  // int map_size=4096;
  char *p = (char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE,
                         MAP_SHARED /* | MAP_FIXED*/, fd, 0);
  if (p == MAP_FAILED) {
    printf("err:%d\n", errno);
  }
  _out->map_begin = p;
  _out->map_end = (char *)((size_t)p + map_size);
}

void apply(map_info *_in, patch_info *_pi, map_info *_out) {

  char *p = _in->map_begin;
  char *q = _out->map_begin;
  patch_format *_pf = (patch_format *)_pi->begin;
  int i;
  uint8_t *end;
  uint8_t *e;
  for (; p != _in->map_end || q != _out->map_end; p++, q++) {
    if (p == _pf->addr + _in->map_begin) {
      // editing
      switch (_pf->cmd) {
      case 1:
        end = _pf->elem + _pf->length;
        e = _pf->elem;
        for (; e != end; e++, q++) {
          *q = *e;
        }
        break;
      case 0:
        for (i = 0; i < _pf->length; p++, i++)
          ;
        continue;
      default:
        printf("apply error!\n");
        break;
      }
      *q = *p;
      if (_pf != _pi->end)
        _pf++;
    }
  }
}

char save(map_info *p) {
  if (msync(p->map_begin, p->map_end - p->map_begin, MS_SYNC) != 0) {
    printf("save error!\n");
    exit(1);
  }
  return 1;
}

void apply_patch(const char *in_f, patch_info *edit, const char *out_f) {

  map_info _in;
  map_info _out;
  map_input_file(in_f, &_in);
  uint64_t output_file_size =
      (uint64_t)(_in.map_end - _in.map_begin) + edit->size_updown;
  map_output_file(out_f, output_file_size, &_out);
  apply(&_in, edit, &_out);
  char res = save(&_out);
  if (res) {
    printf("%s was saved.", out_f);
  }
}

void show_usage() {
  const char *usage = "1st argument:original file\n"
                      "2nd argument:patch file\n"
                      "3rd argument:output file\n";
  printf("%s", usage);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    show_usage();
  }
  patch_info edit;
  read_patch(argv[2], &edit);
  apply_patch(argv[1], &edit, argv[3]);
}
