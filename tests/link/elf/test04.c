
#define _GNU_SOURCE
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern int main();

void _init(){
  printf("inii\n");
}
void _fini(){
  printf("fini\n");
}

extern void __libc_start_init(void);

/* __attribute__((constructor)) */
/* void init_f1() { */
/*   printf("init called\n"); */
/* } */

/* __attribute__((constructor)) */
/* void init_f2() { */
  
/* } */

static int INIT_ITERATION = 1;

static int callback(struct dl_phdr_info *info, size_t size, void *data) {

  // printf("callback\n", info->);
  printf("Name: \"%s\" (%d segments)\n", info->dlpi_name, info->dlpi_phnum);

  if (INIT_ITERATION == 0)
    return 0;

  INIT_ITERATION = 0;
  
  char *type;
  int p_type;
  uint8_t* p;
  
  for (int j = 0; j < info->dlpi_phnum; j++) {
    p_type = info->dlpi_phdr[j].p_type;
    type =  (p_type == PT_LOAD) ? "PT_LOAD" :
      (p_type == PT_DYNAMIC) ? "PT_DYNAMIC" :
      (p_type == PT_INTERP) ? "PT_INTERP" :
      (p_type == PT_NOTE) ? "PT_NOTE" :
      (p_type == PT_INTERP) ? "PT_INTERP" :
      (p_type == PT_PHDR) ? "PT_PHDR" :
      (p_type == PT_TLS) ? "PT_TLS" :
      (p_type == PT_GNU_EH_FRAME) ? "PT_GNU_EH_FRAME" :
      (p_type == PT_GNU_STACK) ? "PT_GNU_STACK" :
      (p_type == PT_GNU_RELRO) ? "PT_GNU_RELRO" : NULL;
    
    printf("    %2d: [%14p; memsz:%7jx] flags: %#jx; ", j,
	   (void *) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr),
	   (uintmax_t) info->dlpi_phdr[j].p_memsz,
	   (uintmax_t) info->dlpi_phdr[j].p_flags);
    p = info->dlpi_addr + info->dlpi_phdr[j].p_vaddr;
    if (p_type == PT_LOAD) {
      printf("%p,%p\n", p, *(size_t*)p);
    }
    if (type != NULL)
      printf("%s\n", type);
    else
      printf("[other (%#x)]\n", p_type);
  }  
  return 0;
}

void _start_c(long *p) {
  int argc = p[0];
  char **argv = (void *)(p+1);

  printf("aaa,%p\n", main);

  dl_iterate_phdr(callback, NULL);
  
  for (;;);
  
  
  __libc_start_main(main, argc, argv, _init, _fini, 0);
  exit(0);
}

