#include <stdio.h>
#include <stdlib.h>

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
};

struct thread* current_thread;
struct thread* inactive_thread;

// switching contexts
// 1. save all of the registers on the current stack
// 2. switch stacks
// 3. restore all the registers off the new stack
void thread_switch(struct thread* oldThread, struct thread* newThread);
void thread_start(struct thread* oldThread, struct thread* newThread);

void yield() {
        printf("Yield Starts...\n");
        // switches the pointers before switching context
        struct thread* temp = current_thread;
        current_thread = inactive_thread;
        inactive_thread = temp;
        printf("Thread Switch Starts...\n");
        thread_switch(inactive_thread, current_thread);
        printf("Thread Switch Ends...\n");
        printf("Yield Ends...\n");
}

void testFunction() {
        int i = 0;
        for (i;i<5;i++) {
                printf("***Testing Yield...%d\n", i);
                yield();
        }
}

void thread_wrap() {
        //testFunction();
        printf("Thread Wrap Starts...\n");
        current_thread->initial_function(current_thread->initial_argument);
        //testFunction();
        // exit point when finished
        yield();
        printf("Thread Wrap Ends...\n");
}

int factorial(int n) {
        printf("Factorial Starts...\n");
        return n == 0 ? 1 : n * factorial(n - 1);
}

void fun_with_threads(void* arg) {
        printf("Fun With Threads Starts...\n");
        int n = *(int*)arg;
        printf("%d! = %d\n", n, factorial(n));
        printf("Fun With Threads Ends...\n");
}

int main(void) {
        printf("Start Program...\n");
        // allocate space
        current_thread = malloc(sizeof(struct thread));
        inactive_thread = malloc(sizeof(struct thread));

        // initialize each member in the thread struct
        current_thread->initial_function = fun_with_threads;

        int* p = malloc(sizeof(int));
        *p = 5;
        current_thread->initial_argument = p;

        int static const STACKSIZE = 1024 * 1024;

        current_thread->stack_pointer = malloc(STACKSIZE);
        current_thread->stack_pointer += STACKSIZE;

        // start thread
        printf("Thread Start...\n");
        thread_start(inactive_thread, current_thread);
        printf("Thread Finishes...\n");

        //testFunction();
        //thread_switch(inactive_thread, current_thread);
        return 0;
}

