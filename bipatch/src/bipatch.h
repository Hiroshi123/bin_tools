
typedef struct {
  uint8_t cmd;
  uint64_t addr;
  uint16_t length;
  uint8_t *elem;
} patch_format;

typedef struct {
  patch_format *begin;
  patch_format *end;
  int size_updown;
} patch_info;

typedef struct {
  char *map_begin;
  char *map_end;
} map_info;


