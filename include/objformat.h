

enum OBJECT_FORMAT {
  NONE,
  ELF,
  MACHO,
  PE,
};

enum OBJECT_FORMAT detect_format(const char* p);