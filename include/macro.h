

#if defined(__APPLE__)
#define EXPORT(s) s
#else
#define EXPORT(s) _##s
#endif

