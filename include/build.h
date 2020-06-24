
#include "hash_table.h"

#define _(X)->(Y) (X)

#define SET(X) context |= (X)
#define ON(X) context |= (X)
#define UNSET(X) context &= ~(X)
#define OFF(X) context &= ~(X)

#define SRC_FILE_MAX_LEN 16

// #define CONTEXT_RULE 0x2
#define CONTEXT_NONE 0x0
#define CONTEXT_RULE_CMD 0x1
#define CONTEXT_RULE_TARGET 0x2
#define CONTEXT_RULE_DEPS 0x2
#define CONTEXT_VARIABLE 0x4
#define CONTEXT_READ_NOW 0x8
#define CONTEXT_RULE_SEP 0x10
#define CONTEXT_COMMENT (0x10 << 1)

// #define DRYRUN 0

typedef uint32_t DWORD;

typedef struct {
  uint8_t* name;
  uint8_t* value;  
  uint8_t kind;
} var;

typedef struct _list list;

struct _list {
  void* p;
  list* next;
};

typedef struct {
  uint8_t* target;
  uint8_t* deps;
  list* cmd;
  int num_deps;
  int num_cmd;
  uint8_t resolved;
} rule;

typedef struct {
  size_t bind_full_str;
  size_t bind_str;
} _bind;

typedef struct {
  uint64_t num;
  var* first_var;
  hash_table var_hash_table;
} vars;

typedef struct {
  uint64_t num;
  rule* first_rule;
} rules;

typedef struct {
  uint8_t context;
  uint8_t* str;
  uint32_t var_num;
  uint32_t rule_num;
} parse_data;

enum var_kind {
  ASSIGN_IMMEDIATE = 1,// :=
  ASSIGN_RECURSIVE,// =
  ASSIGN_RECURSIVE_ALLOW_NON_DEFINED,// ?=
  ASSIGN_ADD,// +=
};

typedef struct {
  vars vars;
  rules rules;
  void* dot_file_handle;
} build_conf;

static void* bind_cmd(rule* r);
static uint8_t check_assign_var(uint8_t** _p, uint8_t** _q);
static void do_wildcard(uint8_t* s, uint8_t** _p, uint8_t** _q);
static void unbind_rule();
static void* assign_var(uint8_t* p, uint8_t* q);
static int get_length_var(uint8_t* p);

void* search_rule(void* p);
void* retrieve(uint64_t** dest, uint8_t* p, uint8_t* p1, uint8_t var_check);
char* __z__build__resolve(char*, rule*);
