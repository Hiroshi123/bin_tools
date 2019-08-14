
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

/* typedef struct { */
/*     u1 tag; */
/*     u1 info[]; */
/* } cp_info; */

/* typedef struct { */
/*     u4             magic; */
/*     u2             minor_version; */
/*     u2             major_version; */
/*     u2             constant_pool_count; */
/*     cp_info        constant_pool[constant_pool_count-1]; */
/*     u2             access_flags; */
/*     u2             this_class; */
/*     u2             super_class; */
/*     u2             interfaces_count; */
/*     u2             interfaces[interfaces_count]; */
/*     u2             fields_count; */
/*     field_info     fields[fields_count]; */
/*     u2             methods_count; */
/*     method_info    methods[methods_count]; */
/*     u2             attributes_count; */
/*     attribute_info attributes[attributes_count]; */
/* } classFile; */

/* typedef struct { */
/*     u1 tag; */
/*     u2 class_index; */
/*     u2 name_and_type_index; */
/* } CONSTANT_Methodref_info; */

#define CONSTANT_Utf8 0x01

#define CONSTANT_Integer 0x03
#define CONSTANT_Float 0x04
#define CONSTANT_Long 0x05
#define CONSTANT_Double 0x06

#define CONSTANT_Class 0x07
#define CONSTANT_String	0x08
#define CONSTANT_Fieldref 0x09
#define CONSTANT_Methodref 0x0a
#define CONSTANT_InterfaceMethodref 0x0b
#define CONSTANT_NameAndType 0x0c

#define CONSTANT_MethodHandle	0x0f
#define CONSTANT_MethodType	0x10
#define CONSTANT_InvokeDynamic  0x12

#define aload 0x19
#define aload0 0x2a
#define aload1 0x2b
#define aload2 0x2c
#define aload3 0x2d


// 1. convert all of endian from big to little
// 2. set the map of index and pointer to the addresss of the element
// of constant pool.

char* read_constant_pool(uint8_t* p, uint64_t* h) {
  
  // the all of the pointer of the constant pool element needs to be set.
  uint16_t c = (*p)*16+*(p+1);
  uint16_t v16;
  uint32_t v32;
  uint64_t v64;
  char* e;
  char s = 0;
  uint8_t i=1;
  for (p+=2;i<c;i++,h++) {
    *h = p;
    printf("p:%x,%x\n",p,*p);
    switch (*p) {
    case CONSTANT_Utf8: {
      p++;
      v16 = (*p)*16+*(p+1);
      *(uint16_t*)p = v16;
      printf("p:%x\n",p);
      uint8_t* t1 = p+2;
      uint8_t* t2 = p+2+v16;      
      for (;t1!=t2;t1++) {
	printf("%c",*t1);
      }
      printf("\n");
      p+=2+v16;
      break;
    }
    case CONSTANT_Integer:
    case CONSTANT_Float:
      for (p++,e = p+4,v32=0;p!=e;p++,s+=4)
	v32 += *p << s;
      *(uint32_t*)(p-4) = v32;
      break;
    case CONSTANT_Long:
    case CONSTANT_Double:
      for (p++,e = p+8,v64=0;p!=e;p++,s+=8)
	v64 += *p << s;
      *(uint64_t*)(p-8) = v64;
      break;
      // consume 2byte
    case CONSTANT_Class:
    case CONSTANT_String:
    case CONSTANT_MethodType:
      p++;
      v16 = *p*16+*(p+1);
      *(uint16_t*)p = v16;
      p+=2;
      break;
      // consume 3byte
    case CONSTANT_MethodHandle:
      p++;
      // reference kind
      p++;
      // reference_index
      v16 = *p*16+*(p+1);
      *(uint16_t*)p = v16;
      p+=2;
      break;
      // consume 4byte
    case CONSTANT_Fieldref:
    case CONSTANT_Methodref:
    case CONSTANT_InterfaceMethodref:
    case CONSTANT_NameAndType:
    case CONSTANT_InvokeDynamic:
      p++;
      v16 = *p*16+*(p+1);
      *(uint16_t*)p = v16;
      p+=2;
      v16 = *p*16+*(p+1);
      *(uint16_t*)p = v16;    
      p+=2;
      break;
    default:
      printf("non matched\n");
      break;
    }
  }
  printf("end\n");
  return p;
}

void emit_x86() {
  //
  
}

#define NOP 0x00
#define ACONST_NULL 0x01
#define ICONST_M1 0x02
#define ICONST_0 0x03
#define ICONST_1 0x04
#define ICONST_2 0x05
#define ICONST_3 0x06
#define ICONST_4 0x07
#define ICONST_5 0x08
#define LCONST_0 0x09
#define LCONST_1 0x0A

#define LDC 0x12

#define ALOAD_0 0x2a

// ldc -> String = "";

void decode(uint8_t* p, uint8_t* e) {

  for (;p!=e;p++) {
    printf("d:%x\n",*p);
    switch (*p) {
    case ALOAD_0:
      break;
    case LDC:      
      break;
    case LCONST_0:
      break;
    }
  }
  
}

char* read_code_attribute(uint8_t* p) {
  
  printf("max stack:%x\n",(*p)*16+*(p+1));
  p+=2;
  printf("max local:%x\n",(*p)*16+*(p+1));
  p+=2;
  int s;
  int code_len;
  for (code_len=0,s=12;s>=0;p++,s-=4) {	
    code_len += *p << s;
  }
  printf("code len:%x\n",code_len);
  decode(p,p+code_len);
  p+=code_len;
  uint16_t exception_len = (*p)*16+*(p+1);
  printf("exception:%x\n",exception_len);
  p+=2;
  p+=exception_len;
  uint16_t attr_len = (*p)*16+*(p+1);
  printf("attr:%x\n",attr_len);
  /* p+=2; */
  /* printf("%x\n",(*p)*16+*(p+1)); */
  
  return p;
  /* p+=2; */
  /* for (attr_len=0,s=12;s>=0;p++,s-=4) {	 */
  /*   code_len += *p << s; */
  /* } */
}

uint8_t* read_line_number_table_attribute(uint8_t* p) {
  uint16_t len = (*p)*16+*(p+1);
  p+=2;
  /* printf("len::%x\n",len); */
  uint8_t* e = p+len*4;
  for (;p!=e;p++) {
    /* printf("%x,%x\n",p,*p); */
  }
  return 0;
}

uint8_t* read_stackmap_table_attribute(uint8_t* p) {
  uint16_t len = (*p)*16+*(p+1);
  printf("number of entries:%x\n",len);
  p+=2;
  printf("v:%x\n",*p);
  return 0;
}

typedef struct {
  uint16_t inner_class_info_index;
  uint16_t outer_class_info_index;
  uint16_t inner_name_index;
  uint16_t inner_class_access_flags;
} innerClass;


typedef struct {
  u2 access_flags;
  u2 name_index;
  u2 descriptor_index;
  u2 attributes_count;
  // attribute_info attributes[attributes_count];
} field_info;

/* typedef struct { */
/*   u2 bootstrap_method_ref; */
/*   u2 num_bootstrap_arguments; */
/*   u2 bootstrap_arguments[num_bootstrap_arguments]; */
/*   uint16_t inner_class_info_index; */
/*   uint16_t outer_class_info_index; */
/*   uint16_t inner_name_index; */
/*   uint16_t inner_class_access_flags; */
/* } BootstrapMethods; */

uint8_t* read_inner_class_attribute(uint8_t* p) {
  uint16_t len = (*p)*16+*(p+1);
  printf("number of classes:%x\n",len);
  p+=2;
  innerClass* q = (innerClass*)p;
  innerClass* e = q+len;
  for (;q!=e;q++) {
    printf("%x\n",q->inner_class_info_index);
    printf("%x\n",q->outer_class_info_index);
    printf("%x\n",q->inner_name_index);
    printf("%x\n",q->inner_class_access_flags);
  }
  return 0;
}

uint8_t* read_source_file_attribute(uint8_t* p) {
  return 0;
}

uint8_t* read_bootstrap_methods_attribute(uint8_t* p) {
  uint16_t len = (*p)*16+*(p+1);  
  printf("dd:%x\n",len);
  p+=2;
  printf("dd:%x\n",(*p)*16+*(p+1));
  p+=2;
  printf("dd:%x\n",(*p)*16+*(p+1));
  p+=2;
  printf("dd:%x\n",(*p)*16+*(p+1));
  p+=2;
  printf("dd:%x\n",(*p)*16+*(p+1));
  p+=2;
  printf("dd:%x\n",(*p)*16+*(p+1));
  
  return 0;
}

uint8_t* read_attribute(uint8_t* t, uint8_t* p) {

  if(!strncmp("Code", t, 4)) {
    p = read_code_attribute(p);
  } else if (!strncmp("SourceFile", t, 10)) {
    p = read_source_file_attribute(p);
  } else if (!strncmp("LineNumberTable", t, 15)) {
    printf("line!\n");
    p = read_line_number_table_attribute(p);
  } else if (!strncmp("StackMapTable", t, 13)) {
    p = read_stackmap_table_attribute(p);
  } else if (!strncmp("InnerClasses", t, 12)) {
    p = read_inner_class_attribute(p);
  } else if (!strncmp("BootstrapMethods", t, 16)) {
    p = read_bootstrap_methods_attribute(p);
  } else {
    p = 0;
    printf("error\n");
  }
  return p;
}

char* read_attribute_info(uint8_t* p, uint64_t* h) {
  int j,k,attr_len;
  char s = 0;
  int attr_c = (*p)*16+*(p+1);
  for (p+=2,j=0;j<attr_c;j++) {
    printf("p..:%x,%x\n",p,attr_c);
    int idx = (*p)*16+*(p+1);
    printf("idx:%d\n",idx);
    char* name;  
    uint8_t* tmp = (char*)*(h+idx-1);
    if (*tmp == CONSTANT_Utf8) {
      printf("vv:%x\n",*((uint16_t*)(tmp+1)));
      tmp+=1;
      int len = *((uint16_t*)(tmp));
      tmp+=2;
      char* end = tmp+len;
      name = tmp;
      for (;tmp!=end;tmp++) {
	printf("%c",*tmp);
      }
      printf("\n");
    }
    p+=2;
    for (attr_len=0,s=12;s>=0;p++,s-=4) {
      attr_len += *p << s;
    }
    printf("attrlen.%x\n",attr_len);
    uint8_t* ee = p+attr_len;
    p = read_attribute(name, p);
    if (p == 0) {
      p = ee;
    } else if (p != ee) {
      p = read_attribute_info(p,h);
      printf("z:%x,%x\n",p,ee);
      p = ee;
    }    
  }
  printf(".....\n");
  return p;
}

char* read_interface_info(char* p, char* h) {
  u2 len = (*p)*16+*(p+1);
  printf("interface:%x\n",len);
  p+=2;
  // contents of interface ref
  //for (;;p+=2);
  p+=len*2;
  return p;
}

char* read_field_info(char* p, uint64_t* h) {
  int len = (*p)*16+*(p+1);
  printf("fc:%p\n",len); 
  int i=0;
  int j;
  for (p+=2;i<len;i++) {
    printf("hhhh\n");
    printf("%x\n",(*p)*16+*(p+1));
    p+=2;
    printf("%x\n",(*p)*16+*(p+1));
    p+=2;
    printf("%x\n",(*p)*16+*(p+1));
    p+=2;
    printf("%x\n",(*p)*16+*(p+1));
    uint16_t attr_len = (*p)*16+*(p+1);
    for (j=0;j<attr_len;j++) {
      printf("read\n");
      read_attribute_info(p, h);
    }
    printf("hhhh!\n");
    p+=2;
  }
  return p;
}

char* read_method_info(char* p, uint64_t* h) {
  int method_c = (*p)*16+*(p+1);
  int i=0;
  for (p+=2;i<method_c;i++) {
    p+=6;
    printf("p:%x\n",p);
    p = read_attribute_info(p, h);
  }
  return p;
}

#define DEBUG 1

char* read_header(char* p) {
  if (*(uint32_t*)p == 0xbebafeca) {
    p+=4;
#ifdef DEBUG
    printf("minor version:%x\n",p);
    printf("major version:%x\n",p+2);
#endif
    p+=4;
    return p;
  }
  return 0;
}

void read_classfile(uint8_t* p) {

  int PAGE_SIZE = 4096;
  uint64_t *h = (char*)
    mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
	 MAP_SHARED|MAP_ANONYMOUS, -1, 0);
  p = read_header(p);
  p = read_constant_pool(p, h);
  p+=6;
  p = read_interface_info(p, h);
  p = read_field_info(p, h);
  p = read_method_info(p, h);
  p = read_attribute_info(p, h);
  printf("%x\n",p);
}


