
#include <stdint.h>
#include <stdio.h>

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
  int file_format;
} ZOB_Config;

static ZOB_Config Conf = {};

#define USUAGE                  \
  "usage\n"                     \
  "minimum GNU make\n"          \
  "-f : specify a build file\n" \
  "1st : specify a target name\n\n"

static uint8_t INST[20] = {};

void start(char** argv) {
  int argc = *argv++;
  if (argc == 1) {
    char* str = USUAGE;
    __os__write(1, str, sizeof(USUAGE));
    return;
  }
  char* fname = argv[1];
  char* start_p = 0;
  enum OBJECT_FORMAT format = __z__obj__detect_format_fname(fname, 0);
  logger_emit_p(format);

  ObjectChain* ocp;
  int code_len = 0;
  switch (format) {
    case PE32:
    case PE64: {
      _set_objformat(format);
      if (format == PE32) {
        _set_arch(1);
      } else {
        _set_arch(2);
      }
      info_on_pe info = {};
      size_t* s = __z__obj__load_pe(fname, &info);
      logger_emit("misc.log", "image base:\n");
      logger_emit_p(info.this_image_base);
      logger_emit_p(info.image_base);
      logger_emit_p(s);
      // start_p = s;
      start_p = info.this_image_base + info.base_of_code;
      __z__emu__x86__set_emu_base(info.this_image_base);
      __z__emu__x86__set_image_base(info.image_base);
      _set_rip(start_p);
      code_len = info.size_of_code;
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
      for (;;)
        ;
      void* scr = __z__mem__alloc_file(fname);
      // void* scr = alloc_file(fname);
      // this should be done as concurrent as it could be in the end.
      int i = 0;
      ocp = 0;
      run_through_elf_shdr3(scr, &_on_elf_section_callback, &ocp);
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
  // __z__obs__t01();
  char* rip = _get_rip();
  _set_pre_rip(rip);
  logger_emit_p(rip);
  int i = 0;
  char* end = rip + code_len + 0x1000;

  logger_init("instruction.log");
  // logger_emit("instruction.log", "aaa\n");

  for (; rip < end; i++) {
    _exec();
    __z__x86__dis();
    _clear_state();
    rip = _get_rip();
    _set_pre_rip(rip);
    logger_emit_p(rip);
  }

  // rip = _get_rip();
  /* uint32_t buf[4] = {};   */
  /* size_t a = _get_cpu_id(0, &buf); */
  /* logger_emit_p(buf[0]); */
  /* logger_emit_p(buf[1]); */
  /* logger_emit_p(buf[2]); */
  /* logger_emit_p(buf[3]); */
  /* logger_emit_p(a); */
  /* logger_emit_p(1); */
  /* for (;;); */

  // _exec();

  return;
}
