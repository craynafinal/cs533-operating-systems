#include "scheduler.h"
#include <aio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const static int debug = 1;

void debug_print(const char* msg) {
  if (debug) {
    fprintf(stderr, "%s", msg);
  }
}

ssize_t read_wrap(int fd, void *buf, size_t count) {
  debug_print("The read_wrap started\n");

  // initialize the async i/o control block
  struct aiocb *aio_cb = malloc (sizeof(struct aiocb));
  memset(aio_cb, 0, sizeof(struct aiocb));
  aio_cb->aio_fildes = fd;
  aio_cb->aio_offset = lseek(fd, 0, SEEK_CUR);
  aio_cb->aio_buf = malloc(count + aio_cb->aio_offset);
  aio_cb->aio_nbytes = count + aio_cb->aio_offset;
  aio_cb->aio_reqprio = 0;
  aio_cb->aio_sigevent.sigev_notify = SIGEV_NONE;

  int result = aio_cb->aio_offset;

  // checking if correct file
  if (result == -1) {
    return result;
  }

  result = aio_read(aio_cb);

  // on success, result should have value 0
  if (result != 0) {
    return result;
  }

  result = aio_error(aio_cb);

  debug_print("Start polling\n");

  // polling and yield if in progress
  while (result == EINPROGRESS) {
    yield();
    result = aio_error(aio_cb);
  }

  debug_print("Polling finished\n");

  if (result) {
    debug_print("The error detected\n");
    if (result == EBADF) {
      result = -1;
    }
  } else {
    debug_print("Successfully done without an error\n");

    memcpy(buf, ((void*)aio_cb->aio_buf) + aio_cb->aio_offset, count);
    aio_cb->aio_offset = lseek(fd, count, SEEK_CUR);

    result = aio_return(aio_cb);
  }

  return result;
}