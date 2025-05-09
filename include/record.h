// source2.cpp
#ifndef RECORD_H
#define RECORD_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "thread_arg.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

  /**
   * @brief Start the record thread.
   * @param[in] arg Shared context pointer.
   * @param[out] tid Thread identifier output.
   * @return true if thread created; false on error.
   */
  bool record_run(SharedCtx *arg, pthread_t *tid);

  /**
   * @brief Create (or truncate) raw video file and write header.
   * @param[in] filepath Path to raw video file.
   * @param[out] fd Output file descriptor.
   * @param[in] width Frame width in pixels.
   * @param[in] height Frame height in pixels.
   * @return 0 on success; -1 on failure (errno set).
   */
  int raw_video_writer_open(const char *filepath, int *fd, int width, int height);

  /**
   * @brief Write one frame's data to raw video file.
   * @param[in] fd File descriptor (after header or previous frame).
   * @param[in] buffer Frame data buffer pointer.
   * @param[in] frame_size Bytes per frame.
   * @return 0 on success; -1 on failure (errno set).
   */
  int raw_video_write_frame(int fd, const unsigned char *buffer, size_t frame_size);
#ifdef __cplusplus
}
#endif

#endif // RECORD_H
