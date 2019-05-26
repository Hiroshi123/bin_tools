
#include <stdio.h>
#include <stdint.h>

uint64_t ascii_to_bin(const char *begin, const char *end) {
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
        r += (*x - 'a' + 10) << s;
      } else {
        printf("parse error\n");
        return -1;
      }
    }
    return r;
  }
  return -1;
}

void hex_text_save(char** p, char v) {  
  (*p)->data = 0x20;// which means space
  *p++;
  (*p)->data = 0x30;// which means 0
  *p++;  
  (*p)->data = 0x78;// which means x
  *p++;  
  (*p)->data = v / 0x10;
  *p++;  
  (*p)->data = v % 0x10;  
}


