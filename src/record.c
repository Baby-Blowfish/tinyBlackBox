/*
 * @file record.c
 * @brief Record thread and frame writing implementation.
 */
#include "record.h"

/**
 * @brief Thread function for dequeuing and writing frames.
 *
 * Dequeues blocks, handles wrap semaphores, writes raw frames,
 * and releases blocks back to pool.
 * @param[in] arg Pointer to SharedCtx.
 * @return NULL on thread exit.
 */
static void *record_thread(void *arg)
{

  /* Open file for writing (create or truncate) */
  int fd = open(RECORD_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0)
  {
    perror("raw_video_writer_open: open");
    goto thread_exit;
  }

  // Initialize the record arguments
  SharedCtx *rec_arg = (SharedCtx *)arg;
  FramePool *frame_pool = rec_arg->frame_pool;
  FrameBlock *fb = NULL;

  pthread_mutex_lock(&rec_arg->ui_arg->mutex);
  rec_arg->ui_arg->fds[0] = fd;
  pthread_mutex_unlock(&rec_arg->ui_arg->mutex);

  fprintf(stderr, "%s:%d in %s() → record thread start \n", __FILE__, __LINE__, __func__);

  while (1)
  {
    /* Dequeue block */
    pthread_mutex_lock(&rec_arg->record_q->mutex);
    while (is_empty(rec_arg->record_q))
    {
      pthread_cond_wait(&rec_arg->record_q->cond_not_empty, &rec_arg->record_q->mutex);
    }
    fb = dequeue(rec_arg->record_q);
    pthread_cond_signal(&rec_arg->record_q->cond_not_full);
    pthread_mutex_unlock(&rec_arg->record_q->mutex);

    /* Handle wrap semaphores */
    while (sem_trywait(&rec_arg->wrap_sem) == 0)
    {
      if (lseek(fd, 0, SEEK_SET) < 0)
      {
        perror("record: lseek rewind");
        break;
      }
    }

    /* Write frame */
    if (raw_video_write_frame(fd, fb->frame.data, frame_pool->total_bytes_per_frame) < 0)
    {
      fprintf(stderr, "%s:%d in %s() → failed to write frame\n", __FILE__, __LINE__, __func__);
      goto thread_exit;
    }

     /* Wait if stopped */
    pthread_mutex_lock(&rec_arg->ui_arg->mutex);

    while (rec_arg->ui_arg->state == STATE_STOPPED)
    {
      pthread_cond_wait(&rec_arg->ui_arg->cond, &rec_arg->ui_arg->mutex);
    }
    if (rec_arg->ui_arg->state == STATE_EXIT)
    {

      fprintf(stderr, "%s:%d in %s() → record thread exit\n", __FILE__, __LINE__, __func__);
      goto thread_exit;
    }

    pthread_mutex_unlock(&rec_arg->ui_arg->mutex);

    // release the frame block
    fp_release(frame_pool, fb);
  }

thread_exit:
  close(fd);
  return NULL;
}

bool record_run(SharedCtx *arg, pthread_t *tid)
{
  if (pthread_create(tid, NULL, record_thread, (void *)arg) != 0)
  {
    perror("pthread_create");
    return false;
  }

  return true;
}

int raw_video_writer_open(const char *filepath, int *fd, int width, int height)
{
  int ret = -1;
  int local_fd = -1;
  ssize_t n;

  /* Open file for writing (create or truncate) */
  local_fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (local_fd < 0)
  {
    perror("raw_video_writer_open: open");
    goto cleanup;
  }

  /* Write frame width */
  n = write(local_fd, &width, sizeof(width));
  if (n != sizeof(width))
  {
    if (n < 0)
      perror("raw_video_writer_open: write width");
    else
      fprintf(stderr, "raw_video_writer_open: incomplete write width\n");
    goto cleanup;
  }

  /* Write frame height */
  n = write(local_fd, &height, sizeof(height));
  if (n != sizeof(height))
  {
    if (n < 0)
      perror("raw_video_writer_open: write height");
    else
      fprintf(stderr, "raw_video_writer_open: incomplete write height\n");
    goto cleanup;
  }

  /* Success: hand back fd */
  *fd = local_fd;
  local_fd = -1;
  ret = 0;

cleanup:
  if (local_fd >= 0)
    close(local_fd);
  return ret;
}

int raw_video_write_frame(int fd, const unsigned char *buffer, size_t frame_size)
{
  size_t remaining = frame_size;
  const unsigned char *ptr = buffer;
  ssize_t n;

  while (remaining > 0)
  {
    n = write(fd, ptr, remaining);
    if (n < 0)
    {
      if (errno == EINTR)
        continue; /* interrupted, retry */
      perror("raw_video_writer_write_frame: write");
      return -1;
    }
    ptr += n;
    remaining -= n;
  }

  return 0;
}
