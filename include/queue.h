#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include "util.h"

  typedef struct
  {
    void **buffer;   // 큐의 버퍼
    size_t head;     // 읽기 인덱스      0 <= head <= N
    size_t tail;     // 쓰기 인덱스      0 <= tail <= N
    size_t capacity; // 큐의 최대 크기   N
    size_t count;    // 현재 큐의 수     0 <= count <= N

    pthread_mutex_t mutex;
    pthread_cond_t cond_not_full;  // 생산해라
    pthread_cond_t cond_not_empty; // 소비해라

    volatile bool done; // 큐 종료 플래그

  } Queue;

  Queue *queue_init(size_t capacity);
  void queue_destroy(Queue *queue);

  int is_empty(Queue *queue);
  int is_full(Queue *queue);

  void enqueue(Queue *queue, void *item);
  void *dequeue(Queue *q);

  void frame_queue_set_done(Queue *q);

#ifdef __cplusplus
}
#endif

#endif
