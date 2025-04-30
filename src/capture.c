#include "capture.h"

static void *capture_thread(void *arg)
{
  capture_arg_t *ctx = (capture_arg_t *)arg;
  FramePool *pool = ctx->pool;
  Queue *free_q = pool->free_q;
  Queue *display_q = pool->display_q;
  Queue *record_q = pool->record_q;
  size_t seq = 0;
  size_t size = pool->frames[0]->width * pool->frames[0]->height;
  int fd = ctx->fd;
  Frame *frame = NULL;
  ssize_t n = 0;
  size_t remaining = 0;
  unsigned char *buf = NULL;

  while (1)
  {
    fprintf(stderr, "%s:%d in %s() → failed \n", __FILE__, __LINE__, __func__);
    pthread_mutex_lock(&free_q->mutex);

    while (is_empty(free_q) && !free_q->done)
    {
      pthread_cond_wait(&free_q->cond_not_empty, &free_q->mutex);
    }

    if (is_empty(free_q) && free_q->done)
    {
      pthread_mutex_unlock(&free_q->mutex);
      break;
    }

    frame = (Frame *)dequeue(free_q);
    atomic_store(&frame->refcount, 2);

    buf = frame->data;
    remaining = size;

    // 한 프레임 전체를 읽을 때까지 반복
    while (remaining > 0)
    {
      n = read(fd, buf, remaining);
      if (n < 0)
      {
        perror("read");
        // 에러 처리 (스레드 종료 or 재시도 등)
        goto thread_exit;
      }
      if (n == 0)
      {
        // EOF → 파일 맨 앞으로 돌리고, 이 프레임 읽기를 다시 시도
        if (lseek(fd, 0, SEEK_SET) < 0)
        {
          perror("lseek");
          goto thread_exit;
        }
        remaining = size;
        buf = frame->data;
        continue;
      }
      buf += n;
      remaining -= n;
    }

    frame->seq = seq++;

    pthread_cond_signal(&free_q->cond_not_full);
    pthread_mutex_unlock(&free_q->mutex);

    pthread_mutex_lock(&display_q->mutex);
    while (is_full(display_q))
    {
      pthread_cond_wait(&display_q->cond_not_full, &display_q->mutex);
    }
    enqueue(display_q, (void *)frame);
    pthread_cond_signal(&display_q->cond_not_empty);
    pthread_mutex_unlock(&display_q->mutex);

    // pthread_mutex_lock(&record_q->mutex);
    // while (is_full(record_q))
    // {
    //   pthread_cond_wait(&record_q->cond_not_full, &record_q->mutex);
    // }
    // enqueue(record_q, (void *)frame);
    // pthread_cond_signal(&record_q->cond_not_empty);
    // pthread_mutex_unlock(&record_q->mutex);

    usleep(33000);
  }

thread_exit:
  return NULL;
}

capture_arg_t *capture_init(const char *filename)
{
  capture_arg_t *arg = (capture_arg_t *)calloc(1, sizeof(capture_arg_t));
  if (!arg)
  {
    fprintf(stderr, "%s:%d in %s() → failed to allocate capture_arg_t\n", __FILE__, __LINE__,
            __func__);
    return false;
  }

  arg->fd = open(filename, O_RDONLY);
  if (arg->fd == -1)
  {
    fprintf(stderr, "%s:%d in %s() → failed to open file: %s\n", __FILE__, __LINE__, __func__,
            filename);
    goto FAIL;
  }


  arg->run = true;

  return arg;

FAIL:
  if (arg->fd != -1)
    close(arg->fd);
  if (arg)
    free(arg);
  return NULL;
}

bool capture_run(capture_arg_t *arg)
{

  if (pthread_create(&arg->tid, NULL, capture_thread, (void *)arg) != 0)
  {
    perror("pthread_create");
    return false;
  }

  return true;
}

void capture_destroy(capture_arg_t *arg)
{
  frame_pool_destroy(arg->pool);
  close(arg->fd);
  safe_free((void **)arg);
}