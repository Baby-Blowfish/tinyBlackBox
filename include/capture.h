
/*
 * @file capture.h
 * @brief Video capture thread and raw frame reading functions
 */
#ifndef CAPTURE_H
#define CAPTURE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> // for usleep

#include "thread_arg.h"

  /**
   * @brief Start the capture thread.
   * @param[in] arg Shared context pointer.
   * @param[out] tid Thread identifier output.
   * @return true if thread created; false on error.
   */
  bool capture_run(SharedCtx *arg, pthread_t *tid);

  /**
   * @brief Open raw video file and read header dimensions.
   * @param[in] filepath Path to raw video file.
   * @param[out] fd Output file descriptor.
   * @param[out] width Frame width in pixels.
   * @param[out] height Frame height in pixels.
   * @return 0 on success; -1 on failure (errno set).
   */
  int raw_video_open(const char *filepath, int *fd, int *width, int *height);

  /**
   * @brief Read one frame's data from raw video file, rewind on EOF.
   * @param[in] fd File descriptor (after header).
   * @param[out] buffer Buffer sized total_bytes_per_frame.
   * @param[in] total_bytes_per_frame Bytes per frame.
   * @return 0 on success; 1 on wraparound (EOF); -1 on error (errno set).
   */
  int raw_video_read_frame(int fd, void *buffer, size_t total_bytes_per_frame);

#ifdef __cplusplus
}
#endif

#endif // CAPTURE_H
