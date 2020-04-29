
/* #include <windows.h> */
#include <stdio.h>
#include <stdint.h>
#include "alloc.h"

#ifdef _Win32
#include "coff.h"
#endif
#include "link.h"

ObjectChain* InitialObject = 0;
ObjectChain* CurrentObject = &InitialObject;

extern Config* Confp;

ObjectChain* alloc_obj_chain_init(void* sym_begin, void* str_begin, uint32_t sym_num) {
  ObjectChain* sc = __malloc(sizeof(ObjectChain));
  sc->symbol_chain_head = 0;
  sc->symbol_chain_tail = 0;
  sc->symbol_table_p = sym_begin;
  sc->str_table_p = str_begin;
  sc->symbol_num = sym_num;
  sc->section_chain_head = 0;
  sc->section_chain_tail = 0;
  sc->next = 0;
  return sc;
}

ObjectChain* _alloc_obj_chain(void* sym_begin, void* str_begin, uint32_t sym_num) {
  ObjectChain* sc = alloc_obj_chain_init(sym_begin, str_begin, sym_num);
  Confp->current_object->next = sc;
  Confp->current_object = sc;
  return sc;
}

void update_object_chain(ObjectChain* oc, SectionChain* schain) {

  if (oc->section_chain_head == 0) {
    oc->section_chain_head = schain;
  } else {
    oc->section_chain_tail->next = schain;
  }
  oc->section_chain_tail = schain;
  return;
}

void iterate_object_chain(void* callback_f, void* arg1) {
  ObjectChain* oc = Confp->initial_object;  
  for (;oc;oc = oc->next) {
    printf("l\n");
    callback_arg2_linux(oc, callback_f, arg1);
  }
}

void alloc_obj_chain(void* sym_begin, void* str_begin, uint32_t sym_num) {
  ObjectChain* sc = __malloc(sizeof(ObjectChain));
  if (!InitialObject) {
    InitialObject = sc;
  } else {
    CurrentObject->next = sc;
  }
  CurrentObject = sc;
  CurrentObject->symbol_chain_head = 0;
  CurrentObject->symbol_chain_tail = 0;
  CurrentObject->symbol_table_p = sym_begin;
  CurrentObject->str_table_p = str_begin;
  CurrentObject->symbol_num = sym_num;
  CurrentObject->section_chain_head = 0;
  CurrentObject->section_chain_tail = 0;
  CurrentObject->next = 0;
}

void update_symbol_table_info() {
  ObjectChain* oc = InitialObject;
  for (;oc;oc=oc->next) {
    printf("symtable p:%p\n", oc->symbol_table_p);
    printf("sym num :%d\n", oc->symbol_num);    
  }
  return 0;
}


SectionChain* get_sc_from_obj(int index) {
  SectionChain* sc = CurrentObject->section_chain_head;
  int i = 1;
  for (;sc;sc=sc->next,i++) {
    if (i == index) {
      return sc;
    }
  }
  return 0;
}

void* alloc_obj(char* fname) {

  return alloc_file(fname);
  /* HANDLE hFile = CreateFile */
  /*   ( */
  /*    fname, GENERIC_ALL/\* | GENERIC_EXECUTE*\/, 0, NULL, */
  /*    OPEN_EXISTING/\*CREATE_NEW*\/, 0/\*FILE_SHARE_READ*\/, NULL */
  /*    ); */
  /* if (hFile == -1) { */
  /*   printf("file not found\n"); */
  /*   return 0; */
  /* } */
  /* DWORD wReadSize; */
  /* DWORD size = GetFileSize(hFile , NULL); */
  /* if (size == -1) { */
  /*   printf("cannot get file size\n"); */
  /*   return 0; */
  /* } */
  /* void* p = __malloc(size); */
  /* ReadFile(hFile, p, size, &wReadSize , NULL); */
  /* CloseHandle(hFile); */
  /* return p; */
}


