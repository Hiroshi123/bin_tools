
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"
#include "objformat.h"
#include "pe.h"
#include "x86_emu.h"

extern void* alloc_file(char*);
extern void gen_elf();
extern void set_virtual_address(void* arg1);
extern void set_program_header(void* arg1);
// extern void do_reloc(void* _oc, void* arg1);

static void do_pe() {
  void* ocp = 0;
  // __z__obj__run_through_coff_shdr3(scr, &__z__link__coff_section_callback,
  // &ocp);
}

typedef struct {
  // these are the parameters which are filled by command line
  int file_format;
  char* stub_path;
  char* loader_path;
  char* exe_path;
  // these parameters are used internally on this program
  int stub_size;
  int loader_size;
  int exe_size;
  //
  uint8_t* stub_p;
  uint8_t* loader_p;
  uint8_t* exe_p;
  uint8_t* ob_loader_p;
  uint8_t* ob_exe_p;
} ZOB_Config;

static ZOB_Config Conf = {};

#define USUAGE                          \
  "usage\n"                             \
  "minimum GNU make\n"                  \
  "-f : specify a build file\n"         \
  "-stub : specify stub location\n"     \
  "-loader : specify loader location\n" \
  "1st : specify a target name\n\n"

static void* read_cmdline(int argc, char** argv) {
  uint8_t file_num = argc;
  if (argc == 1) {
    return 0;
  }
  uint8_t i = 1;
  size_t* p = __malloc(sizeof(void*) * (argc));
  __z__logger_emit_p(p);
  for (; i < argc; i++) {
    if (!__z__std__strcmp(argv[i], "-stub")) {
      __z__logger_emit("misc.log", argv[i + 1]);

      Conf.stub_path = argv[i + 1];
      i++;
    } else if (!__z__std__strcmp(argv[i], "-loader")) {
      Conf.loader_path = argv[i + 1];
      i++;
      __os__write(1, "fttttt!\n", 8);
    } else if (!__z__std__strcmp(argv[i], "-exe")) {
      Conf.exe_path = argv[i + 1];
      i++;
    }
  }
  return 0;
}

void f1() {}

static uint8_t INST[20] = {};

static load_loader() {
  if (Conf.loader_path) {
    // Conf.loader_path = 0;
    /* void* loader_p = __z__mem__alloc_file(Conf.loader_path); */
    /* int size = __z__mem__get_file_size(loader_p); */
    __z__logger_emit("misc.log", "xx!");

    int size = 0;
    void* p = __z__mem__alloc_file_with_size(Conf.loader_path, &size);

    // return p;

    __z__logger_emit_p(p);
    __z__logger_emit_p(size);

    __z__logger_emit("misc.log", "hei");

    for (;;)
      ;

    // __z__logger_emit_p(format);
  } else {
    __z__logger_emit("misc.log", "eee");
  }
}

static load_exe() {
  if (Conf.exe_path) {
    int size = 0;
    void* p = __z__mem__alloc_file_with_size(Conf.exe_path, &size);
    // return p;

    __z__logger_emit_p(p);
    __z__logger_emit_p(size);

  } else {
  }
}

void start(char** argv) {
  int argc = *argv++;
  if (argc == 1) {
    char* str = USUAGE;
    __os__write(1, str, sizeof(USUAGE));
    return;
  }

  __z__logger_init("misc.log");
  __z__mem_init();

  read_cmdline(argc, argv);

  load_loader();

  load_exe();

  // enum OBJECT_FORMAT format = __z__obj__detect_format_fname(fname, 0);
  for (;;)
    ;

  char* fname = argv[1];
  char* start_p = 0;
  // __os__write(1, "format\n", fname);
  enum OBJECT_FORMAT format = __z__obj__detect_format_fname(fname, 0);
  __z__logger_emit_p(format);

  ObjectChain* ocp;
  int code_len = 0;
  switch (format) {
    case PE32:
    case PE64: {
      break;
    }
    // hash
    // iteration script(inside loop, hash function, return offset from ntdll)
    // find ntdll

    // 1. ntdll access
    // 2. read export directory entry
    // 3. compute hash

    // 1. add payload
    // 2. add symbol_table can be dt(gnu)_hash

    // void* scr = alloc_file();
    break;
    case ELF32:
      // break;
    case ELF64: {
      //
      // __z__obj__load_elf(fname, 0);

      void* scr = __z__mem__alloc_file(fname);
      // void* scr = alloc_file(fname);
      // this should be done as concurrent as it could be in the end.
      int i = 0;
      ocp = 0;
      run_through_elf_shdr3(scr, &_on_elf_section_callback, &ocp);
      for (;;)
        ;
      run_through_elf_symtable3(
          ocp->symbol_table_p,
          ((Elf64_Sym*)ocp->symbol_table_p) + ocp->symbol_num,
          &__z__link__on_elf_symtab_callback_for_link, ocp->str_table_p);

      break;
    }
    default:
      __os__write(1, "format\n", 8);
      for (;;)
        ;
      break;
  }

  return;
}
