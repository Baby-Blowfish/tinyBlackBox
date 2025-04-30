#ifndef CAPTURE_H
#define CAPTURE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> // for usleep

#include "frame_pool.h"

#define FRAME_INTERVAL_US 33 // 33ms

  typedef struct
  {
    pthread_t tid;
    int fd;
    FramePool *pool;
    volatile bool run;
  } capture_arg_t;

  capture_arg_t *capture_init(const char *filename);

  bool capture_run(capture_arg_t *arg);

  void capture_destroy(capture_arg_t *arg);

#ifdef __cplusplus
}
#endif

#endif // CAPTURE_H
