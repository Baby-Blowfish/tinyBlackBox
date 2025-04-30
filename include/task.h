#ifndef TASK_H
#define TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console_color.h"

  volatile sig_atomic_t keep_running = 1;

#define TASK_QUEUE_CAPACITY 128 // Maximum queue size

  /**
   * @brief Structure representing a task to be processed
   * @details This structure contains information about a command to be executed,
   *          including the client file descriptor, received data, and data length.
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  typedef struct
  {
    int fd;            // Client file descriptor
    char data[BUFSIZ]; // Received data
    int length;        // Data length
  } Task;

  /**
   * @brief Structure representing a task queue
   * @details This structure manages a queue of tasks using a circular buffer,
   *          with synchronization mechanisms for thread safety. The queue is
   *          implemented as a circular buffer with front and rear pointers.
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  typedef struct
  {
    Task queue[TASK_QUEUE_CAPACITY]; // Task storage space (circular queue)
    int front;                       // Queue front index
    int rear;                        // Queue rear index
    int count;                       // Current number of tasks

    pthread_mutex_t mutex; // Mutex for queue access
    pthread_cond_t cond;   // Condition variable (signals when tasks are available)
  } TaskQueue;

  /**
   * @brief Initializes a new task queue
   * @param q Pointer to the task queue to be initialized
   * @details Initializes the task queue by setting front, rear, and count to 0,
   *          and initializing the mutex and condition variables. This function
   *          must be called before any other task queue operations.
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void task_queue_init(TaskQueue *q);

  /**
   * @brief Adds a task to the queue
   * @param q Pointer to the task queue
   * @param task Task to be added
   * @details Adds a task to the end of the queue. If the queue is full, the function
   *          will wait until space becomes available. This function is typically called
   *          by the main thread (epoll loop) to add new tasks.
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void task_queue_push(TaskQueue *q, Task task);

  /**
   * @brief Removes a task from the queue
   * @param q Pointer to the task queue
   * @return The task removed from the queue
   * @details Removes a task from the front of the queue. If the queue is empty, the function
   *          will wait until a task becomes available, unless keep_running is false.
   *          This function is typically called by worker threads to process tasks.
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  Task task_queue_pop(TaskQueue *q);

  /**
   * @brief Frees all resources used by the task queue
   * @param q Pointer to the task queue to be destroyed
   * @details Frees the task queue and all associated resources, including
   *          destroying the mutex and condition variables.
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void task_queue_destroy(TaskQueue *q);

#ifdef __cplusplus
}
#endif

#endif
