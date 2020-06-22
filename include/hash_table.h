
enum hash_collision {
  COL_NOSTRCMP,
  COL_OVERRIDE,
  COL_ADD,
  COL_RAISE_ERROR,
};

typedef struct {
  uint32_t nbucket;
  uint32_t nchain;
  size_t* bucket;
  void* chain;
} hash_table;

typedef struct {
  char* key;
  char* value;
  void* next;
} chain;

void __z__std__init_hash_table(void* p);
void __z__std__hash_set(char*, char* , int);
char* __z__std__hash_find(char* key);


