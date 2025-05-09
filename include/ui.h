/*
 * @file ui.h
 * @brief UI thread setup and control
 */
#ifndef UI_H
#define UI_H

#include <pthread.h>
#include <stdbool.h>
#include <termios.h>

/**
 * @enum State
 * @brief Program execution states managed by the UI thread.
 */
typedef enum
{
  STATE_STOPPED, /**< Paused state */
  STATE_RUNNING, /**< Running state */
  STATE_EXIT     /**< Exit requested */
} State;

/**
 * @struct UiArgs
 * @brief Context and synchronization primitives for the UI thread.
 */
typedef struct
{
  int fds[2];                  /**< File descriptors for reset callbacks */
  State state;                 /**< Current program state */
  pthread_mutex_t mutex;       /**< Protects state changes */
  pthread_cond_t cond;         /**< Signals state changes */
  void (*reset_callback)(int); /**< Callback to reset file offset */
  struct termios orig_tio;     /**< Original terminal settings */
} UiArgs;

/**
 * @brief Initialize and start the UI thread.
 *
 * Allocates UiArgs, configures terminal raw mode, and spawns UI thread.
 * @param[out] arg Double pointer to receive allocated UiArgs.
 * @param[out] tid Pointer to pthread_t to receive thread ID.
 * @return true if thread started successfully; false otherwise.
 */
bool ui_run(UiArgs **arg, pthread_t *tid);

/**
 * @brief Clean up UI thread resources and restore terminal.
 * @param[in] ctx Pointer to UiArgs to clean up.
 */
void ui_thread_cleanup(UiArgs *ctx);

/**
 * @brief Allocate and initialize UiArgs structure.
 *
 * Sets initial state, mutex, condvar, and enables raw terminal mode.
 * @return Pointer to UiArgs on success; NULL on failure.
 */
UiArgs *ui_init(void);

#endif // UI_H