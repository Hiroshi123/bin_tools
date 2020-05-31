
#include <stdio.h>
#include <stdint.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "alloc.h"
#include "load.h"
#include "os.h"

static int PAGE_SIZE = 0x1000;
// #define PAGE_SIZE 0x1000

#define SET_DEPS\
  void* a1[2] = {};\
  a1[0] = &dsos[0];\
\
  void* a2[3] = {};\
  a2[0] = &dsos[0];\
  a2[1] = &dsos[1];\
\
  void* a3[4] = {};\
  a3[0] = &dsos[0];\
  a3[1] = &dsos[1];\
  a3[2] = &dsos[2];\
\
  dsos[1].deps = &a1;\
  dsos[2].deps = &a2;\
  dsos[3].deps = &a3;\

/* static struct dso ldso; */
/* // this is /src/internal/lic.c */
/* struct __libc __libc; */
/* size_t __hwcap; */
/* static struct dso *builtin_deps[2]; */
/* static struct dso *head, *tail, *fini_head, *syms_tail, *lazy_head; */

static struct dso *load_library(const char *name, struct dso *dso, uint8_t recurse);

static int search_vec(size_t *v, size_t *r, size_t key) {
  for (; v[0]!=key; v+=2)
    if (!v[0]) return 0;
  *r = v[1];
  return 1;
}

static void decode_vec(size_t *v, size_t *a, size_t cnt) {
  size_t i;
  for (i=0; i<cnt; i++) a[i] = 0;
  for (; v[0]; v+=2) {
    __write(1, "f\n", 2);
    if (v[0]-1<cnt-1) {
      a[0] |= 1UL<<v[0];
      a[v[0]] = v[1];
    }
  }
}

static void set_dynamic(size_t* v, struct dso *dso) {
  for (; *v; v+=2) {
    switch (*v) {
    case DT_NEEDED:
      break;
    case DT_STRTAB:
      dso->strings = dso->base + *(v+1);
      break;
    case DT_SYMTAB:
      dso->syms = dso->base + *(v+1);
      break;
    case DT_GNU_HASH:
      dso->ghashtab = dso->base + *(v+1);
      break;
    case DT_HASH:
      dso->hashtab = dso->base + *(v+1);
      break;
    default:
      break;
    }
  }
}

static void *map_library(int fd, struct dso * dso) {

  Ehdr ehdr;
  Phdr phdr;
  size_t l = __read(fd, &ehdr, sizeof(Ehdr));
  Ehdr *eh = &ehdr;
  Phdr *ph, *ph0;
  size_t phsize;
  if (l<0) return 0;
  if (/*l<sizeof *eh ||*/ (eh->e_type != ET_DYN && eh->e_type != ET_EXEC))
    goto noexec;
  phsize = eh->e_phentsize * eh->e_phnum;
  // ph = ph0 = (void *)((char *)ehdr + eh->e_phoff);
  size_t i = 0;
  uint8_t* v;
  int offset = l + phsize;
  for (i=eh->e_phnum; i; i--) {
    l += __read(fd, &phdr, sizeof(Phdr));
    __write(1, "r\n", 2);
    if (phdr.p_type == PT_LOAD) {
      __write(1, "p\n", 2);
      v = __os__mmap
    	(NULL, phdr.p_memsz + (phdr.p_vaddr & PAGE_SIZE-1)
    	 /*PAGE_SIZE*/, PROT_READ | PROT_WRITE | PROT_EXEC,
    	 MAP_PRIVATE, fd, phdr.p_offset & -PAGE_SIZE);
      dso->base = v;// + offset;
      if (v == -1) {
    	__write(1, "e\n", 2);
      }
    }
    if (phdr.p_type == PT_DYNAMIC) {
      dso->dynv = dso->base + phdr.p_vaddr;
    }
  }
  dso->entry = dso->base + eh->e_entry;
  set_dynamic(dso->dynv, dso);
  __write(1, "o\n", 2);

 noexec:
  return;
}

static uint32_t gnu_hash(const char *s0)
{
  const unsigned char *s = (void *)s0;
  uint_fast32_t h = 5381;
  for (; *s; s++)
    h += h*32 + *s;
  return h;
}

static uint8_t strcmp(char* p1, char* p2) {
  for (;*p1 && *p2 && *p1 == *p2;p1++,p2++);
  return *p1 - *p2;
}

static Sym *gnu_lookup(uint32_t h1, uint32_t *hashtab, struct dso *dso, const char *s)
{
  uint32_t nbuckets = hashtab[0];
  uint32_t *buckets = hashtab + 4 + hashtab[2]*(sizeof(size_t)/4);
  uint32_t i = buckets[h1 % nbuckets];

  if (!i) return 0;

  uint32_t *hashval = buckets + nbuckets + (i - hashtab[1]);

  for (h1 |= 1; ; i++) {
    uint32_t h2 = *hashval++;
    if ((h1 == (h2|1)) && (!dso->versym || dso->versym[i] >= 0)
	&& !strcmp(s, dso->strings + dso->syms[i].st_name))
      return dso->syms+i;
    if (h2 & 1) break;
  }
  return 0;
}

static Sym *gnu_lookup_filtered(uint32_t h1, uint32_t *hashtab, struct dso *dso, const char *s, uint32_t fofs, size_t fmask)
{
  const size_t *bloomwords = (const void *)(hashtab+4);
  size_t f = bloomwords[fofs & (hashtab[2]-1)];
  if (!(f & fmask)) return 0;

  f >>= (h1 >> hashtab[3]) % (8 * sizeof f);
  if (!(f & 1)) return 0;

  return gnu_lookup(h1, hashtab, dso, s);
}

static uint8_t find_sym(struct dso *dso, const char *s, int use_deps, struct symdef* def) {
  // int need_def
  uint32_t h = 0, gh = gnu_hash(s), gho = gh / (8*sizeof(size_t)), *ght;
  size_t ghm = 1ul << gh % (8*sizeof(size_t));
  struct dso **deps = use_deps ? dso->deps : 0;
  // syms_next will determine which one is going to be.
  for (; dso; dso = use_deps ? *deps++ : dso->syms_next) {
    Sym *sym;
    if ((ght = dso->ghashtab)) {
      __write(1, "m\n", 2);
      sym = gnu_lookup_filtered(gh, ght, dso, s, gho, ghm);
    }
    if (sym) {
      __write(1, "good\n", 5);
      def->sym = sym;
      def->dso = dso;
      return 1;
    }
    __write(1, "n\n", 2);
  }
  return 0;
}

static void do_relocs(struct dso *dso, size_t* rel, size_t rel_size, size_t stride) {

  unsigned char *base = dso->base;
  int type;
  Elf64_Rela* r;
  Sym *sym;
  const char *name;
  Sym *syms = dso->syms;
  char *strings = dso->strings;
  int sym_index;
  size_t *reloc_addr;
  size_t sym_val;
  size_t addend;
  struct symdef def;
  for (; rel_size; rel+=stride, rel_size-=stride*sizeof(size_t)) {
    reloc_addr = dso->base + rel[0];
    type = ELF64_R_TYPE(rel[1]);
    sym_index = ELF64_R_SYM(rel[1]);
    addend = rel[2];
    if (sym_index) {
      sym = syms + sym_index;
      name = strings + sym->st_name;
      __write(1, name, strlen(name));
      __write(1, "\n", 1);
      int r = find_sym(dso, name, 1, &def);
      sym_val = def.sym ? def.dso->base + def.sym->st_value : 0;
      // char* tmp = def.dso->base + def.sym->st_value;
    }
    switch (type) {
    case 0:
      break;
    case R_X86_64_GLOB_DAT/*6*/:
      break;
    case R_X86_64_JUMP_SLOT/*7*/:
      if (*reloc_addr) {
	__write(1, "ee\n", 3);
      }
      *reloc_addr = sym_val + addend;
      if (sym_val) {
	__write(1, "g\n", 2);
      }
      break;
    case R_X86_64_RELATIVE/*8*/:
      *reloc_addr = dso->base + addend;
      __write(1, "h\n", 2);
      break;
    default:
      __write(1, "l\n", 2);
      break;
    }
  }
}

static void reloc_all(struct dso *p) {
  void* rel = 0;
  void* rela = 0;
  size_t relsz = 0;
  size_t relasz = 0;
  size_t* v;
  for (; p; p=p->next) {
    if (p->relocated) continue;
    // decode_vec(p->dynv, dyn, 40);
    v = p->dynv;
    if (v == 0) return;
    for (; *v; v+=2) {
      if (*v == DT_REL) {
	rel = *(v+1);
      }
      if (*v == DT_RELA) {
	rela = *(v+1);
      }
      if (*v == DT_RELSZ) {
	relsz = *(v+1);
      }
      if (*v == DT_RELASZ) {
	relasz = *(v+1);
      }
    }
    if (relsz) do_relocs(p, p->base + (size_t)rel, relsz, 2);
    if (relasz) do_relocs(p, p->base + (size_t)rela, relasz, 3);
    // do_relocs(p, laddr(p, dyn[DT_RELA]), dyn[DT_RELASZ], 3);
  }
}

static void load_direct_deps(struct dso *p) {
  for (; p; p=p->next) {
    if (p->relocated) continue;
    // decode_vec(p->dynv, dyn, 40);
    size_t* v = p->dynv;
    for (; *v; v+=2) {
      if (*v == DT_NEEDED) {
	// v + 1;
	// head->
	__write(1, "e\n", 2);
      }
    }
  }
}

static void load_deps(struct dso *p) {
  if (p->deps) return;
  for (; p; p=p->next)
    load_direct_deps(p);
}

static struct dso *load_library(const char *name, struct dso *_dso, uint8_t recurse) {

  int fd = __open(name, O_RDONLY, 0777);
  if (fd == -1) {
    __write(1, "e", 1);
    return 0;
  }
  struct dso* dso;
  if (_dso == 0) {
    dso = __malloc(sizeof(struct dso));
    __write(1, "ok\n", 3);
  } else {
    dso = _dso;
  }
  map_library(fd, dso);
  if (recurse) {
    // load_deps();
  }
  if (dso->dynv) {

  }
  __close(fd);
  reloc_all(dso);
  return dso;
}

static int strlen(char* p) {
  int i;
  for (i=0;*p;p++,i++);
  return i;
}

static void memcpy(char* src, char* dst, int len) {
  int i=0;
  for (;i < len;src++,dst++,i++) {
    *dst = *src;
  }
}

static char** read_argv(char** argv, char** fname) {
  int argc = *argv++;  
  char* own_fname = *argv;
  *argv = argc - 1;
  argv++;
  int f = 0;
  for (;*argv;argv++) {
    if (!f) {
      *fname = *argv;
      f = 1;
    }
    __write(1, *argv, strlen(*argv));
    __write(1, "\n", 1);
  }
  return argv;
}

static char** read_env(char** argv) {
  for (;*argv;argv++);
  return argv;
}

static char** read_zero(char** argv) {
  for (;*argv == 0;argv++);
  return argv;
}

static void read_auxv(char** argv, size_t* aux) {
  Elf64_auxv_t *auxv = 0;
  for (auxv = (Elf64_auxv_t *)argv; auxv->a_type != AT_NULL; auxv++) {
    aux[auxv->a_type] = auxv->a_un.a_val;
    switch (auxv->a_type) {
    case AT_PAGESZ:
      if (PAGE_SIZE == auxv->a_un.a_val) {
	__write(1, "PAGESIZE differ\n", 16);
    	PAGE_SIZE = auxv->a_un.a_val;
      }
      break;
    default:
      break;
    }
  }
  if (!aux[AT_BASE]) aux[AT_BASE] = aux[AT_PHDR] & -4096;
}

static inline set_first_deps(struct dso* dsos) {
  void* a1[2] = {};
  a1[0] = &dsos[0];

  void* a2[3] = {};
  a2[0] = &dsos[0];
  a2[1] = &dsos[1];

  void* a3[4] = {};
  a3[0] = &dsos[0];
  a3[1] = &dsos[1];
  a3[2] = &dsos[2];

  dsos[1].deps = &a1;
  dsos[2].deps = &a2;
  dsos[3].deps = &a3;
}


void __start(void* rsp) {
  char** argv = rsp;
  size_t i, aux[AT_MAX];
  struct dso dsos[4] = {};
  char* fname = 0;
  argv = read_argv(argv, &fname);
  argv = read_zero(argv);
  argv = read_env(argv);
  argv = read_zero(argv);
  read_auxv(argv, &aux);
  // this is how you will be able to know base address when you did not define base address.
  dsos[3].base = aux[AT_BASE];
  Ehdr *eh = (void *)dsos[3].base;
  Phdr *ph = (void *)(dsos[3].base + eh->e_phoff);
  // if (aux[AT_PHNUM] != eh->e_phnum) __write(1, "error\n", 6);
  // if (aux[AT_PHENT] != eh->e_phentsize) __write(1, "error\n", 6);
  size_t phnum = eh->e_phnum;
  size_t phent = eh->e_phentsize;
  while (phnum-- && ph->p_type != PT_DYNAMIC)
    ph = (void *)((size_t)ph + phent);
  Elf64_Dyn* dynv = (void *)(dsos[3].base + ph->p_vaddr);
  dsos[3].dynv = dynv;
  set_dynamic(dynv, &dsos[3]);

  //////////////////////////////////
  // resolve deps
  SET_DEPS
  ///////////////////////////////
  int k = 0;
  char* name = 0;
  for (i=0; dynv->d_tag; dynv++) {
    if (dynv->d_tag == DT_NEEDED) {
      name = dsos[3].strings + dynv->d_un.d_val;
      __write(1, name, strlen(name));
      __write(1, "\n", 1);
      load_library(name, &dsos[k], 0);
      k++;
    }
  }
  if (dsos[3].dynv) {
    __write(1, "y\n", 2);
  }
  reloc_all(&dsos[3]);

  logger_init("misc.log");
  mem_init();
  
  void* initial_dso_heap = malloc(sizeof(struct dso) * 4);
  memcpy(&dsos[0], initial_dso_heap, sizeof(struct dso) * 4);
  
  struct dso* entry_dso = load_library(fname, 0, 0);
  if (entry_dso->entry) {
    __go_entry(entry_dso->entry, rsp);
  }
}

