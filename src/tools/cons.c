
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

char* ADD = "add rax, rax";
char* SUB = "sub rax, rax";
char* EQ = "cmp rax, rax";

typedef struct _Node Node;

struct Node {
  uint8_t type;
  union {
    uint8_t op;
    uint8_t val;
  } kind;
  Node* left;
  Node* right;
};

char* SYS_READ = {
    "mov rax,0", "mov rdi,0", "lea rsi,[d2]", "mov rdx,0x10", "syscall",
};

int main(int argc, char** argv) {
  printf("%s\n", argv[0]);

  char* fname = "ex01.asm";
  void* f1 = open__(fname, O_CREAT | O_WRONLY | O_TRUNC);
  uint8_t* b = SYS_READ;
  /* for (;*b;b++) { */
  /* } */
  char* text = "\tsection .text";
  __os__write(f1, b, strlen(b));
  // printf("%s\n",A);
  // printf("%s\n");
}
