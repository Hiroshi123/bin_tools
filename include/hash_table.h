
enum hash_collision {
  COL_NOSTRCMP,
  COL_OVERRIDE,
  COL_ADD,
  COL_RAISE_ERROR,
};

typedef struct _list list;

struct _list {
  void* p;
  list* next;
};

typedef struct {
  uint32_t nbucket;
  uint32_t nchain;
  size_t* bucket;
  void* chain;
  uint32_t (*hash_f)(const char *s0, void**);
} hash_table;

typedef struct {
  char* key;
  char* value;
  void* next;
} chain;

typedef struct _implicit_trim_len implicit_trim_len;
struct _implicit_trim_len {
  uint8_t begin;
  uint8_t end;
};

void __z__std__init_hash_table(void* p);
void __z__std__hash_set(hash_table* hash_table_p, char*, char* , int);
char* __z__std__hash_find(hash_table* hash_table_p, char* key);
char* __z__std__hash_partial_find(hash_table* hash_table_p, char* key, void* list);

uint32_t __z__std__sysv_hash(const char *s0);
uint32_t __z__std__with_escape_hash(const char *s0, uint8_t*);


