#include "queue.h"

/*
 * head, tail, count = 0
 */
Queue *queue_init(size_t capacity)
{

  if (capacity == 0)
    return NULL;

  Queue *queue = (Queue *)malloc(sizeof(Queue));
  if (!queue)
    return NULL;

  queue->buffer = (void **)calloc(capacity, sizeof(void *));
  if (!queue->buffer)
  {
    free(queue);
    return NULL;
  }

  queue->capacity = capacity;
  queue->head = 0;
  queue->tail = 0;
  queue->count = 0;
  pthread_mutex_init(&queue->mutex, NULL);
  pthread_cond_init(&queue->cond_not_full, NULL);
  pthread_cond_init(&queue->cond_not_empty, NULL);

  queue->done = false;

  return queue;
}

void queue_destroy(Queue *queue)
{
  safe_free((void **)&queue->buffer);
  safe_free((void **)&queue);
  pthread_mutex_destroy(&queue->mutex);
  pthread_cond_destroy(&queue->cond_not_full);
  pthread_cond_destroy(&queue->cond_not_empty);
}

int is_empty(Queue *queue)
{
  return queue->count == 0;
}

int is_full(Queue *queue)
{
  return queue->count == queue->capacity;
}

void enqueue(Queue *queue, void *item)
{
  queue->buffer[queue->tail] = item;
  queue->tail = (queue->tail + 1) % queue->capacity;
  queue->count++;
}

void *dequeue(Queue *queue)
{
  void *item = queue->buffer[queue->head];
  queue->head = (queue->head + 1) % queue->capacity;
  queue->count--;
  return item;
}

void queue_set_done(Queue *queue)
{
  pthread_mutex_lock(&queue->mutex);
  queue->done = 1;
  pthread_cond_broadcast(&queue->cond_not_empty);
  pthread_cond_broadcast(&queue->cond_not_full);
  pthread_mutex_unlock(&queue->mutex);
}
