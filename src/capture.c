/*
 * @file capture.c
 * @brief Capture thread implementation and raw video I/O in DoxyZen style.
 */
#include "capture.h"

/**
 * @brief Thread function for reading frames and dispatching to consumers.
 *
 * Reads raw frames from file, handles wrap-around, sets sequence numbers,
 * and enqueues to display and record queues.
 * @param[in] arg Pointer to SharedCtx containing queues, pool, and UI args.
 * @return NULL on thread exit.
 */
static void *capture_thread(void *arg)
{
  // Open the raw video file
  int fd = open(CAPTURE_FILE, O_RDONLY);
  if (fd == -1)
  {
    fprintf(stderr, "%s:%d in %s() → failed to open file: %s\n", __FILE__, __LINE__, __func__,
            CAPTURE_FILE);
    goto thread_exit;
  }

  // Initialize the capture arguments
  SharedCtx *cap_arg = (SharedCtx *)arg;
  FramePool *frame_pool = cap_arg->frame_pool;
  size_t seq = 0;
  FrameBlock *fb = NULL;
  int wrapped = 0;

  /* Notify UI of input FD */
  pthread_mutex_lock(&cap_arg->ui_arg->mutex);
  cap_arg->ui_arg->fds[1] = fd;
  pthread_mutex_unlock(&cap_arg->ui_arg->mutex);

  fprintf(stderr, "%s:%d in %s() → capture thread start \n", __FILE__, __LINE__, __func__);

  /* Main capture loop */
  while (1)
  {
    /* Wait for RUN state */
    pthread_mutex_lock(&cap_arg->ui_arg->mutex);

    while (cap_arg->ui_arg->state == STATE_STOPPED)
    {
      pthread_cond_wait(&cap_arg->ui_arg->cond, &cap_arg->ui_arg->mutex);
    }
    if (cap_arg->ui_arg->state == STATE_EXIT)
    {
      fprintf(stderr, "%s:%d in %s() → capture thread exit\n", __FILE__, __LINE__, __func__);
      goto thread_exit;
    }

    pthread_mutex_unlock(&cap_arg->ui_arg->mutex);

    // Allocate a frame block from the pool
    fb = fp_alloc(frame_pool, 2);
    if (!fb)
    {
      fprintf(stderr, "%s:%d in %s() → failed to allocate frame block\n", __FILE__, __LINE__,
              __func__);
      goto thread_exit;
    }

    // read the frame data into the block (returns 1 on wrap)
    if ((wrapped = raw_video_read_frame(fd, fb->frame.data, frame_pool->total_bytes_per_frame)) < 0)
    {
      fprintf(stderr, "%s:%d in %s() → failed to read frame\n", __FILE__, __LINE__, __func__);
      goto thread_exit;
    }
    else if (wrapped == 1) // EOF reached
    {
      // fprintf(stderr, "%s:%d in %s() → EOF reached\n", __FILE__, __LINE__, __func__);
      // rewind the file offset to the beginning
      sem_post(&cap_arg->wrap_sem);
    }

    /* Assign sequence */
    fb->frame.seq = seq++;

    /* Enqueue to display */
    pthread_mutex_lock(&cap_arg->display_q->mutex);
    while (is_full(cap_arg->display_q))
    {
      pthread_cond_wait(&cap_arg->display_q->cond_not_full, &cap_arg->display_q->mutex);
    }
    enqueue(cap_arg->display_q, (void *)fb);
    pthread_cond_signal(&cap_arg->display_q->cond_not_empty);
    pthread_mutex_unlock(&cap_arg->display_q->mutex);

    /* Enqueue to record */
    pthread_mutex_lock(&cap_arg->record_q->mutex);
    while (is_full(cap_arg->record_q))
    {
      pthread_cond_wait(&cap_arg->record_q->cond_not_full, &cap_arg->record_q->mutex);
    }
    enqueue(cap_arg->record_q, (void *)fb);
    pthread_cond_signal(&cap_arg->record_q->cond_not_empty);
    pthread_mutex_unlock(&cap_arg->record_q->mutex);
  }

thread_exit:
  close(fd);
  return NULL;
}

bool capture_run(SharedCtx *arg, pthread_t *tid)
{
  if (pthread_create(tid, NULL, capture_thread, (void *)arg) != 0)
  {
    perror("pthread_create");
    return false;
  }

  return true;
}

int raw_video_open(const char *filepath, int *fd, int *width, int *height)
{
  int ret = -1;
  int local_fd = -1;
  ssize_t n;

  /* Open file for reading */
  local_fd = open(filepath, O_RDONLY);
  if (local_fd < 0)
  {
    perror("raw_video_open: open");
    goto cleanup;
  }

  /* Read frame width */
  n = read(local_fd, width, sizeof(*width));
  if (n != sizeof(*width))
  {
    if (n < 0)
      perror("raw_video_open: read width");
    else
      fprintf(stderr, "raw_video_open: unexpected EOF reading width\n");
    goto cleanup;
  }

  /* Read frame height */
  n = read(local_fd, height, sizeof(*height));
  if (n != sizeof(*height))
  {
    if (n < 0)
      perror("raw_video_open: read height");
    else
      fprintf(stderr, "raw_video_open: unexpected EOF reading height\n");
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

int raw_video_read_frame(int fd, void *buffer, size_t total_bytes_per_frame)
{
  size_t remaining = total_bytes_per_frame;
  unsigned char *ptr = (unsigned char *)buffer;
  ssize_t n;
  int wrapped = 0;

  while (remaining > 0)
  {
    n = read(fd, ptr, remaining);
    if (n < 0)
    {
      if (errno == EINTR)
        continue; /* interrupted, retry */
      perror("raw_video_read_frame: read");
      return -1;
    }
    if (n == 0)
    {
      // /* EOF → rewind past header (2 ints) */
      // if (lseek(fd, 2 * sizeof(int), SEEK_SET) < 0)
      // {
      //   perror("raw_video_read_frame: lseek");
      //   return -1;
      // }

      /* EOF → rewind 0 file offset */
      if (lseek(fd, 0, SEEK_SET) < 0)
      {
        perror("raw_video_read_frame: lseek");
        return -1;
      }
      wrapped = 1;
      continue;
    }
    ptr += n;
    remaining -= n;
  }

  return wrapped;
}