
void logger_init(char* name);
void logger_emit(char* name, char* log);
void logger_emit3(char* name, char* log, size_t** va);
void logger_emit_p(void* p);
void logger_emit_m(void* p);
void logger_emit_d(void* p);

#define set_va(V,S)\
  size_t** V;\
  {\
    uint8_t* c1 = S;\
    size_t tmp[2] = {};\
    tmp[0] = c1;\
    V = (size_t**)&tmp;\
  }\
  


