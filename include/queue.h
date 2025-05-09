/*
 * @file queue.h
 * @brief Thread-safe fixed-size FIFO queue
 */
#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "util.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

  /**
   * @struct Queue
   * @brief Thread-safe fixed-capacity FIFO queue.
   */
  typedef struct
  {
    void **buffer;                 /**< Array buffer of capacity slots */
    size_t head;                   /**< Dequeue index (0 <= head < capacity) */
    size_t tail;                   /**< Enqueue index (0 <= tail < capacity) */
    size_t capacity;               /**< Max number of items */
    size_t count;                  /**< Current number of items */
    pthread_mutex_t mutex;         /**< Protects queue fields */
    pthread_cond_t cond_not_full;  /**< Signaled when space available */
    pthread_cond_t cond_not_empty; /**< Signaled when items available */
    volatile bool done;            /**< Shutdown flag to unblock waiters */
  } Queue;

  /**
   * @brief Create a queue with given capacity.
   * @param[in] capacity Number of slots (>0).
   * @return Pointer to Queue or NULL (errno set).
   */
  Queue *queue_init(size_t capacity);

  /**
   * @brief Destroy a queue and free resources.
   * @param[in,out] queue Queue pointer (NULL safe).
   */
  void queue_destroy(Queue *queue);

  /**
   * @brief Check if queue is empty.
   * @param[in] queue Queue pointer (NULL safe).
   * @return true if empty or q is NULL.
   */
  int is_empty(const Queue *queue);

  /**
   * @brief Check if queue is full.
   * @param[in] queue Queue pointer (NULL safe).
   * @return true if full or q is NULL.
   */
  int is_full(const Queue *queue);

  /**
   * @brief Enqueue an item (assumes space available).
   * @param[in,out] queue Queue pointer (>NULL).
   * @param[in] item Item pointer to enqueue.
   */
  void enqueue(Queue *queue, void *item);

  /**
   * @brief Dequeue an item (assumes item available).
   * @param[in,out] queue Queue pointer (>NULL).
   * @return Pointer to dequeued item.
   */
  void *dequeue(Queue *queue);

  /**
   * @brief Signal shutdown, unblocking all waiting threads.
   * @param[in,out] queue Queue pointer (>NULL).
   */
  void queue_set_done(Queue *queue);

#ifdef __cplusplus
}
#endif

#endif // QUEUE_H
