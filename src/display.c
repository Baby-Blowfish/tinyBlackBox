#include "display.h"

#include "record.h"

static void *display_thread(void *arg)
{

  // Initialize the record arguments
  dev_fb frame_dev;
  SharedCtx *rec_arg = (SharedCtx *)arg;
  FramePool *frame_pool = rec_arg->frame_pool;
  FrameBlock *fb = NULL;

  // Framebuffer initialization
  if (fb_init(&frame_dev) < 0)
  {
    fprintf(stderr, "%s:%d in %s() → failed to init framebuffer device\n", __FILE__, __LINE__,
            __func__);
    goto thread_exit;
  }

  fprintf(stderr, "%s:%d in %s() → display thread start \n", __FILE__, __LINE__, __func__);

  // int num = 200;
  // size_t seq = 0;

  // while ((num--) > 0)
  // {
  while (1)
  {
    // fprintf(stderr, "%s:%d in %s() → display thread seq = %ld \n", __FILE__, __LINE__, __func__,
    //         seq++);
    // queue the frame block to the record queue
    pthread_mutex_lock(&rec_arg->display_q->mutex);
    while (is_empty(rec_arg->display_q))
    {
      pthread_cond_wait(&rec_arg->display_q->cond_not_empty, &rec_arg->display_q->mutex);
    }
    fb = dequeue(rec_arg->display_q);
    pthread_cond_signal(&rec_arg->display_q->cond_not_full);
    pthread_mutex_unlock(&rec_arg->display_q->mutex);

    if (fb_drawGray(&frame_dev, fb->frame.data, fb->frame.width, fb->frame.height) < 0)
    {
      fprintf(stderr, "%s:%d in %s() → failed to draw frame\n", __FILE__, __LINE__, __func__);
      goto thread_exit;
    }

    // release the frame block
    fp_release(frame_pool, fb);

    usleep(FRAME_INTERVAL_US);
  }

thread_exit:
  fb_close(&frame_dev);
  return NULL;
}

bool display_run(SharedCtx *arg, pthread_t *tid)
{
  if (pthread_create(tid, NULL, display_thread, (void *)arg) != 0)
  {
    perror("pthread_create");
    return false;
  }

  return true;
}
