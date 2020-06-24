

typedef struct {
  void   (*work)(int argc, void** args);
  // void* work;
  int argc;
  void** args;
  void* prev;
} task;

typedef struct {
  task* front;
  task* rear;
  int len;
} queue;

typedef struct thread_pool {
  // thread**   threads;                  /* pointer to threads        */
  volatile int num_threads_alive;      /* threads currently alive   */
  volatile int num_threads_working;    /* threads currently working */
  // pthread_mutex_t  thcount_lock;       /* used for thread count etc */
  // pthread_cond_t  threads_all_idle;    /* signal to thpool_wait     */
  queue queue;
} thread_pool;

void __z__std__init_thread_pool(int num);
void __z__std__put_task(void* work, int argc, void** args);

