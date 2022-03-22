
#include <stdio.h>
#include <stdint.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "alloc.h"
#include "load.h"
#include "os.h"
#include "objformat.h"

#define DEBUG 1

// static int PAGE_SIZE = 0x1000;
#define PAGE_SIZE 0x1000
static int DEB = 0;

static struct dso *HEAD, *TAIL;

static struct dso *load_library(const char *name, struct dso *dso, uint8_t recurse);

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

static uint8_t strcmp(char* p1, char* p2) {
  for (;*p1 && *p2 && *p1 == *p2;p1++,p2++);
  return *p1 - *p2;
}

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
    if (v[0]-1<cnt-1) {
      a[0] |= 1UL<<v[0];
      a[v[0]] = v[1];
    }
  }
}

static void set_dynamic(size_t* v, struct dso *dso) {
  for (; *v; v+=2) {
    // __write(1, "d\n", 2);
    switch (*v) {
    case DT_NEEDED:
      dso->ndeps_direct++;
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

static void copy(uint8_t* src, uint8_t* dst, uint32_t len) {
  int n = 0;
  for (;n<len;src++,dst++,n++)
    *dst = *src;  
}

// why is this implemented here?
// that is because os.dll is not available until it is initilized.
static int getfilesize(void* fd) {
  __lseek(fd, 0, SEEK_END); // seek to end of file */
  int size = __lseek(fd, 0, SEEK_CUR);
  __lseek(fd, 0, SEEK_SET);
  return size;
}

static void *map_library(int fd, struct dso * dso) {

  Ehdr ehdr;
  Phdr phdr;
  int filesize = getfilesize(fd);
  size_t l = __read(fd, &ehdr, sizeof(Ehdr));
  Ehdr *eh = &ehdr;
  if (l<0) return 0;
  if (/*l<sizeof *eh ||*/ (eh->e_type != ET_DYN && eh->e_type != ET_EXEC))
    goto noexec;
  size_t phsize = eh->e_phentsize * eh->e_phnum;
  size_t i = 0;
  uint8_t* v;
  uint8_t* v1;
  int offset = l + phsize;
  void* dyn_offset = 0;
  int dynsize = 0;
  size_t begin_addr = 0;
  int begin_fixed = 0;
  int mapflag = MAP_PRIVATE;
  int prot = 0;
  int size = 0;
  
  // what we do is we always kick off by mapping every data on a file,
  // which does not have any significant performance drawback.
  // while iterating program headers, parts of it which is refferred from offset
  // will be set on the page which was also required from program headers.
  // this initial mapping is no longer needed once data is copied on each subsequently allocated pages.
  v1 = __os__mmap
    (NULL/*begin_addr*/, filesize
     /*PAGE_SIZE*/, PROT_READ | PROT_WRITE | PROT_EXEC,
     mapflag, fd, 0);
  // this is because you are not able to double-map a data on a single file on different memory.
  // It applies the case where the source data and destination address is mutually distinct each other.  

  // That means next procedure is suited.

  // 1. put the data on loaded file on a temporary page
  // 2. put crate empty pages putting these on the required address
  // 3. put the data on each header
  
  for (i=eh->e_phnum; i; i--) {
    l += __read(fd, &phdr, sizeof(Phdr));
    if (phdr.p_type == PT_LOAD) {
      prot = (((phdr.p_flags&PF_R) ? PROT_READ : 0) |
	      ((phdr.p_flags&PF_W) ? PROT_WRITE: 0) |
	      ((phdr.p_flags&PF_X) ? PROT_EXEC : 0));      
      begin_addr = phdr.p_paddr & -PAGE_SIZE;
      // mapflag = MAP_PRIVATE;
      size = phdr.p_memsz + (phdr.p_vaddr & PAGE_SIZE-1);
      /* if (dso->base == 0 && eh->e_phnum == 2) */
      /* 	v = __os__mmap */
      /* 	  (NULL/\*begin_addr*\/, size */
      /* 	   /\*PAGE_SIZE*\/, PROT_READ | PROT_WRITE | PROT_EXEC, */
      /* 	   mapflag, fd, phdr.p_offset & -PAGE_SIZE);       */
      if (begin_addr) {
	begin_fixed = 1;		
	v = __os__mmap
	  (begin_addr, size
	   /*PAGE_SIZE*/, PROT_READ | PROT_WRITE | PROT_EXEC,
	   MAP_PRIVATE | MAP_FIXED|MAP_ANON, -1, 0/*phdr.p_offset & -PAGE_SIZE*/);
	if (DEB)
	  copy(v1 + (phdr.p_offset & -PAGE_SIZE), v, size);
      } else {
	// for (;;);
	v = v1;
      }
      if (v == -1) {
        __write(1, "mmap error\n", 13);
	return;
      }
      if (dso->base == 0)
	dso->base = v;
      else {
	__write(1, "dynamic load phdr skip\n", sizeof("dynamic load phdr skip\n"));	
	// dso->dynv = v + (phdr.p_vaddr & PAGE_SIZE-1);
	// continue;
      }
    }
    if (phdr.p_type == PT_DYNAMIC) {
      if (dso->base == 0) {
        __write(1, "base error\n", 13);
      	// continue;
      }
      if (!dso->dynv)
	dso->dynv = dso->base + phdr.p_offset;
    }
  }
  __write(1, "m1\n", 3);
  // glibc always start from entry point,
  // musl on the other hand compute entry point with base + entry.
  if (begin_fixed == 0)
    dso->entry = dso->base + eh->e_entry;
  else {
    dso->entry = eh->e_entry;
    __write(1, "m4\n", 3);

  }
  // allows the case where dynamic header does not exist
  if (dso->dynv)
    set_dynamic(dso->dynv, dso);

 noexec:
  return;
}

static uint32_t gnu_hash(const char *s0) {
  const unsigned char *s = (void *)s0;
  uint_fast32_t h = 5381;
  for (; *s; s++)
    h += h*32 + *s;
  return h;
}

static Sym *gnu_lookup(uint32_t h1, uint32_t *hashtab, struct dso *dso, const char *s) {
  uint32_t nbuckets = hashtab[0];
  uint32_t *buckets = hashtab + 4 + hashtab[2]*(sizeof(size_t)/4);
  uint32_t i = buckets[h1 % nbuckets];

  if (!i) return 0;

  uint32_t *hashval = buckets + nbuckets + (i - hashtab[1]);

  for (h1 |= 1; ; i++) {
    uint32_t h2 = *hashval++;
    /* __write(1, s, strlen(s)); */
    /* __write(1, "!\n", 3); */
    if ((h1 == (h2|1)) && (!dso->versym || dso->versym[i] >= 0)
        && !strcmp(s, dso->strings + dso->syms[i].st_name))
      return dso->syms+i;
    if (h2 & 1) break;
  }
  return 0;
}

static Sym *gnu_lookup_filtered
(uint32_t h1, uint32_t *hashtab, struct dso *dso,
 const char *s, uint32_t fofs, size_t fmask) {
  const size_t *bloomwords = (const void *)(hashtab+4);
  size_t f = bloomwords[fofs & (hashtab[2]-1)];
  if (!(f & fmask)) return 0;

  f >>= (h1 >> hashtab[3]) % (8 * sizeof f);
  if (!(f & 1)) return 0;

  return gnu_lookup(h1, hashtab, dso, s);
}

// default iteration is
// 1. first-in last-check
// 2. deps only
// instead you may choose
// 1. fist-in fisrt check
static uint8_t find_sym(struct dso *dso, const char *s, int use_deps, struct symdef* def) {
  // int need_def
  uint32_t h = 0, gh = gnu_hash(s), gho = gh / (8*sizeof(size_t)), *ght;
  size_t ghm = 1ul << gh % (8*sizeof(size_t));

  struct dso* d = TAIL;
  Sym *sym = 0;
  int i = TAIL - HEAD;//DSO_NUM;
  for (;;d--,i--) {
    if ((1 << i) & dso->deps_index) {
      if ((ght = d->ghashtab)) {
        sym = gnu_lookup_filtered(gh, ght, d, s, gho, ghm);
      }
      // __write(1, d->name, strlen(d->name));
      // __write(1,"\n", 1);
      if (sym) {
        def->sym = sym;
        def->dso = d;
        return 1;
      }
    }
    if (d == HEAD) break;
  }
  
  // struct dso **deps = use_deps ? HEAD + dso->deps_index : 0;
  // struct dso **deps = use_deps ? dso->deps : 0;
  // syms_next will determine which one is going to be.

  /* for (; dso; dso = use_deps ? *deps++ : dso->syms_next) { */
  /*   Sym *sym; */
  /*   if ((ght = dso->ghashtab)) { */
  /*     sym = gnu_lookup_filtered(gh, ght, dso, s, gho, ghm); */
  /*   } */
  /*   if (sym) { */
  /*     def->sym = sym; */
  /*     def->dso = dso; */
  /*     return 1; */
  /*   } */
  /* } */
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
  uint32_t sym_index;
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
#ifdef DEBUG
      __write(1, name, strlen(name));
      __write(1, "\n", 1);
#endif
      int r = find_sym(dso, name, 1, &def);
      if (r)
	sym_val = def.sym ? def.dso->base + def.sym->st_value : 0;
      else {
	__write(1, "re error\n", 9);
	continue;
      }
      // char* tmp = def.dso->base + def.sym->st_value;
    }

    switch (type) {
    case 0:
      break;
    case R_X86_64_GLOB_DAT/*6*/:
      *reloc_addr = sym_val + addend;
      break;
    case R_X86_64_JUMP_SLOT/*7*/:
      __write(1, "jmpsl\n", 6);
      *reloc_addr = sym_val + addend;
      break;
    case R_X86_64_RELATIVE/*8*/:
      *reloc_addr = dso->base + addend;
      // for (;;);
      break;
    default:
      __write(1,"reloc error\n",12);
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
  __write(1, "!\n", 2);
  if (relasz) do_relocs(p, p->base + (size_t)rela, relasz, 3);
  
  // do_relocs(p, laddr(p, dyn[DT_RELA]), dyn[DT_RELASZ], 3);
  //}
}

static struct dso* load_direct_deps(struct dso *p, char* name) {

  struct dso *q;
  for (q = HEAD; q; q=q->next) {
    if (!strcmp(name, q->name)) {
      return q;
    }
  }
  __write(1, "tttt\n",5);
  return load_library(name, 0, 1);
}

static void load_deps(struct dso *p) {
  // if (p->deps) return;
  size_t* v = p->dynv;
  // always resolve by itself.
  p->deps_index = 1 << p - HEAD;
  if (p->ndeps_direct == 0) return;
  // size_t* q = __malloc(sizeof(void*) * (1 + p->ndeps_direct));
  // p->deps = q;
  struct dso *dso;
  __write(1, "load deps\n", sizeof("load deps\n"));
  for (; *v; v+=2) {
    if (*v == DT_NEEDED) {
#ifdef DEBUG
      // __write(1, p->strings + *(v+1), strlen(p->strings + *(v+1)));
      __write(1, "-\n", 2);
#endif
      // *q = load_direct_deps(p, p->strings + *(v+1));
      // q++;
      dso = load_direct_deps(p, p->strings + *(v+1));
      p->deps_index |= 1 << (dso - HEAD);
    }
  }
}

static struct dso *__load_library(const char *name, struct dso *_dso, uint8_t recurse) {

/*   int fd = __open(name, O_RDONLY, 0777); */
/*   if (fd == -1) { */
/* #ifdef DEBUG */
/*     char str[] = "loadlibrary error\n"; */
/*     __write(1, str, sizeof(str)); */
/* #endif */
/*     return 0;     */
/*   } */
  /* enum OBJECT_FORMAT format = __z__obj__detect_format_fname(name, 0); */
  /* if (format == PE64) { */
  /*   __z__obj__load_pe(name); */
  /* } */
  /* logger_emit("misc.log", "!!!\n");   */
  // logger_emit_p(d);
  return 0;
}
static struct dso *load_library(const char *name, struct dso *_dso, uint8_t recurse) {

  int fd = __open(name, O_RDONLY, 0777);
  if (fd == -1) {
#ifdef DEBUG
    char str[] = "loadlibrary error\n";
    __write(1, str, strlen(str));
#endif
    return 0;
  }
  struct dso* dso;
  if (_dso == 0) {
    dso = TAIL + 1;
    __write(1, TAIL->name, strlen(TAIL->name));
    __write(1, "\n", 1);
    __write(1, name, strlen(name));
    __write(1, "\n", 1);
    __write(1, "gggg\n",5);
    // for (;;);
    // dso = __malloc(sizeof(struct dso));
  } else {
    dso = _dso;
  }
  dso->name = name;
  __write(1, name, strlen(name));
  map_library(fd, dso);
  
  TAIL->next = dso;
  TAIL = dso;
  
  if (recurse) {
    load_deps(dso);
  }
  __close(fd);
  reloc_all(dso);
  return dso;
}

static char** read_argv(char** argv, char** own_name, char** fname) {
  int argc = *argv++;
  // char* own_fname = *argv;
  *own_name = *argv;
  *argv = argc - 1;
  argv++;
  // *fname = *argv;
  int f = 0;
  for (;*argv;argv++) {
    if (!f) {
      *fname = *argv;
      f = 1;
    }
    /* __write(1, *argv, strlen(*argv)); */
    /* __write(1, "\n", 1); */
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
      if (PAGE_SIZE != auxv->a_un.a_val) {
        __write(1, "PAGESIZE differ\n", 16);
        // PAGE_SIZE = auxv->a_un.a_val;	
      }
      break;
    default:
      break;
    }
  }
  if (!aux[AT_BASE]) aux[AT_BASE] = aux[AT_PHDR] & -4096;
}

#define DEPS_N 0

void __start(void* rsp) {

  // __write(1, "s\n", 2);
  char** argv = rsp;
  // stack usage should be minimized.
  size_t i, aux[AT_MAX];
  // struct dso dsos[4] = {};
  char* own_name = 0;
  char* fname = 0;
  argv = read_argv(argv, &own_name, &fname);
  argv = read_zero(argv);
  argv = read_env(argv);
  argv = read_zero(argv);
  read_auxv(argv, &aux);

  // dynamic shared object is allocated on a special area in an aligned manner.
  // 1 page is able to contain maximum 10 dso.
  // In case if the loaded module is more than that, consider increase the size of it.
  struct dso* dsos = __os__mmap
    (NULL, 0x1000/*PAGE_SIZE*/, PROT_READ | PROT_WRITE,
     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  // this is how you will be able to know base address when you did not define base address.
  dsos[DEPS_N].base = aux[AT_BASE];
  Ehdr *eh = (void *)dsos[DEPS_N].base;
  Phdr *ph = (void *)(dsos[DEPS_N].base + eh->e_phoff);
  if (aux[AT_PHNUM] != eh->e_phnum) __write(1, "err001\n", 6);
  if (aux[AT_PHENT] != eh->e_phentsize) __write(1, "err002\n", 6);
  size_t phnum = eh->e_phnum;
  size_t phent = eh->e_phentsize;
  while (phnum-- && ph->p_type != PT_DYNAMIC)
    ph = (void *)((size_t)ph + phent);
  Elf64_Dyn* dynv = (void *)(dsos[DEPS_N].base + ph->p_vaddr);
  dsos[DEPS_N].dynv = dynv;
  dsos[DEPS_N].name = "./__z__loader.so"; // own_name;
  
  set_dynamic(dynv, &dsos[DEPS_N]);
  //
  HEAD = TAIL = &dsos[0];

  
  //////////////////////////////////
  // resolve deps
  
  /* dsos[3].deps_index = 1; */
  /* dsos[2].deps_index = 1 | 2; */
  /* dsos[1].deps_index = 1; */

  /* TAIL = &dsos[4]; */
  /* ((struct dso*)&dsos[3])->next = TAIL; */
  dsos[0].deps_index = 1;// | (1 << 1) | (1 << 2) | (1 << 3);
  
  // relocation for this library itself.
  // After this point, API on which it depends is available.

  // reloc_all(&dsos[DEPS_N]);
    
  /* enum OBJECT_FORMAT format = __z__obj__detect_format_fname(fname, 0); */
  /* if (format != ELF64) { */
  /*   for (;;); */
  /*   return; */
  /* } */
  
  __write(1, "ggg\n", 4);
  DEB = 1;
  struct dso* entry_dso = load_library(fname, 0, 1);
  // for (;;);
  if (entry_dso->entry) {
    __go_entry(entry_dso->entry, rsp);
  }
  for(;;);
}
