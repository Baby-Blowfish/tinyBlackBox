#include "display.h"

#include "record.h"

static void *display_thread(void *arg)
{

  // Initialize the record arguments
  dev_fb frame_dev;
  SharedCtx *disp_arg = (SharedCtx *)arg;
  FramePool *frame_pool = disp_arg->frame_pool;
  FrameBlock *fb = NULL;
  const char *labels[MENU_COUNT] = {"Stop", "Running", "Exit"};

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
    pthread_mutex_lock(&disp_arg->display_q->mutex);
    while (is_empty(disp_arg->display_q))
    {
      pthread_cond_wait(&disp_arg->display_q->cond_not_empty, &disp_arg->display_q->mutex);
    }
    fb = dequeue(disp_arg->display_q);
    pthread_cond_signal(&disp_arg->display_q->cond_not_full);
    pthread_mutex_unlock(&disp_arg->display_q->mutex);

    if (fb_drawGray(&frame_dev, fb->frame.data, fb->frame.width, fb->frame.height) < 0)
    {
      fprintf(stderr, "%s:%d in %s() → failed to draw frame\n", __FILE__, __LINE__, __func__);
      goto thread_exit;
    }

    // 4) UI 메뉴 오버레이
    for (int i = 0; i < MENU_COUNT; ++i)
    {
      pixel pos = {.x = 10 + i * 110, .y = 10};
      if (i == (int)disp_arg->ui_arg->state)
      {
        // 선택된 버튼: 흰 배경 + 검은 텍스트
        fb_fillBox(&frame_dev, (pixel){pos.x - 2, pos.y - 2}, 100, 24, 255, 255, 255);
        fb_printStr(&frame_dev, labels[i], &pos, 15, 0, 0, 0);
      }
      else
      {
        // 비선택 버튼: 테두리 + 흰 텍스트
        fb_drawBox(&frame_dev, (pixel){pos.x - 2, pos.y - 2}, 100, 24, 255, 255, 255);
        fb_printStr(&frame_dev, labels[i], &pos, 15, 255, 255, 255);
      }
    }

    pthread_mutex_lock(&disp_arg->ui_arg->mutex);
    while (disp_arg->ui_arg->state != STATE_RUNNING)
    {
      pthread_cond_wait(&disp_arg->ui_arg->cond, &disp_arg->ui_arg->mutex);
    }
    pthread_mutex_unlock(&disp_arg->ui_arg->mutex);

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
