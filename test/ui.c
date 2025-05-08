// ui_thread.c
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 터미널 raw 모드 활성화
static void enable_raw_mode(UiArgs *arg)
{
  struct termios tio;
  tcgetattr(STDIN_FILENO, &arg->orig_tio);
  tio = arg->orig_tio;
  tio.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &tio);
}

void ui_thread_cleanup(UiArgs *arg)
{
  tcsetattr(STDIN_FILENO, TCSANOW, &arg->orig_tio);
}

static void *ui_thread_func(void *arg)
{
  UiArgs *ui_arg = (UiArgs *)arg;
  enable_raw_mode(ui_arg);
  while (1)
  {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1)
    {
      pthread_mutex_lock(ui_arg->mutex);
      switch (c)
      {
      case '1':
        if (*ui_arg->state == STATE_STOPPED)
        {
          *ui_arg->state = STATE_RUNNING;
          pthread_cond_broadcast(ui_arg->cond);
          printf("[UI] Start\n");
        }
        break;
      case '2':
        if (*ui_arg->state == STATE_RUNNING)
        {
          *ui_arg->state = STATE_STOPPED;
          printf("[UI] Stop\n");
        }
        break;
      case '3':
        *ui_arg->state = STATE_STOPPED;
        printf("[UI] Restarting: reset...\n");
        ui_arg->reset_capture();
        *ui_arg->state = STATE_RUNNING;
        pthread_cond_broadcast(ui_arg->cond);
        printf("[UI] Restarted\n");
        break;
      case 'q':
        ui_thread_cleanup(ui_arg);
        exit(0);
      }
      pthread_mutex_unlock(ui_arg->mutex);
    }
    usleep(10000); // 10ms
  }
  return NULL;
}

int ui_run(UiArgs *arg, pthread_t *tid)
{
  return pthread_create(tid, NULL, ui_thread_func, arg);
}
