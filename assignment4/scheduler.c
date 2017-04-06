#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "scheduler.h"

#define NOT_HELD 0
#define DEBUG 1

struct thread* current_thread;
struct queue ready_list;
size_t stack_size = 1024 * 1024;

void debug_print_id(int id, const char* msg) {
  if (DEBUG) {
    fprintf(stderr, "id %d: %s\n", id, msg);
  }
}

void debug_print_simple(const char* msg) {
  if (DEBUG) {
    fprintf(stderr, "%s\n", msg);
  }
}

void mutex_init(struct mutex* lock) {
  // debug_print_simple("mutex_init called");

  lock->held = NOT_HELD;
  lock->waiting_threads = malloc(sizeof(struct queue));
}

void mutex_lock(struct mutex* lock) {
  // debug_print_id(current_thread->id, "mutex_lock called");

  if(lock->held != NOT_HELD) {
    // keep the thread info in the waiting list
    debug_print_id(current_thread->id, "mutex not acquired");
    current_thread->state = BLOCKED;
    thread_enqueue(lock->waiting_threads, current_thread);
    yield();
  } else {
    // get the lock, use id to save the thread info
    lock->held = current_thread->id;
    debug_print_id(lock->held, "lock acquired");
  }
}

void mutex_unlock(struct mutex* lock) {
  // debug_print_id(current_thread->id, "mutex_unlock called");

  // exit if thread id does not match
  if(current_thread->id != lock->held) {
    return;
  }

  // lock is acquirable by others
  debug_print_id(lock->held, "lock released");
  lock->held = NOT_HELD;
  struct thread* waiting_thread = thread_dequeue(lock->waiting_threads);

  if(waiting_thread) {
    // if the thread is valid, it gets the lock
    waiting_thread->state = READY;
    thread_enqueue(&ready_list, waiting_thread);
    lock->held = waiting_thread->id;
    debug_print_id(waiting_thread->id, "lock acquired");
  }
}

void condition_init(struct condition* cond) {
  cond->waiting_threads = malloc(sizeof(struct queue));
}

void condition_wait(struct condition* cond, struct mutex* lock) {
  mutex_unlock(lock);

  // current thread becomes blocked and waits
  current_thread->state = BLOCKED;
  thread_enqueue(cond->waiting_threads, current_thread);
  yield();
}

void condition_signal(struct condition* cond) {
  struct thread *wake_thread = thread_dequeue(cond->waiting_threads);

  if(wake_thread) {
    // if the thread is valid, put it in the read list
    wake_thread->state = READY;
    thread_enqueue(&ready_list, wake_thread);
  }
}

void condition_broadcast(struct condition* cond) {
  struct thread* wake_thread = thread_dequeue(cond->waiting_threads);

  while(wake_thread) {
    // repeat until the wake_thread is valid since it is broadcast
    wake_thread->state = READY;
    thread_enqueue(&ready_list, wake_thread);
    wake_thread = thread_dequeue(cond->waiting_threads);
  }
}

void thread_join(struct thread* th) {
  // debug_print_id(th->id, "thread_join entered");

  while (th->state != DONE) {
    debug_print_id(th->id, "before yield");
    yield();
    debug_print_id(th->id, "after yield");
  }
  // debug_print_id(th->id, "thread_join finished");
}

void thread_wrap() {
  // printf("Thread Wrap Starts...\n");
  current_thread->initial_function(current_thread->initial_argument);

  // calling thread_wrap means this thread finished its job
  current_thread->state = DONE;
  yield();

  // program never arrives to this point
  // printf("Thread Wrap Ends...\n");
}

void yield() {

  if (!is_empty(&ready_list)) {
    // put this thread in the ready  list if still running
    if (current_thread->state != DONE && current_thread->state != BLOCKED) {
      current_thread->state = READY;
      thread_enqueue(&ready_list, current_thread);
    }

    // get a thread in ready from queue
    struct thread* next_thread = thread_dequeue(&ready_list);

    if (next_thread) {
      next_thread->state = RUNNING;
      struct thread* temp = current_thread;
      current_thread = next_thread;
      thread_switch(temp, current_thread);
    }
  }
}

void scheduler_begin() {
  // printf("Scheduler Begin Starts...\n");
  // allocate a new thread to current_thread
  current_thread = malloc(sizeof(struct thread));
  current_thread->state = RUNNING;

  // there is nothing in the ready_list yet
  ready_list.head = NULL;
  ready_list.tail = NULL;

  // printf("Scheduler Begin Ends...\n");
}

struct thread* thread_fork(void(*target)(void*), void *arg) {
  // printf("Thread Fork Starts...\n");

  // create a new thread
  struct thread* new_thread;
  new_thread = malloc(sizeof(struct thread));
  // allocate a new stack
  void* stack = malloc(stack_size);
  new_thread->stack_pointer = stack + stack_size - 1;

  new_thread->initial_function = target;
  new_thread->initial_argument = arg;

  new_thread->id = clock();

  // current running thread goes to ready list
  current_thread->state = READY;
  thread_enqueue(&ready_list, current_thread);

  new_thread->state = RUNNING;
  struct thread* temp = current_thread;
  current_thread = new_thread;
  thread_start(temp, current_thread);

  // printf("Thread Fork Ends...\n");

  // returning new_thread for assignment 4
  return new_thread;
}

void scheduler_end() {
  // program arrives here right after thread_fork operations
  // printf("Scheduler End Starts...\n");
  while (!is_empty(&ready_list)) {
    yield();
  }
  // program only arrives here when it is done
  // printf("Scheduler End Ends...\n");
}