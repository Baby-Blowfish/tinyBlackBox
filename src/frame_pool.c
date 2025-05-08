#include "frame_pool.h"

FramePool *frame_pool_create(size_t pool_size, size_t width, size_t height, DEPTH depth)
{
  if (pool_size == 0 || depth == 0 || width == 0 || height == 0)
  {
    errno = EINVAL;
    return NULL;
  }

  FramePool *fp = malloc(sizeof(*fp));
  if (!fp)
  {
    errno = ENOMEM;
    return NULL;
  }

  if (frame_pool_init(fp, pool_size, width, height, depth) != 0)
  {
    free(fp);
    return NULL;
  }

  return fp;
}

int frame_pool_init(FramePool *fp, size_t pool_size, size_t width, size_t height, DEPTH depth)
{
  if (!fp || pool_size == 0 || depth == 0 || width == 0 || height == 0)
  {
    errno = EINVAL;
    return -1;
  }

  // 블록당 크기: Frame 구조체 + 픽셀 데이터
  size_t total_bytes_per_frame;
  if (__builtin_mul_overflow(height, width, &total_bytes_per_frame) ||
      __builtin_mul_overflow(total_bytes_per_frame, depth, &total_bytes_per_frame))
  {
    errno = EOVERFLOW;
    return -1;
  }

  fp->pool_size = pool_size;
  fp->total_bytes_per_frame = total_bytes_per_frame;
  fp->blocks = NULL;
  fp->pool_data = NULL;
  fp->free_list = NULL;

  fp->blocks = calloc(pool_size, sizeof(FrameBlock));
  if (!fp->blocks)
  {
    errno = ENOMEM;
    return -1;
  }

  fp->pool_data = calloc(pool_size, total_bytes_per_frame);
  if (!fp->pool_data)
  {
    errno = ENOMEM;
    free(fp->blocks);
    fp->blocks = NULL;
    return -1;
  }

  pthread_mutex_init(&fp->mutex, NULL);
  pthread_cond_init(&fp->cond, NULL);

  for (size_t i = 0; i < pool_size; ++i)
  {
    FrameBlock *f = &fp->blocks[i];
    atomic_init(&f->refcount, 0);
    f->frame.width = width;
    f->frame.height = height;
    f->frame.seq = 0;
    f->frame.depth = depth;
    f->frame.data = (void *)((char *)fp->pool_data + i * total_bytes_per_frame);
    f->next = fp->free_list;
    fp->free_list = f;
  }
  return 0;
}

void frame_pool_destroy(FramePool *fp)
{
  if (!fp)
    return;
  frame_pool_free(fp);
  free(fp);
  fp = NULL;
}

void frame_pool_free(FramePool *fp)
{
  if (!fp)
    return;
  pthread_mutex_destroy(&fp->mutex);
  free(fp->pool_data);
  free(fp->blocks);
  fp->pool_data = NULL;
  fp->blocks = NULL;
  fp->free_list = NULL;
  fp->pool_size = 0;
  fp->total_bytes_per_frame = 0;
}

FrameBlock *fp_alloc(FramePool *fp, int init_count)
{
  if (!fp || init_count <= 0)
  {
    errno = EINVAL;
    return NULL;
  }
  pthread_mutex_lock(&fp->mutex);
  // free_list가 비어 있으면 cond로 대기
  while (fp->free_list == NULL)
  {
    pthread_cond_wait(&fp->cond, &fp->mutex);
  }
  FrameBlock *f = fp->free_list;
  if (f)
  {
    fp->free_list = f->next;
  }
  pthread_mutex_unlock(&fp->mutex);

  if (!f)
  {
    errno = ENOMEM;
    return NULL;
  }
  atomic_store_explicit(&f->refcount, init_count, memory_order_relaxed);
  return f;
}

void fp_retain(FrameBlock *blk)
{
  if (blk)
  {
    atomic_fetch_add_explicit(&blk->refcount, 1, memory_order_relaxed);
  }
}

void fp_release(FramePool *fp, FrameBlock *blk)
{
  if (!fp || !blk)
    return;
  int prev = atomic_fetch_sub_explicit(&blk->refcount, 1, memory_order_acq_rel);
  if (prev == 1)
  {
    pthread_mutex_lock(&fp->mutex);
    blk->next = fp->free_list;
    fp->free_list = blk;
    pthread_cond_signal(&fp->cond);
    pthread_mutex_unlock(&fp->mutex);
  }
}

void *fp_get_block_data(const FrameBlock *blk)
{
  return blk ? blk->frame.data : NULL;
}

int fp_get_block_refcount(const FrameBlock *blk)
{
  return blk ? atomic_load_explicit(&blk->refcount, memory_order_relaxed) : -1;
}

size_t fp_available_count(const FramePool *fp)
{
  if (!fp)
    return 0;

  size_t count = 0;
  pthread_mutex_lock((pthread_mutex_t *)&fp->mutex);
  for (FrameBlock *p = fp->free_list; p; p = p->next)
    ++count;
  pthread_mutex_unlock((pthread_mutex_t *)&fp->mutex);
  return count;
}

size_t fp_used_count(const FramePool *fp)
{
  if (!fp)
    return 0;
  return fp->pool_size - fp_available_count(fp);
}

size_t fp_total_block_size(const FramePool *fp)
{
  return fp ? fp->pool_size : 0;
}

size_t fp_total_bytes_per_frame_size(const FramePool *fp)
{
  return fp ? fp->total_bytes_per_frame : 0;
}

void fp_debug_dump(const FramePool *fp, FILE *file_p)
{
  if (!file_p)
    file_p = stdout;
  if (!file_p)
  {
    fprintf(file_p, "[FramePool] NULL\n");
    return;
  }

  fprintf(file_p, "[FramePool]\n");
  fprintf(file_p, "  Total Blocks : %zu\n", fp->pool_size);
  fprintf(file_p, "  total_bytes_per_frame_size   : %zu bytes\n", fp->total_bytes_per_frame);
  fprintf(file_p, "  Free Blocks  : %zu\n", fp_available_count(fp));
}
