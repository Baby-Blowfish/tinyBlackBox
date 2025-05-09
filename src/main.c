/*
 * @file main.c
 * @brief Application entry: setup threads and shared resources.
 */
#include "capture.h"
#include "display.h"
#include "record.h"
#include "thread_arg.h"

int main(void)
{
  pthread_t capture_thread;
  pthread_t display_thread;
  pthread_t record_thread;
  pthread_t ui_thread;

  SharedCtx *sh_ctx = malloc(sizeof(SharedCtx));
  if (sh_ctx == NULL)
  {
    fprintf(stderr, "%s:%d in %s() → Failed to allocate memory for SharedCtx\n", __FILE__, __LINE__,
            __func__);
    return EXIT_FAILURE;
  }

  /* Open input/output raw files */
  sh_ctx->fd_in = open("in.raw", O_RDONLY);
  sh_ctx->fd_out = open("out.raw", O_RDWR | O_CREAT, 0666);

  /* Init wrap semaphore */
  if (sem_init(&sh_ctx->wrap_sem, 0, 0) < 0)
  {
    perror("sem_init");
    return EXIT_FAILURE;
  }

  /* Create queues and pool */
  sh_ctx->display_q = queue_init(QUEUE_SIZE);
  sh_ctx->record_q = queue_init(QUEUE_SIZE);
  if (sh_ctx->display_q == NULL || sh_ctx->record_q == NULL)
  {
    fprintf(stderr, "%s:%d in %s() → Failed to allocate memory for queues\n", __FILE__, __LINE__,
            __func__);
    return EXIT_FAILURE;
  }

  sh_ctx->frame_pool = frame_pool_create(POOL_SIZE, WIDTH, HEIGHT, TYPE);
  if (sh_ctx->frame_pool == NULL)
  {
    fprintf(stderr, "%s:%d in %s() → Failed to allocate memory for FramePool\n", __FILE__, __LINE__,
            __func__);
    return EXIT_FAILURE;
  }

  /* Start UI thread */
  if (ui_run(&sh_ctx->ui_arg, &ui_thread) == false)
  {
    return EXIT_FAILURE;
  }

  usleep(1000); // 1초 대기
  // UI Thread가 초기화될 때까지 대기

  /* Start worker threads */
  if (capture_run(sh_ctx, &capture_thread) == false)
  {
    return EXIT_FAILURE;
  }

  if (record_run(sh_ctx, &record_thread) == false)
  {
    return EXIT_FAILURE;
  }

  if (display_run(sh_ctx, &display_thread) == false)
  {
    return EXIT_FAILURE;
  }

  /* Join and cleanup */
  pthread_join(capture_thread, NULL);
  pthread_join(record_thread, NULL);
  pthread_join(display_thread, NULL);
  pthread_join(ui_thread, NULL);

  queue_destroy(sh_ctx->display_q);
  queue_destroy(sh_ctx->record_q);
  frame_pool_destroy(sh_ctx->frame_pool);

  if (sh_ctx)
    free(sh_ctx);
  sh_ctx = NULL;

  return EXIT_SUCCESS;
}