
struct tls_module {
  struct tls_module *next;
  void *image;
  size_t len, size, align, offset;
};

typedef Elf64_Ehdr Ehdr;
typedef Elf64_Phdr Phdr;
typedef Elf64_Sym Sym;
typedef Elf64_Verdef Verdef;
typedef Elf64_Verdaux Verdaux;

typedef uint64_t Elf_Symndx;

typedef uint64_t dev_t;
typedef uint64_t ino_t;

struct dso {

#if DL_FDPIC
  struct fdpic_loadmap *loadmap;
#else
  unsigned char *base;
#endif
  char *name;
  size_t *dynv;
  struct dso *next, *prev;

  Phdr *phdr;
  int phnum;
  size_t phentsize;
  Sym *syms;
  Elf_Symndx *hashtab;
  uint32_t *ghashtab;
  int16_t *versym;
  char *strings;
  struct dso *syms_next, *lazy_next;
  size_t *lazy, lazy_cnt;
  unsigned char *map;
  size_t map_len;
  dev_t dev;
  ino_t ino;
  char relocated;
  char constructed;
  char kernel_mapped;
  char mark;
  char bfs_built;
  char runtime_loaded;
  struct dso **deps, *needed_by;
  size_t ndeps_direct;
  size_t next_dep;
  int ctor_visitor;
  char *rpath_orig, *rpath;
  struct tls_module tls;
  size_t tls_id;
  size_t relro_start, relro_end;
  uintptr_t *new_dtv;
  unsigned char *new_tls;
  struct td_index *td_index;
  struct dso *fini_next;
  char *shortname;
#if DL_FDPIC
  unsigned char *base;
#else
  struct fdpic_loadmap *loadmap;
#endif
  struct funcdesc {
    void *addr;
    size_t *got;
  } *funcdescs;
  size_t *got;
  void* entry;
  uint64_t deps_index;
  char buf[];
};

struct __locale_struct {
  const struct __locale_map *cat[6];
};

struct __libc {
  char can_do_threads;
  char threaded;
  char secure;
  volatile signed char need_locks;
  int threads_minus_1;
  size_t *auxv;
  struct tls_module *tls_head;
  size_t tls_size, tls_align, tls_cnt;
  size_t page_size;
  struct __locale_struct global_locale;
};

struct symdef {
  Sym *sym;
  struct dso *dso;
};

/* enum { */
/*   REL_NONE = 0, */
/*   REL_SYMBOLIC = -100, */
/*   REL_USYMBOLIC, */
/*   REL_GOT, */
/*   REL_PLT, */
/*   REL_RELATIVE, */
/*   REL_OFFSET, */
/*   REL_OFFSET32, */
/*   REL_COPY, */
/*   REL_SYM_OR_REL, */
/*   REL_DTPMOD, */
/*   REL_DTPOFF, */
/*   REL_TPOFF, */
/*   REL_TPOFF_NEG, */
/*   REL_TLSDESC, */
/*   REL_FUNCDESC, */
/*   REL_FUNCDESC_VAL, */
/* }; */

