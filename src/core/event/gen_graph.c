

#include <stdio.h>
#include <stdint.h>
#include "memory.h"
#include "objformat.h"

extern uint64_t EXPORT(rip);
extern uint8_t EXPORT(objformat);
extern uint64_t EXPORT(meta_page_head);
extern uint64_t EXPORT(out_page_head);
extern uint64_t EXPORT(out_page_ptr);
extern uint64_t EXPORT(current_fname);

void add_edge(uint64_t next) {
  
  printf("////////////////////////////////////////////////\n");
  printf("rip:%x,%x\n",rip, next);

  // get meta_page_head
  printf("meta:%x,%x\n", EXPORT(meta_page_head),EXPORT(objformat));
  printf("out:%x,%x\n", EXPORT(out_page_head), EXPORT(out_page_ptr));
  uint8_t* offset = EXPORT(out_page_ptr);

  memcpy(EXPORT(out_page_ptr),EXPORT(current_fname),strlen(EXPORT(current_fname)));
  offset += strlen(EXPORT(current_fname));
  
  const char* a1 = " -> ";
  memcpy(offset, a1, strlen(a1));
  offset += strlen(EXPORT(a1));

  char* name = check_fname(EXPORT(meta_page_head), next, EXPORT(objformat));
  if (!name) {
    name = "tmp_f01";
  }
  memcpy(offset, name, strlen(name));
  offset += strlen(EXPORT(name));
  
  const char* a2 = "\n";
  memcpy(offset, a2, strlen(a2));
  offset += strlen(EXPORT(a2));
  
  /* char* name = get_name_of_f_on_macho64(next); */
  printf("%s\n",name);
  EXPORT(current_fname) = name;
  
  p_host host_rip = get_host_addr(rip);
  printf("host rip:%lx\n",host_rip);
  p_host host_head = get_host_head(rip);
  printf("host head:%lx\n",host_head);
  
  // from given rio,
  // you should know if the mapping contains information about symbol.
  // if it has, then,
  char in;
  // you have the address which are about to call.
  
  const char* a3 = "}";
  memcpy(offset, a3, strlen(a3));
  EXPORT(out_page_ptr) = offset;
  read(0, in, sizeof(in));
  
}




