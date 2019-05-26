

#include <stdio.h>
#include <stdint.h>
#include "memory.h"

extern uint64_t EXPORT(rip);

void add_edge(uint64_t diff) {

  printf("////////////////////////////////////////////////\n");
  printf("rip:%x,%x\n",rip + diff, diff);
  p_host host_rip = get_host_addr(rip);  
  printf("host rip:%lx\n",host_rip);
  p_host host_head = get_host_head(rip);
  printf("host head:%lx\n",host_head);
  
  // from given rio,
  // you should know if the mapping contains information about symbol.
  // if it has, then, 
  printf("%lx\n",diff);
  char in;
  // you have the address which are about to call.
  read(0, in, sizeof(in));
  
}




