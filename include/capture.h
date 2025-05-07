#ifndef CAPTURE_H
#define CAPTURE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> // for usleep

#include "frame_pool.h"

#define FRAME_INTERVAL_US 33 // 33ms

  typedef struct
  {
    int fd;
    FramePool *pool;
    pthread_t tid;
    volatile bool run;
  } CapArg;

  /**
   * @brief   CapArg 구조체를 할당 및 초기화(pthread_t 제외)
   * @param   filename    [in] 캡처할 파일 이름 (읽기 전용)
   * @param   count       [in] 풀에 미리 생성할 Frame 개수 (>0)
   * @param   pixel_size  [in] 한 픽셀 당 바이트 수 (ex. GRAY=1, RGB=3)
   * @param   width       [in] 프레임 가로 해상도 (>0)
   * @param   height      [in] 프레임 세로 해상도 (>0)
   * @return  성공 시 MemoryPool*, 실패 시 NULL (errno 설정)
   */
  CapArg *capture_init(const char *filename, size_t pool_size, size_t width, size_t height,
                       DEPTH depth);

  /**
   * @brief   캡처 쓰레드를 실행합니다.
   * @param   arg [in] 캡처 인자 구조체 포인터
   * @return  true: 성공, false: 실패
   */
  bool capture_run(CapArg *arg);

  /**
   * @brief   캡처 쓰레드를 종료합니다.
   * @param   arg [in] 캡처 인자 구조체 포인터
   */
  void capture_destroy(CapArg *arg);

#ifdef __cplusplus
}
#endif

#endif // CAPTURE_H
