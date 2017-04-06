#include "scheduler.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static int input = 0;
const static int read_max = 1000000000;

void read_wrap_test(void *arg) {
  ssize_t* tmp = (ssize_t*)arg;
  *tmp = read_wrap(input, malloc(read_max + 1), read_max);
}

void print_nth_prime(void *pn) {
  int n = *(int *)pn;
  int c = 1, i = 1;
  while (c <= n) {
    ++i;
    int j, isprime = 1;
    for (j = 2; j < i; ++j) {
      if (i % j == 0) {
        isprime = 0;
        break;
      }
    }
    if (isprime) {
      ++c;
    }
    yield();
  }
  printf("%dth prime: %d\n", n, i);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: ./main <input file name>");
    exit(0);
  }
  input = open(argv[1], NULL);

  ssize_t *result = malloc(sizeof(ssize_t));

  scheduler_begin();

  int n0 = 10, n1 = 1000, n2 = 10000, n3 = 100;
  thread_fork(print_nth_prime, &n0);
  thread_fork(read_wrap_test, (void*)result);
  thread_fork(print_nth_prime, &n1);
  thread_fork(print_nth_prime, &n2);
  thread_fork(print_nth_prime, &n3);

  scheduler_end();

  close(input);

  printf("The read result is %d\n", (int)read(open(argv[1], NULL), malloc(read_max + 1), read_max));
  printf("The read_wrap result is %d\n", (int)result[0]);

  return 0;
}
