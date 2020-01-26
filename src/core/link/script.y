
%{
  #include <stdio.h>
  #include <stdint.h>
  #include <windows.h>
  #include "link.h"

  uint8_t* wordp;
  uint8_t* p;
  uint32_t CurrentDot;
  
  ///////////////////////////////////////
  typedef struct _Memory Memory;

  // this memory struct is allocated by iterating each entry.
  struct _Memory {
    void* name;
    Memory* next;
    uint32_t attribute;
    uint32_t virtual_address;
    uint32_t len;
  };
  Memory Mem;
  // Since the above memory block is allocated on somewhere heap,
  // you need to make a chain for them as a linked list.
  // After constructing the linked list, you can follow the elements by remembering its beginning.
  Memory* InitialMemoryElem;
  // CurrentMemoryElem is updated when a new entry comes.
  Memory* CurrentMemoryElem;
  
  ////////////////////////////////////////
  // these are about section.  
  uint8_t* OutputSectionName;
  void* InputSectionName;
  void* InputObjectFileName;
  void* CurrentSymbol;
  SectionContainer* InitialSectionContainer;
  SectionContainer* LastSectionContainer;
  //////////////////////////////////////////
  
  int yylex(void);
  void yyerror(const char* s);
  void* _get_word(uint8_t* s);
  void* _print_digit(uint8_t* s);
  void* _get_attribute(uint8_t* s);
  // void check_memory_list(void*);
  
  %}

%token W
%token _NUM
%token NUM
%token MEMORY SECTIONS PHDRS INSERT_K AFTER BEFORE
%token ORIGIN LENGTH

%%

input   : /* empty */
        | input expr
	;

expr    : _NUM
        { printf("num!\n");        }
        | expr expr '+'
        { printf("add!\n");        }
        { $$ = $1 + $2;            }
        | expr expr '-'
	{ $$ = $1 - $2;            }
        | expr expr '*'
	{ $$ = $1 * $2;            }
        | expr expr '/'
	{ $$ = $1 / $2;            }
	| MEMORY '{' memory_group '}'
	{printf("memory!!comp\n");}
        | SECTIONS '{' sections_group '}' assign_memory
	{printf("section!!comp\n");}
;

assign_memory: '>' memory_list
             |
             {printf("assign memory\n");}

// memory statement is the place where you can set virtual address offset, length and its attribute with its tagged name.
// After section was defined with its content enclosed by {}, additional virtual memory information can be followed which
// are noted as one of the name which are defined on a memory statement.
// The action which needs to be done here is to check the given name is matched on a predefined memory name.
// 
memory_list:W
           {
	     printf("memory list:%p\n",p);
	     check_memory_list(wordp);
	   }
;

memory_group:
        memory_group memory_elem
	|
;

memory_elem:
           memory_name '(' attribute ')' ':' ORIGIN '=' virtual_address ',' LENGTH '=' memory_len
	   {
	     printf("memory elem processed\n");
	     printf("memory name:");
	     _get_word(Mem.name);
	     printf("attribute,%d\n", Mem.attribute);
	     printf("virtual address,%d\n", Mem.virtual_address);
	     alloc_memory_elem();
	   }
	   ;

memory_name:word
           {
	     Mem.name = wordp;
	   }
;

attribute:W
         {
	   printf("get attribute\n");
	   p = _get_attribute(wordp);
	   Mem.attribute = 0x777;
	 }
;

virtual_address:num
               {Mem.virtual_address = wordp;}

;

memory_len:num
          {Mem.len = 100/*wordp*/;}
;

///////////////////////////////////////////////////////////

sections_group:
        sections_group sections_elem
	|
	;

// this is aboug procedure where you can resolve definition of each section.
sections_elem :
        '.' '=' num ';' 
        {printf("assignment\n");}

        | section_name ':' '{' section_group '}'
	{
	  printf("section was added,%p,%p\n",OutputSectionName,*OutputSectionName);
	  _get_word(OutputSectionName);
	  SectionContainer* sec = alloc_section_container(0, OutputSectionName);
	  sec->candidate_list = 0;
	  if (LastSectionContainer) {
	    LastSectionContainer->next = sec;
	  } else {
	    InitialSectionContainer = sec;
	  }
	  LastSectionContainer = sec;
	}
;

num: NUM
    {p = _print_digit(wordp);}
;

section_name: word
            {OutputSectionName = wordp;}
;

section_group: section_group section_elem
             |
;

// 1. */(object file name) (section name) => alloc_candidate_section
// 2. *(section name) => alloc_candidate_section
// 3. . = number => section number updating
// 4. name = . => add_symbol
// 5. ....
section_elem:
              '*' '/' object_file_name '(' input_section_name ')'
	    {
	      printf("input object name, input section name\n");
	      // alloc_candidate_section(0, wordp);
	    }
	    | '*' '(' input_section_name ')'
	    {
	      printf("section which are named as xxx comes into the section\n");
	      alloc_candidate_symbol(0, wordp);
	    }
            | '.' '=' NUM ';'
	    {
	      printf("current pointer should be updated\n");
	      // CurrentDot = atoi(num);
	      // num = 0;
	    }
	    | word '=' '.'
	    {
	      printf("should add a symbol on a current virtual address\n");
	      // should reflect CurrentDot
	      // make_ImageSymbol(val);
	    }
	    ;

object_file_name: word
           {
	     InputObjectFileName = wordp;
	   }
;

input_section_name: word
                  {
		    InputSectionName = wordp;
		  }
;

word : W
     {p = _get_word(wordp);}
;

/* sections : SECTIONS */

%%

void* _get_attribute(uint8_t* s) {
  for (;;s++) {
    if (*s == 'r' || *s == 'w' || *s == 'x') {
      
    } else {
      break;
    }
  }
  return s;
}

void* _get_word(uint8_t* s) {
  for (;;s++) {
    if ((0x2d <= *s && *s <= 0x39) ||
	(0x41 <= *s && *s <= 0x5a) ||
	(0x61 <= *s && *s <= 0x7a)// ||
	// (*s == 0x2e)
	) {
      printf("%c",*s);
    } else {
      break;
    }
    // if (*s == 0x20 || *s == 0x0a || *s == 0x00) break;
  }
  printf("\n");
  printf("_print ok\n");
  return s;
}

void* _print_digit(uint8_t* s) {
  if (*s == 0x30/*0*/ && *(s+1) == 0x78/*x*/) {
    s+=2;
    printf("0x");
  }
  for (;;s++) {
    if (0x30 <= *s && *s <= 0x39) {
      printf("%c",*s);
    } else {
      break;
    }
  }
  printf("\n");
  printf("print digit,%p\n",*s);
  return s;
}

#define CHECK_STRING_TOKEN(X,Y)			\
  if (!strncmp(p, #X, Y)) {\
    p+=Y;\
    return X;\
  }\

int yylex(void) {

  printf("in\n");
  getchar();
  uint8_t* q;
  for(;*p == 0x20/* */ || *p == '\t' || *p == '\n' ;p++);
  if (*p == 0x40/*@*/) {
    printf("eof\n");
    p++;
    return 0;
  }
  if (*p == '\n'/*0x0a*/ || *p == '{' || *p == ',' ||
      *p == '}' || *p == '.' || *p == '/' ||
      *p == '=' || *p == ':' || *p == '*' ||
      *p == ';' || *p == '(' || *p == ')') {
    printf("aaa,%p,%c\n",*p,*p);
    if (*p == '.' && *(p+1) != ' ') {}
    else {
      q = *p;
      p++;
      return q;
    }
  }
  CHECK_STRING_TOKEN(SECTIONS,8);
  CHECK_STRING_TOKEN(MEMORY,6);
  CHECK_STRING_TOKEN(ORIGIN,6);
  CHECK_STRING_TOKEN(LENGTH,6);

  if (0x30 <= *p && *p <= 0x39) {
    printf("num\n");
    wordp = p;
    return NUM;
  }
  wordp = p;
  return W;
  
  /* printf("LL:%p,%p\n", c, p); */
  // c = input_array[i];
  // i++;
  /* if (c == '.' || isdigit(c)) { */
  /*   printf("c\n"); */
  /*   ungetc(c, stdin); */
  /*   printf("c\n"); */

  /*   scanf("%lf", &yylval); */
  /*   return NUM; */
  /* } */
  /* printf("d,%d\n",c); */
}

void yyerror(const char* s)
{
  fprintf(stderr, "error: %s\n", s);
}

// VirtualAddress
// output_section_name
// pointer to input_section_name_list

// input section list
// alloc_section_container(va, name)
// 

/////////////////////////////////////////////////

void alloc_memory_elem() {
  Memory* m = __malloc(sizeof(Memory));
  m->name = Mem.name;
  m->attribute = Mem.attribute;
  m->virtual_address = Mem.virtual_address;
  m->len = Mem.len;
  if (CurrentMemoryElem) {
    CurrentMemoryElem->next = m;
  } else {
    InitialMemoryElem = m;
  }
  CurrentMemoryElem = m;
}

// memory list is for 
void check_memory_list(void* p) {
  Memory* m = InitialMemoryElem;
  for (;m;m = m->next) {
    
    // if the string matches, then you should say the entry is existed on the table.
    /* if (m->name) { */
      
    /* } */
    // 
  }
}

////////////////////////////////////////////////////

void __check_section() {
  SectionContainer* sec = InitialSectionContainer;
  for (;sec;sec = sec->next) {
    _get_word(sec->name);
    printf("section virtual address:%p\n", sec->virtual_address);
  }
}

void parse_script(void* scr){
  p = scr;
  printf("parse script\n");
  int r = yyparse();
  printf("%d\n",r);
  __check_section();
}


