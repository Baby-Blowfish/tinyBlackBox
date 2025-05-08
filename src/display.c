#include "display.h"

#include "record.h"

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
  RecordArgs *rec_arg = (RecordArgs *)arg;
  FramePool *frame_pool = rec_arg->frame_pool;
  FrameBlock *fb = NULL;

  fprintf(stderr, "%s:%d in %s() → record thread start \n", __FILE__, __LINE__, __func__);

  int num = 3;
  size_t seq = 0;

  while ((num--) > 0)
  {
    fprintf(stderr, "%s:%d in %s() → record thread seq = %ld \n", __FILE__, __LINE__, __func__, seq++);
    // queue the frame block to the record queue
    pthread_mutex_lock(&rec_arg->record_q->mutex);
    while (is_empty(rec_arg->record_q))
    {
      pthread_cond_wait(&rec_arg->record_q->cond_not_empty, &rec_arg->record_q->mutex);
    }
    fb = dequeue(rec_arg->record_q);
    pthread_cond_signal(&rec_arg->record_q->cond_not_full);
    pthread_mutex_unlock(&rec_arg->record_q->mutex);

    // read the frame data into the block
    if (raw_video_writer_write_frame(fd, fb->frame.data, frame_pool->total_bytes_per_frame) < 0)
    {
      fprintf(stderr, "%s:%d in %s() → failed to write frame\n", __FILE__, __LINE__, __func__);
      goto thread_exit;
    }

    // release the frame block
    fp_release(frame_pool, fb);

    /////////////// 테스트 용도
    fp_release(frame_pool, fb);
  }

thread_exit:
  close(fd);
  return NULL;
}

bool record_run(RecordArgs *arg, pthread_t *tid)
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

int raw_video_writer_write_frame(int fd, const unsigned char *buffer, size_t frame_size)
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

