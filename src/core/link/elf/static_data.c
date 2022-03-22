
#include <stdint.h>
#include <stdio.h>

#include "link.h"

Config* Confp;

void static_data_init(void* arg1) {
  Confp = arg1;
  /* config.virtual_address_offset = con->virtual_address_offset; */
  /* printf("static_data_init:%p\n", Confp->virtual_address_offset); */
}
