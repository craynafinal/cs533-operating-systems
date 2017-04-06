#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "scheduler.h"

struct thread* current_thread;
struct queue ready_list;
size_t stack_size = 1024 * 1024;

void thread_wrap() {
  printf("Thread Wrap Starts...\n");
  current_thread->initial_function(current_thread->initial_argument);

  // calling thread_wrap means this thread finished its job
  current_thread->state = DONE;
  yield();

  // program never arrives to this point
  printf("Thread Wrap Ends...\n");
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
  printf("Scheduler Begin Starts...\n");
  // allocate a new thread to current_thread
  current_thread = malloc(sizeof(struct thread));
  current_thread->state = RUNNING;

  // there is nothing in the ready_list yet
  ready_list.head = NULL;
  ready_list.tail = NULL;

  printf("Scheduler Begin Ends...\n");
}

void thread_fork(void(*target)(void*), void *arg) {
  printf("Thread Fork Starts...\n");

  // create a new thread
  struct thread* new_thread;
  new_thread = malloc(sizeof(struct thread));
  // allocate a new stack
  void* stack = malloc(stack_size);
  new_thread->stack_pointer = stack + stack_size - 1;

  new_thread->initial_function = target;
  new_thread->initial_argument = arg;

  // current running thread goes to ready list
  current_thread->state = READY;
  thread_enqueue(&ready_list, current_thread);

  new_thread->state = RUNNING;
  struct thread* temp = current_thread;
  current_thread = new_thread;
  thread_start(temp, current_thread);

  printf("Thread Fork Ends...\n");
}

void scheduler_end() {
  // program arrives here right after thread_fork operations
  printf("Scheduler End Starts...\n");
  while (!is_empty(&ready_list)) {
    yield();
  }
  // program only arrives here when it is done
  printf("Scheduler End Ends...\n");
}
