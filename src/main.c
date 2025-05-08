
#include "capture.h"
#include "display.h"
#include "record.h"
#include "thread_arg.h"

int main(void)
{
  pthread_t capture_thread;
  pthread_t display_thread;
  pthread_t record_thread;

  CaptureArgs *cap_arg = malloc(sizeof(CaptureArgs));
  if (cap_arg == NULL)
  {
    fprintf(stderr, "%s:%d in %s() → Failed to allocate memory for CaptureArgs\n", __FILE__,
            __LINE__, __func__);
    return EXIT_FAILURE;
  }

  DisplayArgs *disp_arg = malloc(sizeof(DisplayArgs));
  if (disp_arg == NULL)
  {
    fprintf(stderr, "%s:%d in %s() → Failed to allocate memory for DisplayArgs\n", __FILE__,
            __LINE__, __func__);
    return EXIT_FAILURE;
  }

  RecordArgs *rec_arg = malloc(sizeof(RecordArgs));
  if (rec_arg == NULL)
  {
    fprintf(stderr, "%s:%d in %s() → Failed to allocate memory for RecordArgs\n", __FILE__,
            __LINE__, __func__);
    return EXIT_FAILURE;
  }

  cap_arg->display_q = queue_init(QUEUE_SIZE);
  cap_arg->record_q = queue_init(QUEUE_SIZE);
  if (cap_arg->display_q == NULL || cap_arg->record_q == NULL)
  {
    fprintf(stderr, "%s:%d in %s() → Failed to allocate memory for queues\n", __FILE__, __LINE__,
            __func__);
    return EXIT_FAILURE;
  }
  disp_arg->display_q = cap_arg->display_q;
  rec_arg->record_q = cap_arg->record_q;

  FramePool *frame_pool = frame_pool_create(POOL_SIZE, WIDTH, HEIGHT, TYPE);
  if (frame_pool == NULL)
  {
    fprintf(stderr, "%s:%d in %s() → Failed to allocate memory for FramePool\n", __FILE__, __LINE__,
            __func__);
    return EXIT_FAILURE;
  }
  cap_arg->frame_pool = frame_pool;
  disp_arg->frame_pool = frame_pool;
  rec_arg->frame_pool = frame_pool;

  if (capture_run(cap_arg, &capture_thread) == false)
  {
    return EXIT_FAILURE;
  }

  if (record_run(rec_arg, &record_thread) == false)
  {
    return EXIT_FAILURE;
  }

  if (display_run(disp_arg, &display_thread) == false)
  {
    return EXIT_FAILURE;
  }

  // Wait for the capture thread to finish
  pthread_join(capture_thread, NULL);
  pthread_join(record_thread, NULL);
  pthread_join(display_thread, NULL);

  if (disp_arg)
    free(disp_arg);
  disp_arg = NULL;

  if (rec_arg)
    free(rec_arg);
  rec_arg = NULL;

  queue_destroy(cap_arg->display_q);
  queue_destroy(cap_arg->record_q);

  if (cap_arg)
    free(cap_arg);
  cap_arg = NULL;

  frame_pool_destroy(frame_pool);
  frame_pool = NULL;

  return EXIT_SUCCESS;
}