#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef enum {
  RUNNING, // currently running
  READY,   // not running but runnable
  BLOCKED, // not running and not runnable
  DONE     // finished
} state_t;

// this represents a thread control block
struct thread {
  // unsigned char is a single byte
  // so a pointer to an array of single bytes
  unsigned char* stack_pointer;

  // creates a variable initial_function that is a pointer to functions
  // the functions take a single void<F3> param and return no values
  void(*initial_function)(void*);

  // void pointers are used to allow pointers to values of
  // any type to be passed into a function
  // so this one goes into the function
  void* initial_argument;

  state_t state;

  // thread id added for assignment 4
  // clock() function will provide id for each thread
  int id;
};

// proto type for API functions
void scheduler_begin();
// thread_fork returns new thread for assignment 4
struct thread* thread_fork(void(*target)(void*), void *arg);
void yield();
void scheduler_end();

void thread_switch(struct thread* oldThread, struct thread* newThread);
void thread_start(struct thread* oldThread, struct thread* newThread);

extern struct thread* current_thread;

struct mutex {
  struct queue* waiting_threads;
  int held;
};

struct condition {
  struct queue* waiting_threads;
};

void mutex_init(struct mutex* lock);
void mutex_lock(struct mutex* lock);
void mutex_unlock(struct mutex* lock);

void condition_init(struct condition* cond);
void condition_wait(struct condition* cond, struct mutex* lock);
void condition_signal(struct condition* cond);
void condition_broadcast(struct condition* cond);

void thread_join(struct thread* th);
#endif