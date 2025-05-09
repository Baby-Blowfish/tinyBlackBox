// thread_args.h
#ifndef THREAD_ARGS_H
#define THREAD_ARGS_H

#include <semaphore.h>

#include "frame_pool.h"
#include "queue.h" // Queue 타입 정의
#include "ui.h"    // UiArgs 타입 정의

#define WIDTH 1920
#define HEIGHT 1080
#define TYPE GRAY
#define POOL_SIZE 10
#define QUEUE_SIZE 30 // 큐의 크기
#define CAPTURE_FILE "data/cap/video1.raw"
#define RECORD_FILE "data/rec/video1_rec.raw"

typedef struct
{
  int fd_in;
  int fd_out;
  sem_t wrap_sem;
  Queue *display_q;
  Queue *record_q;
  FramePool *frame_pool;
  UiArgs *ui_arg; // UI Thread와의 상호작용을 위한 포인터
} SharedCtx;

#endif // THREAD_ARGS_H
