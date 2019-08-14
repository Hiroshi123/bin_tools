

// note that guest address is not actually pointer as it does not allow you
// to access the memory that it contains.
#define p_guest uint32_t
#define p_host uint8_t*

#define LOG_DIR "static"
#define PAGE_DIR "page"

#define DOT_FNAME "graph.dot"
#define MEM_FNAME "memory.md"

#define CODE_SECTION "text.bin"
#define DATA_SECTION "data.bin"
#define LINKEDIT_SECTION "linkedit.bin"

#define INDEX_LIB "_00"
#define INDEX_SEC "_00"
#define PAGR_HTML_ADDRESS "http://localhost:8000/page.html"

// char* PAGE_PATH = "cdcd";

/* #define DOT_PATH LOG_DIR ## / ## DOT_FNAME */
/* #define MEM_PATH LOG_DIR ## / ## MEM_FNAME */

