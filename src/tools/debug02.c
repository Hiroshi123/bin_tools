
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "alloc.h"
#include "logger.h"
#include "objformat.h"
#include "os.h"

static int A = 0;
static uint8_t B = 0;
static int C = 0;
static uint8_t D = 0;

extern void* read_elf(char*);
extern void* _inst_f1();
extern void* _get_rip();
extern void _set_rip(void*);
extern void _exec_one();
extern void* _exec();
extern void* _get_context();
extern void* _get_rax();

extern void* _get_a();
extern void* _get_tmp01();
/* extern void* _rax; */

/* extern void read_elf(uint8_t*); */

/* static char Inst[] = { */
/*   "add", */
/*   "or", */
/*   "adc", */
/*   "sbb", */
/*   "and", */
/*   "sub", */
/*   "xor", */
/*   "cmp", */

/* } */

void start(int argc, char* argv) {
  logger_init("misc.log");
  char* s = "koi";
  logger_emit("misc.log", s);
  char* s2 = "cdacdcdc";
  set_va(va, 6);
  logger_emit3("misc.log", "%d,efghigk\n", va);
  mem_init();

  int size = 4096;
  __malloc(10);
  __os__write(1, "ok!", 4);

  // printf("a:%s\n", argv[1]);
  int i = 0;
  char* p = argv;
  for (; *argv; argv++, i++)
    ;
  __os__write(1, p, i);
  argv++;
  p = argv;

  i = 0;
  for (; *argv; argv++, i++)
    ;
  __os__write(1, p, i);

  /* int fd = __os__open(p, O_RDONLY, 0777); */
  /* if (fd == 0) { */
  /*   return; */
  /* } */
  /* uint32_t header_size = 0; */
  /* enum OBJECT_FORMAT o = detect_format(fd, &header_size); */
  /* if (o) { */
  /*   __os__write(1, "ok\n", 4); */
  /*   __os__write(1, "ok\n", 4); */
  /* } */
  // __os__exit(0);
  char* fname = p;
  uint8_t* p1 = alloc_file(fname);
  void* begin = read_elf(p1);

  /* logger_emit_p(p1); */
  /* logger_emit_p(*p1); */

  _set_rip(begin);

  /* _inst_f1(); */

  Context* c = _get_context();
  logger_emit_p(c->dflag);

  uint64_t** op_table = c->opcode_table;

  /* uint64_t* op_table2 = *((uint64_t*)c + 1); */
  /* uint64_t* op_table3 = *((uint64_t*)c + 2); */
  /* uint64_t* op_table4 = *((uint64_t*)c + 3); */

  /* uint64_t* op1 = *op_table; */
  /* uint64_t* op2 = *(op_table+1); */
  /* uint64_t* op3 = *(op_table+2); */
  /* uint64_t* op4 = *(op_table+3); */
  // logger_emit_p(op_table[0]);
  // logger_emit_p(op_table[1]);

  /* logger_emit_p(op1); */
  /* logger_emit_p(op2); */
  /* logger_emit_p(op3); */
  /* logger_emit_p(op4); */

  // void* t = _rax;
  // logger_emit_p(op4);
  /* logger_emit_p(op_table2); */
  /* logger_emit_p(op_table3); */
  /* logger_emit_p(op_table4); */

  Reg* v = _get_rax();
  logger_emit_p(v);
  logger_emit_p(v->rax);

  if (v->rax == 0x7) {
    __os__write(1, "ok!!\n", 6);
  } else {
    return;
  }
  // __os__write(1, "ok!", 4);

  logger_emit_p(&v->rip);
  logger_emit_p(begin);

  logger_emit("misc.log", "----\n");

  //
  uint64_t* e = _exec();
  logger_emit_p(v->rip);
  logger_emit_p(*(uint64_t*)v->rip);

  _exec();
  _exec();
  /* _exec(); */

  logger_emit_p(v->rip);
  logger_emit_p(*(uint64_t*)v->rip);

  // logger_emit_p(e);

  //
  // uint64_t* e2 = _inst_f1();

  /* uint64_t* o = &c->opcode_table; */
  /* logger_emit_p(c->opcode_table); */
  /* int ii = 0; */
  // uint64_t* pp = _get_tmp01();
  /* for (;ii < 20;o++,ii++) { */
  /*   logger_emit_p(*o); */
  /*   if (*o) { */
  /*     uint64_t* tt = (uint8_t*)o + 4; */
  /*     logger_emit_p(tt); */
  /*     logger_emit_p(*tt); */
  /*   } */
  /* } */

  /* logger_emit_p(c->rex); */
  /* logger_emit_m(c->data_prefix); */
  /* logger_emit_p(c->addr_prefix); */
  /* logger_emit_p(c->repz); */
  /* logger_emit_p(c->repnz); */

  // Context*

  /* logger_emit_p(pp); */
  /* logger_emit_p(*pp); */

  /* logger_emit_p(&c->dflag); */
  /* logger_emit_p(c->opcode); */

  // logger_emit_p(*pp);

  // logger_emit_d(c->dflag);
  // logger_emit_p(&c->opcode);
}
