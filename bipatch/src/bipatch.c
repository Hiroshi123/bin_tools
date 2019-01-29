
/*
  This is a short script which will apply patch to an input binary file.  
  patch file format is defined in its own way.
*/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include "bipatch.h"

static uint64_t ascii_to_bin(const char *begin, const char *end) {
  char *x = (char *)begin;
  uint64_t r = 0;
  char s = 0;
  if (*x == '0' && *(x + 1) == 'x') {
    for (; x != end; x++);
    x--;
    for (; x != begin+1; x--, s += 4) {
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

char read_patch(const char *fname, patch_info *info) {

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
  for (; p != end && *p != '\n';p++,q++) {
    if (*p == '1') {
        q->cmd = 1;	
    } else if (*p == '0') {
      q->cmd = 0;
    } else {
      return 0;
    }
    for (p++; *p != ' '; p++);
    p++;
    addr = ascii_to_bin(p, p + 10);
    if (addr == (uint64_t)-1) {
      return 0;
    }
    q->addr = addr;
    // next
    for (p+=10; *p == ' '; p++);
    if (q->cmd == 0) {
      d = ascii_to_bin(p, p + 4);
      if (d == (uint64_t)-1) {
	return 0;
      }
      p+=4;
      q->length = d;
      q->elem = 0;
      info->size_updown -= d;
    } else if (q->cmd==1) {
      q->elem = (uint8_t *)data;
      for (; *p != '\n'; data++, q->length++) {
	d = ascii_to_bin(p, p + 4);
	if (d == (uint64_t)-1) {
	  return 0;
	}
	*data = d;
	for (p+=4; *p == ' '; p++);
      }
      info->size_updown += q->length;
    }
  }
  munmap(q, map_size);
  info->end = (patch_format *)p;
  return 1;
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
  _p->map_end = (char *)((size_t)p + stbuf.st_size);
}

void map_output_file(const char *fname, uint64_t map_size, map_info *_out) {

  int fd = open(fname, O_RDWR | O_CREAT | O_TRUNC);
  if (fd == -1) {
    printf("error:%d\n",errno);
    exit(1);
  }
  const char c = 0x00;
  int PAGE_SIZE = 4096;
  // const size_t offset = COUNT * PAGE_SIZE;
  lseek(fd, map_size, SEEK_SET);
  write(fd, &c, sizeof(char));
  lseek(fd, 0, SEEK_SET);
  // int map_size=4096;
  map_size = (map_size + 0x1000) & 0xfffff000;
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
  for (; p != _in->map_end && q != _out->map_end;) {
    if (p == _pf->addr + _in->map_begin) {
      // editing
      switch (_pf->cmd) {	
      case 1:
	end = _pf->elem + _pf->length;
	for (e = _pf->elem; e != end; e++, q++) *q = *e;	
        break;
      case 0:
	for (i = 0; i < _pf->length; p++, i++);
        break;
      default:
        printf("apply error!\n");
        break;
      }
      if (_pf != _pi->end) _pf++;
    } else {
      *q = *p;
      p++, q++;      
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
      (uint64_t)(_in.map_end - _in.map_begin - 1) + edit->size_updown;
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
  if (!read_patch(argv[2], &edit)) {
    printf("parse error\n");
  }
  apply_patch(argv[1], &edit, argv[3]);
}
