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
#define FRAME_INTERVAL_US 33000 // 33ms
#define MENU_COUNT 3

  /**
   * @brief   display 쓰레드를 실행합니다.
   * @param   arg [in]  display 인자 구조체 포인터
   * @param   tid [out] display 쓰레드 ID
   * @return  true: 성공, false: 실패
   */
  bool display_run(SharedCtx *arg, pthread_t *tid);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_H
