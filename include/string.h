
#include <stdint.h>

uint8_t __z__std__strcmp(char* p1, char* p2);
int __z__std__strlen(char* p);
void* __z__std__strtok(uint8_t* p);

#define strcmp __z__std__strcmp
#define strlen __z__std__strlen
#define _strtok __z__std__strtok

/* static uint8_t strcmp(char* p1, char* p2) { */
/*   for (;*p1 && *p2 && *p1 == *p2;p1++,p2++); */
/*   return *p1 - *p2; */
/* } */

/* static int strlen(char* p) { */
/*   int i; */
/*   for (i=0;*p;p++,i++); */
/*   return i; */
/* } */

/* static void* _strtok(uint8_t* p) { */

/*   uint8_t has = 0; */
/*   for (;;p++) { */
/*     if (*p==0x20) { */
/*       *p = 0x0; */
/*       return p + 1; */
/*     } */
/*     if (*p==0x0) { */
/*       return has; */
/*     } */
/*     has = 1; */
/*   } */
/*   return 0; */
/* } */

