// thread_args.h
#ifndef THREAD_ARGS_H
#define THREAD_ARGS_H

#include "frame_pool.h"
#include "queue.h" // Queue 타입 정의

#define WIDTH 1920
#define HEIGHT 1080
#define TYPE GRAY
#define POOL_SIZE 10
#define QUEUE_SIZE 30 // 큐의 크기
#define CAPTURE_FILE "data/cap/video1.raw"
#define RECORD_FILE "data/rec/video1_rec.raw"

typedef struct
{
  Queue *display_q;
  Queue *record_q;
  FramePool *frame_pool;
} CaptureArgs;

typedef struct
{
  Queue *display_q;
  FramePool *frame_pool;
} DisplayArgs;

typedef struct
{
  Queue *record_q;
  FramePool *frame_pool;
} RecordArgs;

#endif // THREAD_ARGS_H
