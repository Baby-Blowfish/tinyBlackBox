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

static void reset_fd_offset_f(int fd)
{

  if (lseek(fd, 0, SEEK_SET) < 0)
  {
    perror("raw_video_read_frame: lseek");
  }
}

static void disable_raw_mode(UiArgs *arg)
{
  tcsetattr(STDIN_FILENO, TCSANOW, &arg->orig_tio);
}

static void *ui_thread_func(void *arg)
{
  UiArgs *ui_arg = (UiArgs *)arg;

  while (1)
  {
    char c = 0;
    if (read(STDIN_FILENO, &c, 1) == 1)
    {
      pthread_mutex_lock(&ui_arg->mutex);
      switch (c)
      {
      case '2':
        if (ui_arg->state == STATE_STOPPED)
        {
          ui_arg->state = STATE_RUNNING;
          pthread_cond_broadcast(&ui_arg->cond);
          // printf("[UI] Start\n");
        }
        break;
      case '1':
        if (ui_arg->state == STATE_RUNNING)
        {
          ui_arg->state = STATE_STOPPED;
          // printf("[UI] Stop\n");
        }
        break;
      case '3':
        ui_arg->state = STATE_STOPPED;
        printf("[UI] Restarting: reset...\n");
        ui_arg->reset_callback(ui_arg->fds[0]);
        ui_arg->reset_callback(ui_arg->fds[1]);
        ui_arg->state = STATE_RUNNING;
        pthread_cond_broadcast(&ui_arg->cond);
        // printf("[UI] Restarted\n");
        break;
      case 'q':
        ui_arg->state = STATE_EXIT;
        printf("[UI] Exiting...\n");
        pthread_cond_broadcast(&ui_arg->cond);
        pthread_mutex_unlock(&ui_arg->mutex);
        goto exit;
      }
      pthread_mutex_unlock(&ui_arg->mutex);
    }
    usleep(10000); // 10ms
  }

exit:
  sleep(3); // Give some time for the exit message to be printed
  ui_thread_cleanup(ui_arg);
  return NULL;
}

bool ui_run(UiArgs **arg, pthread_t *tid)
{
  *arg = ui_init();

  if (*arg == NULL)
  {
    fprintf(stderr, "Failed to initialize UI *arguments\n");
    return false;
  }

  if (pthread_create(tid, NULL, ui_thread_func, (void *)*arg) != 0)
  {
    perror("pthread_create");
    ui_thread_cleanup(*arg);
    return false;
  }

  return true;
}

UiArgs *ui_init()
{
  UiArgs *args = (UiArgs *)malloc(sizeof(UiArgs));
  if (args == NULL)
  {
    perror("malloc");
    return NULL;
  }

  args->fds[0] = -1;
  args->fds[1] = -1;

  // Initialize the state to STATE_STOPPED
  args->state = STATE_STOPPED;
  if (pthread_mutex_init(&args->mutex, NULL) != 0)
  {
    perror("pthread_mutex_init");
    goto cleanup;
  }

  if (pthread_cond_init(&args->cond, NULL) != 0)
  {
    perror("pthread_cond_init");
    goto cleanup;
  }

  args->reset_callback = reset_fd_offset_f;

  enable_raw_mode(args);

  return args;

cleanup:
  ui_thread_cleanup(args);
  return NULL;
}

void ui_thread_cleanup(UiArgs *arg)
{
  if (arg)
  {
    disable_raw_mode(arg);
    pthread_mutex_destroy(&arg->mutex);
    pthread_cond_destroy(&arg->cond);
    free(arg);
    arg = NULL;
  }
}