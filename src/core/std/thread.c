
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "os.h"
#include "alloc.h"
#include "thread.h"

static thread_pool* THREAD_POOL_P = 0;
static int keep_alive = 0;

static task* enqueue(void* work, int argc, void** args) {

  ////////////
  
  task* t = __malloc(sizeof(task));
  t->work = work;
  t->argc = argc;
  t->args = args;
  if (THREAD_POOL_P == 0) {
    __os__write(1, "error\n", 6);
    return 0;
  }
  if (THREAD_POOL_P->queue.rear == 0) {
    THREAD_POOL_P->queue.front = t;
    THREAD_POOL_P->queue.rear = t;
  } else {
    t->prev = THREAD_POOL_P->queue.rear;
    THREAD_POOL_P->queue.rear = t;
  }
  THREAD_POOL_P->queue.len++;

  /////
  
  return 0;
}

static task* dequeue() {

  if (THREAD_POOL_P->queue.len) {
    THREAD_POOL_P->queue.len--;
    task* t = THREAD_POOL_P->queue.front;
    THREAD_POOL_P->queue.front = 0;
    return t;
  }
  return 0;
}

static void do_idle() {
  
  THREAD_POOL_P->num_threads_alive++;
  while (keep_alive) {
    task* t = dequeue();
    if (t) {
      t->work(t->argc, t->args);
    }
  }
  THREAD_POOL_P->num_threads_alive--;
}

static void spawn(void* f) {
  
  int STACK_SIZE = 0x1000;
  uint64_t* r1 = __os__mmap
    (0,STACK_SIZE,PROT_WRITE|PROT_READ|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
  // uint64_t* t = __malloc(8);
  uint64_t* t2 = __malloc(8);
  // *t = 0;  
  // printf("Child pid is %x,%x,%x\n", getppid(),getpid(),syscall(SYS_gettid));
  r1 = (uint64_t*)(((uint8_t*)r1) + STACK_SIZE - 8);
  *r1 = f;
  __os__clone(0,r1,0,t2,0,0);
}

void __z__std__init_thread_pool(int num) {

  thread_pool* pool = __malloc(sizeof(thread_pool));
  THREAD_POOL_P = pool;
  int n = 0;
  keep_alive = 1;
  for (;n<num;n++) {
    spawn(&do_idle);
  }
  
  while (THREAD_POOL_P->num_threads_alive < num);

}

// task should be provided as function pointer.
void __z__std__put_task(void* work, int argc, void** args) {
  
  enqueue(work, argc, args);
  // queue.
}


