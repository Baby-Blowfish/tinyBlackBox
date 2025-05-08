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
   * @brief   record 쓰레드를 실행합니다.
   * @param   arg [in]  record 인자 구조체 포인터
   * @param   tid [out] record 쓰레드 ID
   * @return  true: 성공, false: 실패
   */
  bool record_run(SharedCtx *arg, pthread_t *tid);

  /**
   * @brief  Create (or truncate) a raw video file and write its header.
   * @param[in]   filepath  Path to the raw video file.
   * @param[out]  fd        Pointer to store opened file descriptor.
   * @param[in]   width     Frame width (pixels).
   * @param[in]   height    Frame height (pixels).
   * @return 0 on success, -1 on failure (errno is set).
   */
  int raw_video_writer_open(const char *filepath, int *fd, int width, int height);

  /**
   * @brief  Write one frame's worth of data to a raw video file.
   * @param[in]   fd          File descriptor (positioned after header or previous frame).
   * @param[in]   buffer      Pointer to frame data buffer.
   * @param[in]   frame_size  Number of bytes per frame (width * height).
   * @return 0 on success, -1 on failure (errno is set).
   */
  int raw_video_write_frame(int fd, const unsigned char *buffer, size_t frame_size);
#ifdef __cplusplus
}
#endif

#endif // RECORD_H
