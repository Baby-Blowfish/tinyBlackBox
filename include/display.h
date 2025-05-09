/*
 * @file display.h
 * @brief Display thread and framebuffer overlay functions in DoxyZen style.
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "console_color.h"
#include "fbDraw.h"
#include "thread_arg.h"
#define FRAME_INTERVAL_US 33000 /**< Delay between frames in microseconds (33ms) */
#define MENU_COUNT 3            /**< Number of UI menu items */

  /**
   * @brief Launch the display thread.
   *
   * Spawns a pthread to handle framebuffer drawing and UI overlay.
   * @param[in] arg SharedCtx pointer containing display context and queues.
   * @param[out] tid Pointer to pthread_t to store created thread ID.
   * @return true on success; false on failure.
   */
  bool display_run(SharedCtx *arg, pthread_t *tid);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_H
