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
   * @brief   캡처 쓰레드를 실행합니다.
   * @param   arg [in] 캡처 인자 구조체 포인터
   * @param   tid [out] 캡처 쓰레드 ID
   * @return  true: 성공, false: 실패
   */
  bool capture_run(CaptureArgs *arg, pthread_t *tid);

  /**
   * @brief  Open a raw video file and read its width and height.
   * @param[in]   filepath  Path to the raw video file.
   * @param[out]  fd        Pointer to store opened file descriptor.
   * @param[out]  width     Pointer to store frame width (pixels).
   * @param[out]  height    Pointer to store frame height (pixels).
   * @return 0 on success, -1 on failure (errno is set).
   */
  int raw_video_open(const char *filepath, int *fd, int *width, int *height);

  /**
   * @brief  Read one frame's worth of data from a raw video file.
   *         If EOF is reached, rewind to the start of frame data.
   * @param[in]   fd          File descriptor (must be positioned after header).
   * @param[out]  buffer      Pointer to buffer of size `frame_size`.
   * @param[in]   frame_size  Number of bytes per frame (width * height).
   * @return 0 on success, -1 on failure (errno is set).
   */
  int raw_video_read_frame(int fd, void *buffer, size_t total_bytes_per_frame);

#ifdef __cplusplus
}
#endif

#endif // CAPTURE_H
