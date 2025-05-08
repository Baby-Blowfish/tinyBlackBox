// test_ui.c (테스트용 예제)
#include "ui.h"
#include <stdio.h>
#include <unistd.h>

// 전역 없이 상태를 로컬로 관리
static State state = STATE_STOPPED;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int menu_sel = 0;
void reset_capture_dummy()
{
  printf("[Test] reset_capture called\n");
}

int main()
{
  pthread_t ui_thread;
  UiArgs ui_arg = {.state = &state,
                   .mutex = &mutex,
                   .cond = &cond,
                   .menu_sel = &menu_sel,
                   .reset_capture = reset_capture_dummy};
  if (ui_run(&ui_arg, &ui_thread) != 0)
  {
    perror("pthread_create");
    return 1;
  }

  // STATE_RUNNING 신호 대기 예제
  while (1)
  {
    pthread_mutex_lock(&mutex);
    while (state != STATE_RUNNING)
    {
      pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    printf("[Test] Main: STATE_RUNNING\n");
    sleep(1);
  }
  return 0;
}
